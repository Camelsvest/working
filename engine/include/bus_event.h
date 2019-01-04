#ifndef _ENGINE_EVENT_H_
#define _ENGINE_EVENT_H_

#include <stdint.h>
#include <pthread.h>
#include "linux-like-list/list.h"
#include "def.h"

#define LOCK_EVENT(event)	pthread_mutex_lock(event->mutex)
#define UNLOCK_EVENT(event)	pthread_mutex_unlock(event->mutex);

#ifdef __cplusplus
extern "C" {
#endif

bus_event_t*    create_bus_event(int32_t id, const char *desc, void *data);
void            destroy_bus_event(bus_event_t * event);

int32_t			bus_event_addref(bus_event_t *event);
int32_t			activate_bus_event(bus_event_t *event, void *data);

#ifdef __cplusplus
}
#endif

#endif
