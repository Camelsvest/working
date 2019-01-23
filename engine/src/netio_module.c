#include <assert.h>
#include <unistd.h>
#include "netio_module.h"
#include "utils/zalloc.h"
#include "logging/logging.h"

typedef int32_t (*netio_module_init_func_t)(netio_module_t *, uint32_t id, const char *desc);
typedef void    (*netio_module_uninit_func_t)(bus_module_t *);

typedef struct _netio_module_vtable_t netio_module_vtable_t;
struct _netio_module_vtable_t {
    async_module_uninit_func_t  base_uninit_func;
    netio_module_uninit_func_t  uninit_func;
};


struct _netio_module_t {
    async_module_t              base;
    netio_module_init_func_t    init_func;

    netio_module_vtable_t       *_vptr;
};

static void netio_module_uninit(bus_module_t *module)
{
    netio_module_t *netio_module;

    ENTER_FUNCTION;
    
    if (module != NULL)
    {
        netio_module = (netio_module_t *)module;
        
        // ... to do here
        //
        assert(netio_module->base.running == FALSE);

        // destroy base class finally
        if (netio_module->_vptr->base_uninit_func != NULL)
            netio_module->_vptr->base_uninit_func(module);
        
    }

    EXIT_FUNCTION;
    
}

static netio_module_vtable_t netio_module_vtable = {
    .base_uninit_func = NULL,
    .uninit_func = netio_module_uninit
};

static void netio_on_start(async_module_t *module)
{
    // ... to do here
    logging_trace("NetIO is staring ... \r\n");
    on_start_async_module(module);
}

static void netio_on_stop(async_module_t *module)
{
    // ... to do here
    on_stop_async_module(module);
    logging_trace("NetIO is stopping ... \r\n");    
}

static int32_t netio_run(async_module_t *module)
{
    netio_module_t *netio;

    netio = (netio_module_t *)module;
    assert(netio != NULL);


    usleep(10000);
    
    return 0;
}

static int32_t netio_module_init(netio_module_t *module, uint32_t id, const char *desc)
{
    int32_t ret = -1;

    ENTER_FUNCTION;

    // call construction of base class
    if (module != NULL) {

        ret = init_async_module(&module->base, id, desc);
        if (ret == 0)
        {
            module->_vptr = &netio_module_vtable;
            
            module->_vptr->base_uninit_func = module->base._vptr->uninit_func;

            // override base class's function uninit, so permit to delete obeject from base classes
            module->base.base._vptr->uninit_func = netio_module_uninit;
            
            module->base._vptr->on_start_process_func = netio_on_start;
            module->base._vptr->run_func = netio_run;
            module->base._vptr->on_end_process_func = netio_on_stop;
        }
    }

    EXIT_FUNCTION;

    return ret;
}

int32_t init_netio_module(netio_module_t *netio, uint32_t id, const char *desc)
{
    int32_t ret = -1;

    ENTER_FUNCTION;
    
    if (netio != NULL)
    {
        netio->init_func = netio_module_init;
        ret = netio->init_func(netio, id, desc);
    }

    EXIT_FUNCTION;

    return ret;
}


netio_module_t* create_netio_module(uint32_t id, const char *desc)
{
    netio_module_t *module = NULL;

    ENTER_FUNCTION;
    
    module = (netio_module_t *)zmalloc(sizeof(netio_module_t));
    if (init_netio_module(module, id, desc) != 0)
    {
        zfree(module);
        module = NULL;
    }

    EXIT_FUNCTION;

    return module;
}

void destroy_netio_module(netio_module_t *netio)
{
    ENTER_FUNCTION;
    
    if (netio != NULL)
    {
        // ... to do here
        //

        if (netio->_vptr != NULL && netio->_vptr->uninit_func != NULL)
            netio->_vptr->uninit_func((bus_module_t *)netio);

        zfree(netio);
    }

    EXIT_FUNCTION;
}

int32_t start_netio(netio_module_t *netio)
{
    return start_async_module(&netio->base);
    
}

int32_t stop_netio(netio_module_t *netio)
{
    return stop_async_module(&netio->base);
}


