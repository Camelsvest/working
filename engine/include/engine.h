#ifndef _ENGINE_H
#define _ENGINE_H

#include <stdint.h>
#include "bus.h"
#include "clock_module.h"
#include "netio_module.h"

class Engine
{
public:
    Engine();
    virtual ~Engine();

    bool    start();
    void    stop();
    bool    isRunning() { return m_Running; }

private:
    Bus*            m_Bus;
    ClockModule*    m_ClockModule;
    NetIOModule*    m_NetIOModule;

    bool            m_Running;
};

#endif
