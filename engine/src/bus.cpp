#include <assert.h>
#include <stdexcept>
#include <string.h>
#include "bus.h"
#include "utils/zalloc.h"
#include "def.h"

#if defined(ZALLOC_DEBUG) || defined(DEBUG) || defined(_DEBUG)
#define new zdebug_new
#endif


pthread_mutex_t BusEvent::m_GlobalMutex = PTHREAD_MUTEX_INITIALIZER;
uint32_t BusEvent::m_GlobalUniqueSeqNo = 0;

BusEvent::BusEvent()
    : m_Id(-1)
    , m_Data(NULL)
    , m_RefCount(0)
    , m_SeqNo(0)
    , m_From(NULL)
    , m_RespSeqNo(0)
{
}

BusEvent::~BusEvent()
{
}

uint32_t BusEvent::generateSeqNo()
{
    uint32_t seq_no;
    
    chk_mutex_lock(&m_GlobalMutex);
    seq_no = m_GlobalUniqueSeqNo++;
    chk_mutex_unlock(&m_GlobalMutex);

    return seq_no;    
}

int32_t BusEvent::init(BusModule *from, int32_t id, const char *desc, void *data)
{
    int32_t ret = -1;
    
    assert(from != NULL);
    m_SeqNo = generateSeqNo();
    m_From = from;
    
    m_RefCount = 0;		
    m_Id = id;
    m_Data = data;
            
    if (desc != NULL)
        ret = setObjectName(desc) ? 0 : -1;

    return ret;

}

int32_t BusEvent::addRef()
{
    int32_t ret;
    lock();
    ret = ++m_RefCount;    
    unlock();

    return ret;
}

int32_t BusEvent::release()
{
    int32_t ret;

    lock();
    ret = --m_RefCount;
    unlock();

    if (ret <= 0)
        delete this;

    return ret;
}

BusModule::BusModule()
    : m_Id(-1)
{

}

BusModule::~BusModule()
{
}

bool BusModule::setBusModuleDesc(const char *desc)
{
    return setObjectName(desc);
}

int32_t BusModule::setBus(Bus *bus)
{
    int32_t ret = -1;

    if (m_Bus == NULL)
    {
        m_Bus = bus;
        ret = 0;
    }
    
    return ret;
}

int32_t BusModule::postEvent(BusEvent *event)
{
    int ret = -1;

    ENTER_CLASS_FUNCTION("BusModule");
    
    if (m_Bus)
    {
        m_Bus->dispatchEvent(event);
        ret = 0;
    }

    EXIT_CLASS_FUNCTION("BusModule");
    
    return ret;    
}

int32_t BusModule::postEventSychronous(BusEvent *event)
{
    int ret = -1;

    ENTER_CLASS_FUNCTION("BusModule");
    
    if (m_Bus)
    {
        m_Bus->dispatchEventSynchronous(event);
        ret = 0;
    }

    EXIT_CLASS_FUNCTION("BusModule");
    
    return ret;
}

int BusModule::dispatchEvent(BusEvent *event)
{
    event_id id;
    std::list<event_id>::iterator itera;

    lock();
    
    for (itera = m_SubscribedId.begin(); itera != m_SubscribedId.end(); itera++)
    {
        id = *itera;
        if (id == event->getEventId())
        {
            // in below function, maybe we shall unsubscribe eventId
            activateEvent(event);
            break;
        }
    }

    unlock();

    return 0;
}

int32_t BusModule::subscribeEvent(event_id id)
{
    std::list<event_id>::iterator itera;
    bool subscribed = false;

    lock();

#ifdef _DEBUG
    event_id node;
    for (itera = m_SubscribedId.begin(); itera != m_SubscribedId.end(); itera++)
    {
        node = *itera;
        if (node == id)
        {
            subscribed = true;
            break;
        }
    }
#endif

    if (!subscribed)
        m_SubscribedId.push_back(id);
    
    unlock();

    return 0;
}

int32_t BusModule::unsubscribeEvent(event_id id)
{
    lock();
    m_SubscribedId.remove(id);
    unlock();

    return 0;
}

void BusModule::onAttach()
{
    // empty
}

void BusModule::onDetach()
{
    // empty    
}

Bus::Bus()
    : Thread("BUS")
    , m_ModuleIndex(0)
{
    int index;
    
    for (index = 0; index < MODULE_MAX_AMOUNT; index++)
    {
        m_ModuleArray[index] = NULL;
    }
}

Bus::~Bus()
{
    uint32_t index;
    BusEvent *event;

    while(!m_EventList.empty())
    {
        event = m_EventList.front();
        event->release();

        m_EventList.pop_front();
    }

    for (index = 0; index < m_ModuleIndex; index++)
    {
        if (m_ModuleArray[index])
        {
            detachModule(index);
        }
    }
}

int32_t Bus::allocModuleId()
{
    int index;

    index = m_ModuleIndex++;
    if (index >= MODULE_MAX_AMOUNT)
    {
        m_ModuleIndex--;
        index = -1;
    }

    return index;
}

int32_t Bus::attachModule(BusModule *module)
{
    int32_t moduleId, ret = -1;

    ENTER_CLASS_FUNCTION("Bus");
    
    lock();

    moduleId = allocModuleId();
    if (moduleId >= 0)
    {
        module->lock();
        module->setBusModuleId(moduleId);
        module->setBus(this);
        module->unlock();

        m_ModuleArray[moduleId] = module;
        
        logging_trace("Attach module %d, %s.\r\n", moduleId, module->getBusModuleDesc());
        ret = 0;
    }    
    unlock();

    module->onAttach();

    EXIT_CLASS_FUNCTION("Bus");
    
    return ret;
}

int32_t Bus::detachModule(BusModule *module)
{
    return detachModule(module->getBusModuleId());
}

int32_t Bus::detachModule(uint32_t moduleId)
{
    BusModule *module;

    ENTER_CLASS_FUNCTION("Bus");
    
    lock();
    if (moduleId >= 0 && moduleId < m_ModuleIndex) 
    {
        module = m_ModuleArray[moduleId];
        if (module != NULL)
        {
            module->onDetach();            

            m_ModuleArray[moduleId] = NULL;
            
            module->lock();
            module->setBus(NULL);
            module->unlock();
            
            logging_trace("Detach module %d, %s.\r\n", moduleId, module->getBusModuleDesc());

        }
        else
        {
             // ... To do, illegal paramter
        }
        
    }
    else
    {
        // ... To do, illegal paramter
    }    
    unlock();

    EXIT_CLASS_FUNCTION("Bus");
    
    return 0;    
}

int32_t Bus::dispatchEvent(BusEvent *event)
{
    int ret;

    ENTER_CLASS_FUNCTION("Bus");
    
    lock();
    m_EventList.push_back(event);
    event->addRef();
    unlock();

    logging_trace("Bus dispatch event: %u.%s.\r\n", event->getSeqNo(), event->getDesc());
    
    ret = wakeUp();

    EXIT_CLASS_FUNCTION("Bus");
    
    return ret;
}

int32_t Bus::dispatchEventSynchronous(BusEvent * event)
{
    int ret;

    ENTER_CLASS_FUNCTION("Bus");

    m_SyncEventList.push_back(event);
    event->addRef();

    logging_trace("Bus dispatch synchronous event: %u.%s.\r\n", event->getSeqNo(), event->getDesc());

    ret = wakeUp();

    EXIT_CLASS_FUNCTION("Bus");

    return ret;
}

void Bus::runOnce()
{
    uint32_t index;
    BusEvent *event;
    BusModule *module;

    ENTER_CLASS_FUNCTION("Bus");

    m_EventList.merge(m_SyncEventList);
    m_SyncEventList.clear();
    
    while (!m_EventList.empty())
    {
        event = m_EventList.front();
        for (index = 0; index < m_ModuleIndex; index++)
        {
            module = m_ModuleArray[index];
            assert(module != NULL);
            module->dispatchEvent(event);
        }
        event->release();
        m_EventList.pop_front();
    }

    EXIT_CLASS_FUNCTION("Bus");
}
