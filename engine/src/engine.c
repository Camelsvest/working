#include "engine.h"
#include "utils/zalloc.h"

typedef struct _engine_vtable_t engine_vtable_t;
typedef int32_t (*engine_init_func_t)(engine_t *engine);
typedef void 	(*engine_uninit_func_t)(engine_t *engine);

struct _engine_t {
	bus_t			*bus;
	netio_module_t	*netio;	

	engine_init_func_t init_func;
	engine_vtable_t *_vptr;
};

struct _engine_vtable_t {
	engine_uninit_func_t uninit_func;
};


static int32_t engine_init(engine_t *engine)
{	
    int32_t id;
	int32_t ret = -1;


	if (engine != NULL)
	{
		engine->bus = create_bus(8);
		if (engine->bus != NULL)
		{
            id = bus_alloc_module_id(engine->bus);
            if (id != -1)   // INVALID_MODULE_ID
            {
    			engine->netio = create_netio_module(id, "NETIO");
    			if (engine->netio != NULL)
    				ret = 0;
            }
		}
	}

	return ret;
}

static void engine_uninit(engine_t *engine)
{
	if (engine != NULL)
	{
		if (engine->netio != NULL)
			destroy_netio_module(engine->netio);

		if (engine->bus != NULL)
			destroy_bus(engine->bus);		
	}
	
	return;
}

static engine_vtable_t engine_vtable = {
	.uninit_func = engine_uninit
};

engine_t* create_engine()
{
    engine_t *engine;

    engine = (engine_t *)zmalloc(sizeof(engine_t));
	if (engine != NULL)
	{
		engine->init_func = engine_init;
		engine->_vptr = &engine_vtable;

		engine->init_func(engine);
	}

	
    return engine;
}

void destroy_engine(engine_t *engine)
{
    if (engine != NULL)
    {
        if (engine->_vptr != NULL && engine->_vptr->uninit_func != NULL)
            engine->_vptr->uninit_func(engine);

        zfree(engine);
    }
}

int32_t start_engine(engine_t *engine)
{
    int32_t ret = -1;
    
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
            }
        }
    }

    return ret;
}

int32_t stop_engine(engine_t *engine)
{
    int32_t ret = -1;

    if (engine != NULL)
    {
        if (is_bus_running(engine->bus) == TRUE)
        {
            stop_bus(engine->bus);
            stop_netio(engine->netio);

            ret = 0;
        }
    }

    return ret;
}

int32_t subcribe_event(engine_t *engine, bus_module_t *module, bus_event_t *event)
{
    int32_t ret = -1;
    
    if (engine != NULL)
    {
        ret = bus_subscribe_event(engine->bus, module, event);
    }
    
    return ret;
}

int32_t unsubscribe_event(engine_t *engine, bus_module_t *module, bus_event_t *event)
{
    int32_t ret = -1;

    if (engine != NULL)
    {
        ret = bus_unsubscribe_event(engine->bus, module, event);
    }

    return ret;
}

int32_t	dispatch_event(engine_t *engine, bus_event_t *event, void *param)
{
    int32_t ret = -1;

    if (engine != NULL)
    {
        ret = bus_dispatch_event(engine->bus, event, param);
    }

    return ret;
}


