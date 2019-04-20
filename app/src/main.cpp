#include <stdio.h>
#include <iostream>
#include "app.h"

int main(int argc, char *argv[])
{
    App *app;
    bool running = false;

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

    fprintf(stdout, "\r\n");
    
    return 0;
}
