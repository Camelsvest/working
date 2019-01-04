#ifndef _BUS_MODULE_H_
#define _BUS_MODULE_H_

#include <pthread.h>
#include <stdint.h>
#include "bus_event.h"

#define BUS_MODULE_ID(module)   (module->id)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _module_t bus_module_t;

typedef bus_event_t bus_event_list;

typedef int32_t (*module_init_func_t)(bus_module_t *module, int32_t id, const char *desc);
typedef void (*module_uninit_func_t)(bus_module_t *module);

typedef struct _bus_module_vtable_t  bus_module_vtable_t;
struct _bus_module_vtable_t {
    module_uninit_func_t uninit_func;    
};


struct _module_t {
    struct list_head        list;
    int32_t                 id;
    char                    *desc;

    pthread_mutex_t         mutex;
    struct list_head        event_list_head;

    module_init_func_t      init_func;

    bus_module_vtable_t     *_vptr;
};

bus_module_t*   create_bus_module(int32_t id, const char *desc);
void        destroy_bus_module(bus_module_t *module);

void        set_bus_module_id(bus_module_t *module, int32_t id);
int32_t     set_bus_module_desc(bus_module_t *module, const char *desc);

int32_t     bus_module_dispatch_event(bus_module_t *module, bus_event_t *event, void *param);
void		bus_module_subscribe_event(bus_module_t *module, bus_event_t* event);
    
#ifdef __cplusplus
}
#endif

#endif

