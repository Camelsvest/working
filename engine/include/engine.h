#ifndef _ENGINE_H
#define _ENGINE_H

#include <stdint.h>
#include "bus.h"
#include "netio_module.h"


typedef struct _engine_t engine_t;

#ifdef __cplusplus
extern "C" {
#endif


engine_t*	create_engine();
void		destroy_engine(engine_t *engine);

int32_t     start_engine(engine_t *engine);
int32_t     stop_engine(engine_t *engine);

int32_t		dispatch_event(engine_t *engine, bus_event_t *event, void *param);


#ifdef __cplusplus
}
#endif

#endif
