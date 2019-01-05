#include <assert.h>
#include <stdint.h>
#include "clock_module.h"
#include "utils/zalloc.h"
#include "logging/logging.h"
#include "events.h"

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
};

static void clock_on_start(async_module_t *module)
{
   
    logging_trace("Clock is starting ... \r\n");

    bus_module_subscribe_event((bus_module_t *)module, TIMER_EVENT);
    
    on_start_async_module(module);
}

static int32_t clock_run(async_module_t *module)
{
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
    }

    EXIT_FUNCTION;

    
}

static int32_t clock_activate_event(bus_module_t *module, bus_event_t *event, void *param)
{
    ENTER_FUNCTION;
    
    if (event != NULL)
    {
        switch (event->id)
        {
        case TIMER_EVENT:
            break;
        default:
            break;
        }
    }

    EXIT_FUNCTION;

    return 0;
}

static clock_vtable_t clock_vtable = {
    .base_uninit_func = NULL,
    .uninit_func = clock_uninit
};

static int32_t clock_init(clock_module_t *clock, uint32_t id, const char *desc)
{
    int32_t ret = -1;

    ENTER_FUNCTION;
    
    if (clock != NULL)
    {
        ret = init_async_module((async_module_t *)clock, id, desc);
        if (ret == 0)
        {
            clock->_vptr = &clock_vtable;

            clock->_vptr->base_uninit_func = clock->base._vptr->uninit_func;
            clock->base.base._vptr->uninit_func = clock_uninit;
            clock->base.base._vptr->callback_func = clock_activate_event;
            
            clock->base._vptr->on_start_process_func = clock_on_start;
            clock->base._vptr->run_func = clock_run;
            clock->base._vptr->on_end_process_func = clock_on_stop;            
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
    return stop_async_module((async_module_t *)clock);
}



