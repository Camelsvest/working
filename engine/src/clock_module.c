#include <assert.h>
#include <stdint.h>
#include "clock_module.h"
#include "utils/zalloc.h"
#include "logging/logging.h"
#include "events.h"
#include "uv.h"
#include "bus.h"

typedef int32_t (*clock_init_func_t)(clock_module_t *clock, uint32_t id, const char *desc);
typedef void (*clock_uninit_func_t)(bus_module_t *module);
typedef struct _clock_vtable_t clock_vtable_t;
struct _clock_vtable_t {
    async_module_uninit_func_t  base_uninit_func;
    clock_uninit_func_t         uninit_func;
};

struct _clock_module_t {
    async_module_t  base;

    clock_init_func_t   init_func;
    clock_vtable_t      *_vptr;

    uv_loop_t           *loop;
    uv_async_t          *async;
};

typedef struct _clock_async_event_t clock_async_event_t;
struct _clock_async_event_t {
    uint32_t    event_id;
    void        *args;
};

static void process_async_event(uv_async_t *handle);


static void clock_activate_timer(clock_module_t *sender, bus_event_t *event)
{
    bus_event_t *resp;
    timer_resp_t *timer_resp;
    
    resp = create_bus_event((bus_module_t *)sender, TIMER_SETUP_RESPONSE, "TIMER_SETUP_RESPONSE", NULL);
    if (resp != NULL)
    {
        timer_resp = (timer_resp_t *)zmalloc(sizeof(timer_resp_t));
        if (timer_resp != NULL)
        {
            timer_resp->error = 0;
            timer_resp->seq_no = event->seq_no;                        
            
            assert(event != NULL);
            resp->dest = event->from;           
            resp->data = timer_resp;

            bus_dispatch_module_event(sender->base.base.bus, event->from, resp);
        }
    }
    else
    {
        logging_error("Cannot allocate event.\r\n");
    }
}

static void activate_timer(uv_timer_t *handle)
{
    bus_event_t *event;

    event = handle->data;
    assert(event->dest != NULL);

    clock_activate_timer((clock_module_t *)event->dest, event);
}

static void clock_add_timer(bus_event_t *event)
{
    timer_param_t *timeout;
    uv_timer_t *timer;

    ENTER_FUNCTION;
    
    if (event != NULL)
    {
        timeout = (timer_param_t *)event->data; 
        assert(timeout != NULL);
        
        timer = (uv_timer_t *)zmalloc(sizeof(uv_timer_t));
        if (timer != NULL)
        {
            uv_timer_init(timer->loop, timer);
            timer->data = event;
            
            uv_timer_start(timer, activate_timer, timeout->millseconds, timeout->repeat);
        }
    }

    EXIT_FUNCTION;
}

static int32_t reinit_async(uv_loop_t *loop, uv_async_t *async)
{
    int32_t ret = -1;
    
    if (async != NULL)
    {
        uv_async_init(loop, async, process_async_event);
        ret = 0;
    }

    return ret;
}

static void process_async_event(uv_async_t *handle)
{
    clock_async_event_t *aysnc_internal_event;
    bus_event_t *event;
    
    ENTER_FUNCTION;

    aysnc_internal_event = (clock_async_event_t *)handle->data;
    switch(aysnc_internal_event->event_id)
    {
        case CLOCK_ADD_TIMER:
            {
                event = (bus_event_t *)aysnc_internal_event->args;
                
                if (event != NULL)
                    clock_add_timer(event);
                else
                    logging_error("Illegal parameter, event cannot be NULL.\r\n");
                
                zfree(event);
                reinit_async(((clock_module_t *)event->dest)->loop, handle);   // next time we continue to use it
            }
            break;
            
        case CLOCK_STOP:
            uv_stop(handle->loop);
            break;
            
        default:
            break;
    }

    zfree(aysnc_internal_event);

    EXIT_FUNCTION;
}


static int32_t init_async(clock_module_t *clock)
{
    int32_t ret = -1;
    
    assert(clock->loop != NULL);

    clock->async = (uv_async_t *)zmalloc(sizeof(uv_async_t));
    if (clock->async != NULL)
    {
        uv_async_init(clock->loop, clock->async, process_async_event);
        
        ret = 0;
    }

    return ret;
}

static void clock_on_start(async_module_t *module)
{   
    assert(module != NULL);
    
    logging_trace("Clock is starting ... \r\n");

    bus_module_subscribe_event((bus_module_t *)module, TIMER_SETUP_REQUEST);
    
    on_start_async_module(module);
}

static int32_t clock_run(async_module_t *module)
{
    clock_module_t *clock;

    ENTER_FUNCTION;
    
    assert(module != NULL);
    clock = (clock_module_t *)module;
    uv_run(clock->loop, UV_RUN_DEFAULT);

    EXIT_FUNCTION;
    
    return 0;
} 

static void clock_on_stop(async_module_t *module)
{
    logging_trace("Clock is stopping...\r\n");
    on_stop_async_module(module);
}

static void clock_uninit(bus_module_t *module)
{
    clock_module_t *clock;

    ENTER_FUNCTION;
    
    clock = (clock_module_t *)module;
    if (clock != NULL)
    {
        assert(clock->base.running == FALSE);
    
        if (clock->_vptr->base_uninit_func != NULL)
            clock->_vptr->base_uninit_func(module);

        zfree(clock->_vptr);
        zfree(clock->async);

        uv_loop_close(clock->loop);
        zfree(clock->loop);        
    }

    EXIT_FUNCTION;   
}

static int32_t clock_activate_event(bus_module_t *module, bus_event_t *event)
{
    clock_module_t *clock;
    clock_async_event_t *async_event;
    BOOL module_matched = TRUE;

    ENTER_FUNCTION;

    if (event != NULL)
    {
        assert(module != NULL);
        clock = (clock_module_t *)module;

        if (event->dest == NULL)
        {
            event->dest = module;
        }
        else if (event->dest != module)
        {
            module_matched = FALSE;
            logging_error("Destination module of Event(ID=%d) dose not match\r\n", event->id);
        }
            
        if (module_matched)
        {
            switch (event->id)
            {
            case TIMER_SETUP_REQUEST:
                async_event = (clock_async_event_t *)zmalloc(sizeof(clock_async_event_t));
                if (async_event != NULL)
                {
                    async_event->event_id = CLOCK_ADD_TIMER;
                    async_event->args = event;                    

                    clock->async->data = async_event;
                    
                    uv_async_send(clock->async);            
                    logging_trace("Activate event: %s\r\n", str_event(event->id));
                }
                break;
            default:
                logging_error("Activate illegal event: %s\r\n", str_event(event->id));
                break;
            }
        }
    }

    EXIT_FUNCTION;

    return 0;
}

static int32_t clock_init(clock_module_t *clock, uint32_t id, const char *desc)
{
    int32_t ret = -1;

    ENTER_FUNCTION;
    
    if (clock != NULL)
    {
        ret = init_async_module((async_module_t *)clock, id, desc);
        if (ret == 0)
        {
            clock->_vptr = (clock_vtable_t *)zmalloc(sizeof(clock_vtable_t));
            clock->_vptr->base_uninit_func = clock->base._vptr->uninit_func;
            clock->_vptr->uninit_func = clock_uninit;

            // override
            clock->base.base._vptr->uninit_func = clock_uninit;
            clock->base.base._vptr->callback_func = clock_activate_event;
            
            clock->base._vptr->on_start_process_func = clock_on_start;
            clock->base._vptr->run_func = clock_run;
            clock->base._vptr->on_end_process_func = clock_on_stop;

            clock->loop = (uv_loop_t *)zmalloc(sizeof(uv_loop_t));
            uv_loop_init(clock->loop);
            if (clock->loop != NULL)
                ret = init_async(clock);
            else
                ret = -1;
        }
    }

    EXIT_FUNCTION;

    return ret;
}

clock_module_t* create_clock(uint32_t id, const char *desc)
{
    clock_module_t *clock;

    ENTER_FUNCTION;
    
    clock = (clock_module_t *)zmalloc(sizeof(clock_module_t));
    clock->init_func = clock_init;
    if (clock->init_func(clock, id, desc) != 0)
    {
        zfree(clock);
        clock = NULL;
    }

    EXIT_FUNCTION;

    return clock;
}

void destroy_clock(clock_module_t *clock)
{
    ENTER_FUNCTION;

    if (clock != NULL)
    {
        if (clock->_vptr->uninit_func != NULL)
            clock->_vptr->uninit_func((bus_module_t *)clock);

        zfree(clock);
    }
    
    EXIT_FUNCTION;
}

int32_t start_clock(clock_module_t *clock)
{
    return start_async_module((async_module_t *)clock);
}

int32_t stop_clock(clock_module_t *clock)
{
    clock_async_event_t *event;
    int32_t ret = -1;

    ENTER_FUNCTION;
    
    event = (clock_async_event_t *)zmalloc(sizeof(clock_async_event_t));
    if (event != NULL)
    {
        event->event_id = CLOCK_STOP;
        event->args = NULL;
        
        clock->async->data = event;
        uv_async_send(clock->async);
    }

    ret = stop_async_module((async_module_t *)clock);

    EXIT_FUNCTION;

    return ret;
}



