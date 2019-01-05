#ifndef _ASYNC_MODULE_H_
#define _ASYNC_MODULE_H_

#include "bus_module.h"

typedef struct _async_module_t async_module_t;
typedef int32_t (*async_module_init_func_t)(async_module_t *module, uint32_t id, const char *desc);
typedef void    (*async_module_uninit_func_t)(bus_module_t *module);
typedef void    (*async_module_on_start_process_func_t)(async_module_t *module);
typedef int32_t (*async_module_run_func_t)(async_module_t *module);
typedef void    (*async_module_on_end_process_func_t)(async_module_t *module);


typedef struct _async_module_vtable_t async_module_vtable_t;
struct _async_module_vtable_t {
    module_uninit_func_t       base_uninit_func;
    async_module_uninit_func_t uninit_func;

    async_module_on_start_process_func_t    on_start_process_func;
    async_module_run_func_t                 run_func;
    async_module_on_end_process_func_t      on_end_process_func;
};

struct _async_module_t {
    bus_module_t    base;
    pthread_t       thread_id;
    char            running;            // 0 -- false; 1 -- true
    int32_t         thread_result;

    async_module_init_func_t    init_func;
    async_module_vtable_t       *_vptr;
};


#ifdef __cplusplsu
extern "C" {
#endif

async_module_t* create_async_module(uint32_t id, const char *desc);
int32_t         init_async_module(async_module_t *module, uint32_t id, const char *desc);
void            destroy_async_module(async_module_t *module);

int32_t         start_async_module(async_module_t *module);
void            on_start_async_module(async_module_t *module);
void            on_stop_async_module(async_module_t *module);
int32_t         stop_async_module(async_module_t *module);

#ifdef __cplusplus
}
#endif


#endif
