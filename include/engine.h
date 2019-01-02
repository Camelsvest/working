#ifndef _ENGINE_H
#define _ENGINE_H

#include <stdint.h>
#include "bus_event.h"

typedef struct _engine_t engine_t;

#ifdef __cplusplus
extern "C" {
#endif


engine_t*	create_engine();
void		destroy_engine(engine_t *engine);       

int32_t     initiate(engine_t * engine);


int32_t     register_event(engine_t *engine, bus_event_t *event);

int32_t     unregister_event1(engine_t *engine, bus_event_t *event);
int32_t     unregister_event2(engine_t *engine, int32_t event_id);



#ifdef __cplusplus
}
#endif

#endif
