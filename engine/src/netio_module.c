#include "netio_module.h"
#include "utils/zalloc.h"

typedef int32_t (*netio_module_init_func_t)(netio_module_t *, uint32_t id, const char *desc);
typedef void    (*netio_module_uninit_func_t)(bus_module_t *);

typedef struct _netio_module_vtable_t netio_module_vtable_t;
struct _netio_module_vtable_t {
    module_uninit_func_t        module_uninit_func;
    netio_module_uninit_func_t  netio_uninit_func;
};


struct _netio_module_t {
    bus_module_t                base;
    netio_module_init_func_t    init_func;

    netio_module_vtable_t       *_vptr;
};

static void netio_module_uninit(bus_module_t *module)
{
    netio_module_t *netio_module;
    
    if (module != NULL)
    {
        netio_module = (netio_module_t *)module;
        
        // ... to do here
        //

        // destroy base class finally
        if (netio_module->_vptr->module_uninit_func != NULL)
            netio_module->_vptr->module_uninit_func(&netio_module->base);
        
    }
}

static netio_module_vtable_t netio_module_vtable = {
    .module_uninit_func = NULL,
    .netio_uninit_func = netio_module_uninit
};

static int32_t netio_module_init(netio_module_t *module, uint32_t id, const char *desc)
{
    int32_t ret = -1;

    // call construction of base class
    if (module != NULL) {

        ret = init_bus_module(&module->base, id, desc);
        if (ret == 0)
        {
            module->_vptr = &netio_module_vtable;
            
            module->_vptr->module_uninit_func = module->base._vptr->uninit_func;

            // override base class's function uninit, so permit to delete obeject from base classes
            module->base._vptr->uninit_func = netio_module_uninit;
        }
    }

    return ret;
}

int32_t init_netio_module(netio_module_t *netio, uint32_t id, const char *desc)
{
    int32_t ret = -1;
    
    if (netio != NULL)
    {
        netio->init_func = netio_module_init;
        ret = netio->init_func(netio, id, desc);
    }

    return ret;
}


netio_module_t* create_netio_module(uint32_t id, const char *desc)
{
    netio_module_t *module = NULL;

    module = (netio_module_t *)zmalloc(sizeof(netio_module_t));
    if (init_netio_module(module, id, desc) != 0)
    {
        zfree(module);
        module = NULL;
    }

    return module;
}

void destroy_netio_module(netio_module_t *netio)
{
    if (netio != NULL)
    {
        // ... to do here
        //

        if (netio->_vptr != NULL)
            netio->_vptr->netio_uninit_func((bus_module_t *)netio);

        zfree(netio);
    }
}

