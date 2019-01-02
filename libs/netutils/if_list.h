#ifndef _IF_LIST_H_
#define _IF_LIST_H_

#include <sys/socket.h>
#include <netdb.h>
#include "linux-like-list/list.h"

struct  _iflist {
  struct list_head   list;
  char               name[32];
  int	             family;
  struct sockaddr    addr;
};
typedef struct _iflist iflist_t;


#ifdef __cplusplus
extern "C" {
#endif

iflist_t*   create_iflist();

void        destroy_iflist(iflist_t *iflist);

/**
 * get_iflist - get network interface list according to socket familiy
 * @family: AF_INET, AF_INET6, AF_PACKET
 */
iflist_t*   get_iflist(int family);

void        print_iflist(iflist_t *iflist);

#ifdef __cplusplus
}
#endif

#endif