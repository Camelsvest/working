#include <assert.h>
#include <unistd.h>
#include "netio_module.h"
#include "utils/zalloc.h"
#include "logging/logging.h"

#if defined(ZALLOC_DEBUG) || defined(DEBUG) || defined(_DEBUG)
#define new zdebug_new
#endif

NetIO::NetIO()
    : Thread("NETIO")
{

}

NetIO::~NetIO()
{

}

void NetIO::runOnce()
{
    
}

NetIOModule::NetIOModule()
{
    bool succeed = false;

    m_NetIO = new NetIO;
    if (m_NetIO)
    {
        succeed = m_NetIO->start();
        if (succeed)
        {
            succeed = setBusModuleDesc("NetIOModule");
            if (succeed)
            {
                subscribeEvent(TIMER_SETUP_RESPONSE);
                subscribeEvent(TIMER_DELETE_RESPONSE);
                subscribeEvent(TIMER_RESPONSE);
            }
        }
    }

    if (!succeed)
        throw std::runtime_error("Failed to construct object NetIOModule.");
}

NetIOModule::~NetIOModule()
{
    if (m_NetIO)
    {
        m_NetIO->stop();
        delete m_NetIO;
    }
}

void NetIOModule::onAttach()
{
    timer_param_t *param;
    BusEvent *event;

    ENTER_CLASS_FUNCTION("NetIOModule");

    param = (timer_param_t *)zmalloc(sizeof(timer_param_t));
    param->millseconds = 2000;
    param->repeat = false;
    
    event = new BusEvent;
    event->init(this, TIMER_SETUP_REQUEST, "TIMER_SETUP_REQUEST", param);
    postEvent(event);

    EXIT_CLASS_FUNCTION("NetIOModule");
}

int NetIOModule::activateEvent(BusEvent *event)
{
    timer_resp_t *timer_resp;
    int ret = -1;
    
    ENTER_CLASS_FUNCTION("NetIOModule");

    logging_trace("NetIOModule activate event %u.%s\r\n", event->getSeqNo(), event->getDesc());

    switch (event->getEventId())
    {
    case TIMER_SETUP_RESPONSE:
        
        timer_resp = (timer_resp_t *)event->getData();
        logging_trace("NetIOModule created timer %d.\r\n", timer_resp->timer_id);

        zfree(timer_resp);
        
        ret = 0;
        break;

    case TIMER_RESPONSE:
        timer_resp = (timer_resp_t *)event->getData();
        assert(timer_resp->activated);
        
        logging_trace("NetIOModule timer %d activated.\r\n", timer_resp->timer_id);

        zfree(timer_resp);
        
        ret = 0;
        break;
        
    default:
        logging_error("Activate illegal event: %s\r\n", str_event(event->getEventId()));
        break;
    }

    EXIT_CLASS_FUNCTION("NetIOModule");
    
    return ret;
}
