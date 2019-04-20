#include <assert.h>
#include <unistd.h>
#include "netio_module.h"
#include "utils/zalloc.h"
#include "logging/logging.h"

NetIO::NetIO()
{

}

NetIO::~NetIO()
{

}

void NetIO::runOnce()
{
    
}

NetIOModule::NetIOModule()
{
    bool succeed = false;

    m_NetIO = new NetIO;
    if (m_NetIO)
    {
        succeed = m_NetIO->start();
    }

    if (!succeed)
        throw std::runtime_error("Failed to construct object NetIOModule.");
}

NetIOModule::~NetIOModule()
{
    if (m_NetIO)
    {
        m_NetIO->stop();
        delete m_NetIO;
    }
}

int NetIOModule::activateEvent(BusEvent *event)
{
    return -1;
}