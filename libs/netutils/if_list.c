#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <arpa/inet.h>

#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/if_link.h>
#include "if_list.h"


iflist_t* create_iflist()
{
	iflist_t *list;

	list = (iflist_t *)calloc(1, sizeof(iflist_t));
	if (list)
		INIT_LIST_HEAD(&list->list);

	return list;
}

void destroy_iflist(iflist_t *iflist)
{
	iflist_t *pos, *next;

	list_for_each_entry_safe(pos, next, &iflist->list, list)
	{
		list_del(&pos->list);
		free(pos);
	}

	free(iflist);
}

iflist_t* get_iflist(int family)
{
    struct ifaddrs *ifaddr, *ifa;
    iflist_t *node, *head;
    int length;


    if (getifaddrs(&ifaddr) == -1)
        return NULL;

    head = create_iflist();
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        if (family == ifa->ifa_addr->sa_family)
        {
            node = create_iflist();
            if (node)
            {
                node->family = ifa->ifa_addr->sa_family;

                length = strlen(ifa->ifa_name);
                if (length >= sizeof(node->name))
                    length = sizeof(node->name) - 1;
                
                node->name[length] = '\0';
                strncpy(node->name, ifa->ifa_name, length);

                node->addr = *ifa->ifa_addr;

                list_add(&node->list, &head->list);
            }
        }
    }

    freeifaddrs(ifaddr);

    return head;
}

void print_iflist(iflist_t *iflist)
{
    iflist_t *pos;
    int s;
    char host[NI_MAXHOST];

    list_for_each_entry(pos, &iflist->list, list)
    {
        printf("%-8s %s (%d)\n",
            pos->name,
            (pos->family == AF_PACKET) ? "AF_PACKET" :
            (pos->family == AF_INET) ? "AF_INET" :
            (pos->family == AF_INET6) ? "AF_INET6" : "???",
            pos->family);

        if (pos->family == AF_INET || pos->family == AF_INET6)
        {
            s = getnameinfo(&pos->addr, 
                            (pos->family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                return;
            }

            printf("\t\taddress: <%s>\n", host);
        }
    }
}
