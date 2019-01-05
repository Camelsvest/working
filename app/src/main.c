#include <stdio.h>
#include "app.h"

int main(int argc, char *argv[])
{
    if (start_app() != 0)
    {
        fprintf(stderr, "Failed to start app.\r\n");
        return -1;
    }

    fprintf(stdout, "press any key to quit.\r\n");
    getchar();
    
    stop_app();

    fprintf(stdout, "\r\n");
    
    return 0;
}
