#include "engine.h"
#include "utils/zalloc.h"
#include "clock_module.h"

typedef struct _engine_vtable_t engine_vtable_t;
typedef int32_t (*engine_init_func_t)(engine_t *engine);
typedef void 	(*engine_uninit_func_t)(engine_t *engine);

struct _engine_t {
	bus_t			*bus;
	netio_module_t	*netio;
    clock_module_t  *clock;

	engine_init_func_t init_func;
	engine_vtable_t *_vptr;
};

struct _engine_vtable_t {
	engine_uninit_func_t uninit_func;
};


static int32_t engine_init(engine_t *engine)
{	
    int32_t netio_id, clock_id;
	int32_t ret = 0;

    ENTER_FUNCTION;
    
	if (engine != NULL)
	{
		engine->bus = create_bus(8);
		if (engine->bus != NULL)
		{
            netio_id = bus_alloc_module_id(engine->bus);
            clock_id = bus_alloc_module_id(engine->bus);
            if (netio_id != -1 && clock_id != -1)
            {
                engine->netio = create_netio_module(netio_id, "NETIO");
                engine->clock = create_clock(clock_id, "CLOCK");

                if (engine->netio != NULL && engine->clock != NULL)
                {
                    ret = bus_attach_module(engine->bus, (bus_module_t *)engine->netio);
                    if (ret == 0)
                        ret = bus_attach_module(engine->bus, (bus_module_t *)engine->clock);
                }
                
                if (ret != 0)
                {
                    if (engine->netio != NULL)
                        destroy_netio_module(engine->netio);

                    if (engine->clock != NULL)
                        destroy_clock(engine->clock);
                }
            }

            if (ret != 0)
            {
                destroy_bus(engine->bus);
                engine->bus = NULL;
            }

		}
	}

    EXIT_FUNCTION;
    
	return ret;
}

static void engine_uninit(engine_t *engine)
{
    ENTER_FUNCTION;
    
	if (engine != NULL && engine->bus != NULL)
	{
        if (engine->netio != NULL)
        {
            bus_detach_module(engine->bus, (bus_module_t *)engine->netio);
			destroy_netio_module(engine->netio);
            engine->netio = NULL;
        }

        if (engine->clock != NULL)
        {
            bus_detach_module(engine->bus, (bus_module_t *)engine->clock);
            destroy_clock(engine->clock);
            engine->clock = NULL;
        }

        destroy_bus(engine->bus);
        engine->bus = NULL;
	}

    EXIT_FUNCTION;
    
	return;
}

static engine_vtable_t engine_vtable = {
	.uninit_func = engine_uninit
};

engine_t* create_engine()
{
    engine_t *engine;

    ENTER_FUNCTION;
    
    engine = (engine_t *)zmalloc(sizeof(engine_t));
	if (engine != NULL)
	{
		engine->init_func = engine_init;
		engine->_vptr = &engine_vtable;

		if (engine->init_func(engine) != 0)
		{
            zfree(engine);
            engine = NULL;
		}
	}

	EXIT_FUNCTION;
    
    return engine;
}

void destroy_engine(engine_t *engine)
{
    ENTER_FUNCTION;
    
    if (engine != NULL)
    {
        if (engine->_vptr != NULL && engine->_vptr->uninit_func != NULL)
            engine->_vptr->uninit_func(engine);

        zfree(engine);
    }

    EXIT_FUNCTION;
}

int32_t start_engine(engine_t *engine)
{
    int32_t ret = -1;

    ENTER_FUNCTION;
    
    if (engine != NULL)
    {
        if (is_bus_running(engine->bus) == FALSE)
        {
            ret = start_bus(engine->bus);
            if (ret == 0)
            {
                ret = start_netio(engine->netio);
                if (ret != 0)
                    stop_bus(engine->bus);
                else
                {
                    ret = start_clock(engine->clock);

                    if (ret != 0)
                    {
                        stop_netio(engine->netio);
                        stop_bus(engine->bus);
                    }
                }
            }
        }
    }

    EXIT_FUNCTION;
    
    return ret;
}

int32_t stop_engine(engine_t *engine)
{
    int32_t ret = -1;

    ENTER_FUNCTION;
    
    if (engine != NULL)
    {
        if (is_bus_running(engine->bus) == TRUE)
        {
            stop_bus(engine->bus);
            stop_netio(engine->netio);
            stop_clock(engine->clock);

            ret = 0;
        }
    }

    EXIT_FUNCTION;
    
    return ret;
}

int32_t	dispatch_event(engine_t *engine, bus_event_t *event)
{
    int32_t ret = -1;

    if (engine != NULL)
    {
        ret = bus_dispatch_event(engine->bus, event);
    }

    return ret;
}


