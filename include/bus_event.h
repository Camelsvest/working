#ifndef _ENGINE_EVENT_H_
#define _ENGINE_EVENT_H_

#include <stdint.h>
#include "linux-like-list/list.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct _bus_event_t bus_event_t;
typedef void (*event_callback_t)(bus_event_t *event, void *param);

struct _bus_event_t {
    struct list_head    list;
    int32_t             id;
    char                *desc;
    
    event_callback_t    callback;
    void                *data;
};



bus_event_t*    create_bus_event(int32_t id, const char *desc, event_callback_t callback, void *data);
void            destroy_bus_event(bus_event_t * event);

#ifdef __cplusplus
}
#endif

#endif
