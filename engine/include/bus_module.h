#ifndef _BUS_MODULE_H_
#define _BUS_MODULE_H_

#include <pthread.h>
#include <stdint.h>
#include "bus_event.h"
#include "def.h"

#define BUS_MODULE_ID(module)   (module->id)
#define LOCK_MODULE(module)     pthread_mutex_lock(&module->mutex)
#define UNLOCK_MODULE(module)   pthread_mutex_unlock(&module->mutex)


#ifdef __cplusplus
extern "C" {
#endif

bus_module_t*   create_bus_module(uint32_t id, const char *desc);
int32_t     init_bus_module(bus_module_t *module, uint32_t id, const char *desc);
void        destroy_bus_module(bus_module_t *module);

void        set_bus_module_id(bus_module_t *module, int32_t id);
int32_t     set_bus_module_desc(bus_module_t *module, const char *desc);
int32_t     set_bus_module(bus_module_t *module, bus_t *bus);

int32_t     bus_module_dispatch_event(bus_module_t *module, bus_event_t *event, void *param);
int32_t     bus_module_subscribe_event(bus_module_t *module, int32_t event_id);
int32_t     bus_module_unsubscribe_event(bus_module_t *module, int32_t event_id);

#ifdef __cplusplus
}
#endif

#endif

