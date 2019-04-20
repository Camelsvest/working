#ifndef _NETIO_MODULE_H_
#define _NETIO_MODULE_H_

#include "bus.h"

class NetIO : public Thread
{
public:
    NetIO();
    virtual ~NetIO();

protected:
    virtual void runOnce();
};

class NetIOModule : public BusModule
{
public:
    NetIOModule();
    virtual ~NetIOModule();

protected:
    virtual int activateEvent(BusEvent *event);

private:
    NetIO *m_NetIO;
};

#endif
