#include <stdio.h>
#include "app.h"

int main(int argc, char *argv[])
{
    if (app_start() != 0)
    {
        fprintf(stderr, "Failed to start app.\r\n");
        return -1;
    }

    fprintf(stdout, "press any key to quit.");
    getchar();
    
    app_stop();

    fprintf(stdout, "\r\n");
    
    return 0;
}
