#include <iostream>
#include "engine.h"
#include "utils/zalloc.h"

#if defined(ZALLOC_DEBUG) || defined(DEBUG) || defined(_DEBUG)
#define new zdebug_new
#endif


Engine::Engine()
    : Object("Engine")
    , m_Bus(NULL)
    , m_ClockModule(NULL)
    , m_NetIOModule(NULL)
    , m_Running(false)
{

}

Engine::~Engine()
{

}

bool Engine::start()
{
    int32_t ret = -1;

    ENTER_FUNCTION;
    
    if (!m_Running)
    {
        try
        {
            m_Bus = new Bus;
			if (m_Bus->start())
			{
                m_ClockModule = new ClockModule;
                m_NetIOModule = new NetIOModule;

                ret = m_Bus->attachModule(m_ClockModule);
                if (ret == 0)
                    ret = m_Bus->attachModule(m_NetIOModule);

                if (ret == 0)
                    m_Running = true;
			}
        }
        catch(const std::exception& e)
        {
            ret = -1;
            std::cerr << e.what() << '\n';
        }
    }

    EXIT_FUNCTION;
    
    return (ret == 0);
}

void Engine::stop()
{
    ENTER_FUNCTION;
    
    if (m_Running)
    {
        m_Bus->stop();
        delete m_Bus;
        m_Bus = NULL;

        delete m_ClockModule;
        m_ClockModule = NULL;

        delete m_NetIOModule;
        m_NetIOModule = NULL;

        m_Running = false;
    }

    EXIT_FUNCTION;
}
