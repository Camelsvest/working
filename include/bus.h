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

int32_t     bus_add_module(bus_t *bus, const char *desc);
int32_t     bus_del_module(bus_t *bus, int32_t id);

int32_t     bus_register_event(bus_t *bus, int32_t module_id, bus_event_t *event);
int32_t     bus_unregister_event(bus_t *bus, int32_t module_id, bus_event_t *event);

int32_t     bus_start(bus_t *bus);
int32_t     bus_stop(bus_t *bus);


#ifdef __cplusplus
}
#endif

#endif
