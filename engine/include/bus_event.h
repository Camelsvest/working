#ifndef _ENGINE_EVENT_H_
#define _ENGINE_EVENT_H_

#include <stdint.h>
#include <pthread.h>
#include "linux-like-list/list.h"
#include "def.h"

#ifdef __cplusplus
extern "C" {
#endif

bus_event_t*    create_bus_event(int32_t id, const char *desc, void *data);

int32_t			bus_event_addref(bus_event_t *event);
int32_t         bus_event_release(bus_event_t *event);

#ifdef __cplusplus
}
#endif

#endif
