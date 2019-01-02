#ifndef _ENGINE_EVENT_H_
#define _ENGINE_EVENT_H_

#include <stdint.h>
#include "linux-like-list/list.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct _bus_event_t         bus_event_t;
typedef int32_t (*event_init_func_t)(bus_event_t *event, int32_t id, const char *desc, void *data);
typedef void    (*event_callback_t)(bus_event_t *event, void *param);
typedef void    (*event_uninit_func_t)(bus_event_t *event);

typedef struct _bus_event_vtable_t  bus_event_vtable_t;
struct _bus_event_vtable_t {
    event_callback_t    callback;
    event_uninit_func_t uninit_func;    
};


struct _bus_event_t {
    struct list_head    list;
    int32_t             id;
    void                *data;
    
    char                *desc;
    
    event_init_func_t   init_func;
    bus_event_vtable_t  *_vptr;
};



bus_event_t*    create_bus_event(int32_t id, const char *desc, void *data);
void            destroy_bus_event(bus_event_t * event);

#ifdef __cplusplus
}
#endif

#endif
