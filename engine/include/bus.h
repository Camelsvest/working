#ifndef _BUS_H_
#define _BUS_H_

#include <stdint.h>
#include "bus_module.h"
#include "bus_event.h"
#include "def.h"


#ifdef __cplusplus
extern "C" {
#endif

bus_t*      create_bus(uint32_t module_count);
void        destroy_bus(bus_t *bus);

int32_t    bus_alloc_module_id(bus_t *bus);

int32_t     bus_attach_module(bus_t *bus, bus_module_t *module);
int32_t     bus_detach_module(bus_t *bus, bus_module_t *module);

int32_t		bus_dispatch_event(bus_t *bus, bus_event_t *event);
int32_t		bus_dispatch_module_event(bus_t *bus, bus_module_t *module, bus_event_t *event);

int32_t     start_bus(bus_t *bus);
int32_t     stop_bus(bus_t *bus);

BOOL        is_bus_running(bus_t *bus);


#ifdef __cplusplus
}
#endif

#endif
