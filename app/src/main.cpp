#include <stdio.h>
#include <iostream>
#include "app.h"

int main(int argc, char *argv[])
{
    App *app;
    bool running = false;

    int ret = -1;

    ret = logging_init(NULL);
    ret = zalloc_init();

    try
    {
        app = new App();
        running = app->start();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    fprintf(stdout, "press any key to quit.\r\n");
    getchar();
    
    if (running)
        app->stop();

    delete app;

    zalloc_uninit();
    logging_uninit();

    fprintf(stdout, "\r\n");
    
    return 0;
}
