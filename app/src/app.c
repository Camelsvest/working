#include "app.h"
#include "engine.h"
#include "utils/zalloc.h"
#include "logging/logging.h"

typedef struct _app_t app_t;

typedef int32_t (*app_init_func_t)(app_t *app);
typedef void	(*app_uninit_func_t)(app_t *app);

typedef struct _app_vtable_t app_vtable_t;
struct _app_vtable_t {
	app_uninit_func_t uninit_func;
};

struct _app_t {
	engine_t *engine;

	app_init_func_t init_func;
	app_vtable_t	*_vptr;
};

static int32_t app_init(app_t *app)
{
	int32_t ret = -1;

	app->engine = create_engine();
	if (app->engine != NULL)
		ret = 0;

	return ret;
}

static void app_uninit(app_t *app)
{
	if (app != NULL)
	{
		destroy_engine(app->engine);
	}
}

static app_t *instance = NULL;
static struct _app_vtable_t app_vtable = {
	.uninit_func = app_uninit
};

int start_app()
{
	int ret = -1;
	
	if (instance == NULL)
	{
	        logging_init(NULL);
		ret = zalloc_init();
                if (ret == 0)
                {
    		        instance = (app_t *)zmalloc(sizeof(app_t));
             		instance->init_func = app_init;
    	        	instance->_vptr = &app_vtable;

    		        ret = instance->init_func(instance);
                        if (ret == 0)
                                ret = start_engine(instance->engine);
                }
	}
	
    return ret;
}

void stop_app()
{
	if (instance != NULL)
	{
        stop_engine(instance->engine);
        
		if (instance->_vptr != NULL && instance->_vptr->uninit_func != NULL)
			instance->_vptr->uninit_func(instance);

		zfree(instance);
		instance = NULL;

		zalloc_uninit();
        logging_uninit();
	}
}
