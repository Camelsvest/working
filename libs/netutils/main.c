#include <stdio.h>
#include "if_list.h"

int main(int argc, char *argv[])
{
    iflist_t *iflist;

    iflist = get_iflist(AF_INET);

    if (iflist)
    {
        print_iflist(iflist);
    }

    destroy_iflist(iflist);

    return 0;
}
