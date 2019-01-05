#include <assert.h>
#include <pthread.h>
#include "utils/zalloc.h"
#include "async_module.h"
#include "logging/logging.h"

static void async_module_uninit(bus_module_t *module)
{
    async_module_t *async_module;

    if (module != NULL)
    {
        async_module = (async_module_t *)module;

        // ...  to do here
        // 

        if (async_module->_vptr->base_uninit_func != NULL)
            async_module->_vptr->base_uninit_func(&async_module->base);
    }
}

static void async_module_on_start(async_module_t *module)
{
    on_start_async_module(module);
}

static void async_module_on_stop(async_module_t *module)
{
    on_stop_async_module(module);
}

static int32_t async_module_run(async_module_t *module)
{
    assert(module != NULL);
    logging_error("async_module(%ul) run !!! You must override it in derived class.\r\n", module->thread_id);

    return -1;
}

static async_module_vtable_t async_module_vtable = {
    .base_uninit_func = NULL,
    .uninit_func = async_module_uninit,
    .on_start_process_func = async_module_on_start,
    .run_func = async_module_run,
    .on_end_process_func = async_module_on_stop
};

static int32_t async_module_init(async_module_t *module, uint32_t id, const char *desc)
{
    int32_t ret = -1;
    
    if (module != NULL)
    {
        ret = init_bus_module(&module->base, id, desc);

        if (ret == 0)
        {
            module->thread_id = 0;
            module->running = FALSE;
            module->thread_result = 0;
            module->_vptr = &async_module_vtable;
            module->_vptr->base_uninit_func = module->base._vptr->uninit_func;
            
            // override
            module->base._vptr->uninit_func = async_module_uninit;
        }
    }

    return ret;
}

int32_t init_async_module(async_module_t *module, uint32_t id, const char *desc)
{
    int32_t ret = -1;

    if (module != NULL)
    {
        module->init_func = async_module_init;
        ret = module->init_func(module, id, desc);
    }

    return ret;
}

async_module_t* create_async_module(uint32_t id, const char *desc)
{    
    async_module_t *module;
    int32_t ret = -1;

    module = (async_module_t *)zmalloc(sizeof(async_module_t));
    if (module != NULL)
    {
        ret = init_async_module(module, id, desc);
        if (ret != 0)
        {
            zfree(module);
            module = NULL;
        }
    }

    return module;
}


void destroy_async_module(async_module_t *module)
{
    if (module != NULL)
    {
        // to do here....
        //

        if (module->_vptr != NULL && module->_vptr->uninit_func != NULL)
            module->_vptr->uninit_func((bus_module_t *)module);

        zfree(module);
    }
}

static void* async_process(void *data)
{
    async_module_t *module;

    module = (async_module_t *)data;
    if (module != NULL)
    {
        assert(module->_vptr != NULL);

        if (module->_vptr->on_start_process_func != NULL)
            module->_vptr->on_start_process_func(module);
        
        LOCK_MODULE((&module->base));
        while ((module->running == TRUE) && (module->thread_result == 0))
        {
            UNLOCK_MODULE((&module->base));

            module->thread_result = module->_vptr->run_func(module);

            LOCK_MODULE((&module->base));
        }
        UNLOCK_MODULE((&module->base));

        if (module->_vptr->on_end_process_func != NULL)
            module->_vptr->on_end_process_func(module);
    }
    
    return &module->thread_result;
}

int32_t start_async_module(async_module_t *module)
{
    int32_t ret = -1;
    BOOL is_running = FALSE;
    
    if (module != NULL)
    {
        LOCK_MODULE((&module->base));
        is_running = module->running;
        UNLOCK_MODULE((&module->base));

        if (is_running == FALSE)
        {
            ret = pthread_create(&module->thread_id, NULL, async_process, module);
        }
    }

    return ret;
}

int32_t stop_async_module(async_module_t *module)
{
    int32_t ret = -1;
    BOOL is_running = FALSE;    
    
    if (module != NULL)
    {
        LOCK_MODULE((&module->base));
        is_running = module->running;
        module->running = FALSE;
        UNLOCK_MODULE((&module->base));

        if (is_running == TRUE)
            ret = pthread_join(module->thread_id, NULL);
    }

    return ret;
}

void on_start_async_module(async_module_t *module)
{
    LOCK_MODULE((&module->base));
    module->running = TRUE;
    UNLOCK_MODULE((&module->base));    
}

void on_stop_async_module(async_module_t *module)
{
    LOCK_MODULE((&module->base));
    module->running = FALSE;    // set again
    UNLOCK_MODULE((&module->base));          
}

