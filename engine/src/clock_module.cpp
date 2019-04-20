#include <assert.h>
#include <stdint.h>
#include "clock_module.h"
#include "utils/zalloc.h"
#include "logging/logging.h"
#include "events.h"
#include "uv.h"
#include "bus.h"

enum {
    CLOCK_ADD_TIMER = 1001,
    CLOCK_DEL_TIMER,
    CLOCK_STOP
};


typedef struct _clock_async_event_t clock_async_event_t;
struct _clock_async_event_t {
    uint32_t    event_id;
    void        *args;
};

int32_t Clock::m_TimerIndex = 0;

Clock::Clock()
    : m_Async(NULL)
    , m_Loop(NULL)
{

}

Clock::~Clock()
{

}

int32_t Clock::generateUniqueTimerId()
{
    int32_t id;

    lock();
    id = ++m_TimerIndex;
    unlock();

    return id;
}

int32_t Clock::addTimer(uint32_t millseconds, bool repeat, timer_notify_func_t func, void *func_args)
{    
    clock_async_event_t *async_event;
    add_timer_params_t *timer_params = NULL;

    async_event = (clock_async_event_t *)zmalloc(sizeof(clock_async_event_t));
    if (async_event != NULL)
    {
        async_event->event_id = CLOCK_ADD_TIMER;
        
        timer_params = (add_timer_params_t *)zmalloc(sizeof(add_timer_params_t));
        if (timer_params)
        {
            uv_async_init(m_Loop, m_Async, processAsyncEvent); 

            timer_params->timer_id = generateUniqueTimerId();
            timer_params->millseconds = millseconds;
            timer_params->repeat = repeat;
            timer_params->func = func;
            timer_params->func_param = func_args;
            timer_params->instance = this;

            async_event->args = timer_params;
            m_Async->data = async_event;
            
            uv_async_send(m_Async);

            wakeUp();  
        }
    }

    return (timer_params != NULL) ? timer_params->timer_id : -1;
}

int32_t Clock::addTimer(add_timer_params_t *param)
{
    uv_timer_t *timer;
    int ret = -1;

    if (param)
    {
        timer = (uv_timer_t *)zmalloc(sizeof(uv_timer_t));
        if (timer)
        {
            timer->data = param;
            uv_timer_init(m_Loop, timer);
            uv_timer_start(timer, onTimer, param->millseconds, param->repeat);

            ret = 0;
        }
    }

    return ret;
}

void Clock::onTimer(uv_timer_t *timer)
{
    add_timer_params_t *param;

    param = (add_timer_params_t *)timer->data;
    if (param)
        param->func(param->timer_id, param->func_param);
}

bool Clock::onThreadStart()
{
    bool ret = false;

    assert(m_Async == NULL);
    assert(m_Loop == NULL);

    m_Loop = (uv_loop_t *)zmalloc(sizeof(uv_loop_t));
    if (m_Loop)
    {
        uv_loop_init(m_Loop);

        m_Async = (uv_async_t *)zmalloc(sizeof(uv_async_t));
        if (m_Async != NULL)
        {                            
            ret = Thread::onThreadStart();
        }        
    }
    
    return ret;
}

void Clock::onThreadStop()
{
    if (m_Async)
    {
        zfree(m_Async);
        m_Async = NULL;
    }
    
    if (m_Loop)
    {
        uv_loop_close(m_Loop);
        zfree(m_Loop);
    }

    Thread::onThreadStop();
}

void Clock::runOnce()
{
    uv_run(m_Loop, UV_RUN_DEFAULT);
}

void Clock::processAsyncEvent(uv_async_t *handle)
{
    Clock *instance;
    clock_async_event_t *async_internal_event;
    
    async_internal_event = (clock_async_event_t *)handle->data;
    if (async_internal_event)
    {
        switch(async_internal_event->event_id)
        {
        case CLOCK_ADD_TIMER:
            if (async_internal_event->args)
            {
                instance = ((add_timer_params_t *)async_internal_event->args)->instance;
                instance->addTimer((add_timer_params_t *)(async_internal_event->args));
                zfree(async_internal_event->args);
            }
            break;
        case CLOCK_DEL_TIMER:
            break;
        default:
            break;
        }

        zfree(async_internal_event);
    }
}

ClockModule::ClockModule()
{
    bool succeed = false;

    m_Clock = new Clock();
    if (m_Clock)
        succeed = m_Clock->start();

    if (!succeed)
        throw std::runtime_error("Failed to construct object ClockModule");
}

ClockModule::~ClockModule()
{
    if (m_Clock)
    {
        m_Clock->stop();
        delete m_Clock;
    }
}

int ClockModule::activateEvent(BusEvent *event)
{
    int32_t timerId;
    timer_param_t *timeout;
    BusModule *module;
    int ret = -1;

    module = event->getDest();
    if (module != NULL && module != this)
    {
        logging_error("Destination module of Event(ID=%d) dose not match\r\n", event->getEventId());
    }
    else
    {
        switch (event->getEventId())
        {
        case TIMER_SETUP_REQUEST:
            timeout = (timer_param_t *)event->getData();
            timerId = m_Clock->addTimer(timeout->millseconds, timeout->repeat, timerNotify, this);
            
            zfree(timeout);
            if (timerId >= 0)
                m_timerId2BusModule[timerId] = event->getFrom();
            
            postTimerSetupResponse(timerId, event->getFrom());
            ret = 0;
            break;
        default:
            logging_error("Activate illegal event: %s\r\n", str_event(event->getEventId()));
            break;
        }
    }

    return ret;
}

void ClockModule::postTimerSetupResponse(int32_t timer_id, BusModule *dest)
{
    BusEvent *event;
    timer_resp_t *resp;

    event = new BusEvent;
    if (event)
    {
        resp = (timer_resp_t *)zmalloc(sizeof(timer_resp_t));
        if (resp)
        {
            resp->timer_id = timer_id;
            resp->activated = false;

            event->init(this, TIMER_SETUP_RESPONSE, "TIMER_SETUP_RESPONSE", resp);

            event->setDest(dest);

            postEvent(event);
        }
        else
        {
            event->release();
        }        
    } 
}

void ClockModule::timerNotify(int32_t timer_id, void *args)
{
    ClockModule *instance;

    instance = static_cast<ClockModule *>(args);
    if (instance)
        instance->timerNotify(timer_id);
}

void ClockModule::timerNotify(int32_t timer_id)
{
    BusModule *dest;

    if (timer_id >= 0)
    {
        dest = m_timerId2BusModule[timer_id];
        if (dest)
        {
            postTimerResponse(timer_id, dest);
        }
    }
    
}

void ClockModule::postTimerResponse(int32_t timer_id, BusModule *dest)
{
    BusEvent *event;
    timer_resp_t *resp;

    event = new BusEvent;
    if (event)
    {
        resp = (timer_resp_t *)zmalloc(sizeof(timer_resp_t));
        if (resp)
        {
            resp->timer_id = timer_id;
            resp->activated = true;
            event->init(this, TIMER_RESPONSE, "TIMER_RESPONSE", resp);
            event->setDest(dest);

            postEvent(event);
        }
        else
        {
            event->release();
        }        
    }    
}
