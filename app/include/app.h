#ifndef _APP_H_
#define _APP_H_

#include "engine.h"

class App
{
public:
    App();
    virtual ~App();

    bool    start();
    void    stop();

    bool    isRunning();

private:
    Engine* m_Engine;
};

#endif
