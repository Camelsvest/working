#ifndef _BUS_H_
#define _BUS_H_

#include <stdint.h>
#include "bus_module.h"
#include "bus_event.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _bus_t       bus_t;

bus_t*      create_bus(uint32_t module_count);
void        destroy_bus(bus_t *bus);

int32_t     bus_attach_module(bus_t *bus, bus_module_t *module);
int32_t     bus_detach_module(bus_t *bus, bus_module_t *module);

int32_t     bus_subscribe_event(bus_t *bus, bus_module_t *module, bus_event_t *event);
int32_t     bus_unsubscribe_event(bus_t *bus, bus_module_t *module, bus_event_t *event);

int32_t		bus_dispatch_event(bus_t *bus, bus_event_t *event, void *param);
int32_t		bus_dispatch_module_event(bus_t *bus, bus_module_t *module, bus_event_t *event, void *param);

int32_t     bus_start(bus_t *bus);
int32_t     bus_stop(bus_t *bus);


#ifdef __cplusplus
}
#endif

#endif
