#include <iostream>
#include "engine.h"
#include "utils/zalloc.h"

Engine::Engine()
    : m_Bus(NULL)
    , m_ClockModule(NULL)
    , m_NetIOModule(NULL)
    , m_Running(false)
{
    int ret = -1;

    ret = logging_init(NULL);
    if (ret != 0)
        throw std::runtime_error("Failed to init logging.");

    ret = zalloc_init();
    if (ret != 0)
        throw std::runtime_error("Failed to init zalloc.");
}

Engine::~Engine()
{
    zalloc_uninit();
    logging_uninit();
}

bool Engine::start()
{
    int32_t ret = -1;

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

    return (ret == 0);
}

void Engine::stop()
{
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
}
