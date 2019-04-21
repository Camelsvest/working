#ifndef _THREAD_H_
#define _THREAD_H_

#include <string>
#include "object.h"

class Thread : public Object {
public:
    Thread(const char *thread_name = NULL);

    bool start();
    bool stop();
    bool isRunning();
    bool wakeUp();

protected:
    virtual ~Thread();

    static void *threadFunc(void *args);

    virtual bool onThreadStart(void);
    virtual void runOnce() = 0;
    virtual void onThreadStop(void); 

protected:
    pthread_t       m_ThreadId;
    pthread_cond_t  *m_Cond;
    bool            m_ThreadQuit;
};

#endif