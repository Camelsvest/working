#include <assert.h>
#include <stdint.h>
#include "clock_module.h"
#include "utils/zalloc.h"
#include "logging/logging.h"
#include "events.h"
#include "bus.h"

#if defined(ZALLOC_DEBUG) || defined(DEBUG) || defined(_DEBUG)
#define new zdebug_new
#endif

enum {
    CLOCK_ADD_TIMER = 1001,
    CLOCK_DEL_TIMER,
    CLOCK_STOP
};


typedef struct _uv_async_args_t uv_async_args_t;
struct _uv_async_args_t {
    Clock*      instance;
    uint32_t    event_id;
    void        *args;
};

typedef struct _uv_timer_args_t uv_timer_args_t;
struct _uv_timer_args_t {
    Clock*      instance;
    void        *args;
};

int32_t Clock::m_TimerIndex = 0;

Clock::Clock()
    : Thread("CLOCK")
    , m_Loop(NULL)
{

}

Clock::~Clock()
{

}

bool Clock::start()
{
    bool succeed = false;
    
    if (m_Loop == NULL)
    {
        m_Loop = (uv_loop_t *)zmalloc(sizeof(uv_loop_t));
        if (m_Loop)
        {
            uv_loop_init(m_Loop);
            succeed = Thread::start();
        }
    }
    
    return succeed;
}

bool Clock::stop()
{
    uv_async_t *async_handle;
    uv_async_args_t *async_args;
    void *result;
    int ret = -1;
    bool succeed = false;

    if (uv_loop_alive(m_Loop))
    {
        async_handle = (uv_async_t *)zmalloc(sizeof(uv_async_t));
        if (async_handle)
        {
            async_args = (uv_async_args_t *)zmalloc(sizeof(uv_async_args_t));
            if (async_args != NULL)
            {
                async_args->instance = this;
                async_args->event_id = CLOCK_STOP;
                async_args->args = NULL;

                uv_async_init(m_Loop, async_handle, processAsyncEvent);
                async_handle->data = async_args;
            
                uv_async_send(async_handle);
            
                ret = pthread_join(m_ThreadId, &result);
                if (ret == 0)
                    m_ThreadId = 0;
            
                succeed = (ret == 0);
            }
            else
            {
                zfree(async_handle);
            }
        }
    }
    else
    {
        succeed = Thread::stop();
    }

    return succeed;
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
    uv_async_t  *async_handle;
    uv_async_args_t *async_args;
    add_timer_params_t *timer_params = NULL;

    ENTER_CLASS_FUNCTION("Clock");
    
    async_args = (uv_async_args_t *)zmalloc(sizeof(uv_async_args_t));
    if (async_args != NULL)
    {
        async_args->instance = this;
        async_args->event_id = CLOCK_ADD_TIMER;
        
        timer_params = (add_timer_params_t *)zmalloc(sizeof(add_timer_params_t));
        if (timer_params)
        {
            async_handle = (uv_async_t *)zmalloc(sizeof(uv_async_t));
            uv_async_init(m_Loop, async_handle, processAsyncEvent); 

            timer_params->timer_id = generateUniqueTimerId();
            timer_params->millseconds = millseconds;
            timer_params->repeat = repeat;
            timer_params->func = func;
            timer_params->func_param = func_args;

            async_args->args = timer_params;
            async_handle->data = async_args;
            
            uv_async_send(async_handle);

            wakeUp();  
        }
    }

    EXIT_CLASS_FUNCTION("Clock");
    
    return (timer_params != NULL) ? timer_params->timer_id : -1;
}

int32_t Clock::addTimer(add_timer_params_t *param)
{
    uv_timer_args_t *timer_args;
    uv_timer_t *timer;
    int ret = -1;

    ENTER_CLASS_FUNCTION("Clock");
    
    if (param)
    {
        timer = (uv_timer_t *)zmalloc(sizeof(uv_timer_t));
        if (timer)
        {
            timer_args = (uv_timer_args_t *)zmalloc(sizeof(uv_timer_args_t));
            timer_args->instance = this;
            timer_args->args = param;

            uv_timer_init(m_Loop, timer);            
            timer->data = timer_args;
            
            uv_timer_start(timer, onTimer, param->millseconds, param->repeat);

            m_PendingTimerList.push_back(timer);
            
            ret = 0;
        }
    }

    EXIT_CLASS_FUNCTION("Clock");
    
    return ret;
}

void Clock::onTimer(uv_timer_t *timer)
{
    Clock *instance;
    uv_timer_args_t *args;
    add_timer_params_t *param;

    ENTER_CLASS_FUNCTION("Clock");
    
    args = (uv_timer_args_t *)timer->data;
    if (args)
    {
        param = (add_timer_params_t *)args->args;
        if (param)
        {
            param->func(param->timer_id, param->func_param);
            zfree(param);
        }

        instance = args->instance;
        if (instance)
            instance->m_PendingTimerList.remove(timer);

        zfree(args);
    }
    
    uv_close((uv_handle_t *)timer, NULL);
    zfree(timer);

    EXIT_CLASS_FUNCTION("Clock");
}

void Clock::onThreadStop()
{
    uv_timer_t *timer_handle;
    uv_timer_args_t *timer_params;
    
    ENTER_CLASS_FUNCTION("Clock");

    lock();

    while (!m_PendingTimerList.empty())
    {
        timer_handle = m_PendingTimerList.front();

        timer_params = (uv_timer_args_t *)timer_handle->data;
        zfree(timer_params->args);

        zfree(timer_params);
        zfree(timer_handle);

        m_PendingTimerList.pop_front();
    }
    
    if (m_Loop)
    {
        uv_loop_close(m_Loop);
        zfree(m_Loop);
        m_Loop = NULL;
    }

    Thread::onThreadStop();

    unlock();

    EXIT_CLASS_FUNCTION("Clock");
}

void Clock::runOnce()
{
    ENTER_CLASS_FUNCTION("Clock");
    
    uv_run(m_Loop, UV_RUN_DEFAULT);

    EXIT_CLASS_FUNCTION("Clock");
}

void Clock::processAsyncEvent(uv_async_t *handle)
{
    Clock *instance;
    uv_async_args_t *async_internal_event;

    ENTER_CLASS_FUNCTION("Clock");
    
    async_internal_event = (uv_async_args_t *)handle->data;
    if (async_internal_event)
    {
        instance = async_internal_event->instance;
        assert(instance != NULL);
        
        switch(async_internal_event->event_id)
        {
        case CLOCK_ADD_TIMER:
            if (async_internal_event->args)
            {
                instance->addTimer((add_timer_params_t *)(async_internal_event->args));
            }
            break;
        case CLOCK_DEL_TIMER:
            break;
            
        case CLOCK_STOP:
            instance->m_ThreadQuit = true;
            uv_stop(instance->m_Loop);
            break;
            
        default:
            break;
        }

        uv_close((uv_handle_t *)handle, NULL);
        zfree(handle);

        zfree(async_internal_event);
    }

    EXIT_CLASS_FUNCTION("Clock");
}

ClockModule::ClockModule()
{
    bool succeed = false;

    m_Clock = new Clock();
    if (m_Clock)
    {
        succeed = m_Clock->start();

        if (succeed)
        {
            succeed = setBusModuleDesc("ClockModule");
            if (succeed)
            {
                subscribeEvent(TIMER_SETUP_REQUEST);
                subscribeEvent(TIMER_DELETE_REQUEST);
            }
        }
    }

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
    int ret = -1;

    ENTER_CLASS_FUNCTION("ClockModule");

    logging_trace("ClockModule activate event %u.%s\r\n", event->getSeqNo(), event->getDesc());

    switch (event->getEventId())
    {
    case TIMER_SETUP_REQUEST:
        
        timeout = (timer_param_t *)event->getData();
        timerId = m_Clock->addTimer(timeout->millseconds, timeout->repeat, timerNotify, this);
        
        zfree(timeout);
        if (timerId >= 0)
            m_timerId2BusModule[timerId] = event->getFrom();
        
        postTimerSetupResponse(timerId, event->getFrom(), event->getRespSeqNo());
        
        ret = 0;
        break;
    default:
        logging_error("Activate illegal event: %s\r\n", str_event(event->getEventId()));
        break;
    }

    EXIT_CLASS_FUNCTION("ClockModule");

    return ret;
}

void ClockModule::postTimerSetupResponse(int32_t timer_id, BusModule *dest, uint32_t resp_seq_no)
{
    BusEvent *event;
    timer_resp_t *resp;

    ENTER_CLASS_FUNCTION("ClockModule");
    
    event = new BusEvent;
    if (event)
    {
        resp = (timer_resp_t *)zmalloc(sizeof(timer_resp_t));
        if (resp)
        {
            resp->timer_id = timer_id;
            resp->activated = false;

            event->init(this, TIMER_SETUP_RESPONSE, "TIMER_SETUP_RESPONSE", resp);
            event->setRespSeqNo(resp_seq_no);

            postEventSychronous(event);
        }
        else
        {
            event->release();
        }        
    } 

    EXIT_CLASS_FUNCTION("ClockModule");
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

    ENTER_CLASS_FUNCTION("ClockModule");
    
    if (timer_id >= 0)
    {
        dest = m_timerId2BusModule[timer_id];
        if (dest)
        {
            postTimerResponse(timer_id, dest);
        }
    }

    EXIT_CLASS_FUNCTION("ClockModule");
}

void ClockModule::postTimerResponse(int32_t timer_id, BusModule *dest)
{
    BusEvent *event;
    timer_resp_t *resp;

    ENTER_CLASS_FUNCTION("ClockModule");
    
    event = new BusEvent;
    if (event)
    {
        resp = (timer_resp_t *)zmalloc(sizeof(timer_resp_t));
        if (resp)
        {
            resp->timer_id = timer_id;
            resp->activated = true;
            event->init(this, TIMER_RESPONSE, "TIMER_RESPONSE", resp);

            postEventSychronous(event);
        }
        else
        {
            event->release();
        }
    }

    EXIT_CLASS_FUNCTION("ClockModule");
}
