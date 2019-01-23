#ifndef _CLOCK_MODULE_H_
#define _CLOCK_MODULE_H_

#include "async_module.h"

typedef struct _clock_module_t clock_module_t;

#ifdef __cplusplus
extern "C" {
#endif

clock_module_t*     create_clock(uint32_t id, const char *desc);
void                destroy_clock(clock_module_t *clock);

int32_t     start_clock(clock_module_t *clock);
int32_t     stop_clock(clock_module_t *clock);

#ifdef __cplusplus
}
#endif

#endif
