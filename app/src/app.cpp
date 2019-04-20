#include "app.h"
#include "engine.h"
#include "utils/zalloc.h"
#include "logging/logging.h"

App::App()
	: m_Engine(NULL)
{
	
}

App::~App()
{
	if (isRunning())
		stop();
}

bool App::isRunning()
{
	if (m_Engine)
		return m_Engine->isRunning();
	else
	{
		return false;
	}
	
}

bool App::start()
{
	bool succeed = false;

	if (!isRunning())
	{
		m_Engine = new Engine;
		if (m_Engine)
		{
			succeed = m_Engine->start();
		}
	}

	return succeed;
}

void App::stop()
{
	if (isRunning())
	{
		m_Engine->stop();
		delete m_Engine;
	}
}