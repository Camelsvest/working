#ifndef _BUS_MODULE_H_
#define _BUS_MODULE_H_

#include <pthread.h>
#include <stdint.h>
#include "bus_event.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _module_t module_t;

typedef bus_event_t bus_event_list;
struct _module_t {
    struct list_head    list;
    int32_t             id;
    char                *desc;

    pthread_mutex_t     mutex;
    struct list_head    event_list_head;
};

module_t*   create_bus_module(int32_t id, const char *desc);
void        destroy_bus_module(module_t *module);

int32_t     bus_module_dispatch_event(module_t *module, bus_event_t *event);
    
#ifdef __cplusplus
}
#endif

#endif

