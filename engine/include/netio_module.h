#ifndef _NETIO_MODULE_H_
#define _NETIO_MODULE_H_

#include "bus_module.h"

typedef struct _netio_module netio_module_t;
struct _netio_module
{
    bus_module_t    base;
};

netio_module_t* create_netio_module();
void            destroy_netio_module(netio_module_t *netio);

#endif
