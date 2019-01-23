#ifndef _NETIO_MODULE_H_
#define _NETIO_MODULE_H_

#include "async_module.h"

typedef struct _netio_module_t netio_module_t;


netio_module_t* create_netio_module(uint32_t id, const char *desc);
int32_t         init_netio_module(netio_module_t *netio, uint32_t id, const char *desc);
void            destroy_netio_module(netio_module_t *netio);

int32_t         start_netio(netio_module_t *netio);
int32_t         stop_netio(netio_module_t *netio);

#endif
