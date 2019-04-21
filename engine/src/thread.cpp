#include <stdexcept>
#include <sys/prctl.h>
#include "zalloc.h"
#include "thread.h"
#include "def.h"

#if defined(ZALLOC_DEBUG) || defined(DEBUG) || defined(_DEBUG)
#define new zdebug_new
#endif

Thread::Thread(const char *thread_name)
    : Object(thread_name)
    , m_ThreadId(0)
    , m_ThreadQuit(true)
{   
    m_Cond = (pthread_cond_t *)zmalloc(sizeof(pthread_cond_t));
    if (m_Cond != NULL)
    {
        pthread_cond_init(m_Cond, NULL);
    }
    else
    {
        throw std::runtime_error("Memory is not enough");
    }    
}

Thread::~Thread()
{
    if (m_Cond)
    {
        pthread_cond_destroy(m_Cond);
        zfree(m_Cond);
    }
}

bool Thread::start()
{
    int ret = -1;

    ENTER_FUNCTION;

    if (m_ThreadId != 0)
        return false;

    ret = pthread_create(&m_ThreadId, NULL, threadFunc, this);
    if (ret == 0)
        pthread_cond_signal(m_Cond);    // trigger once

    EXIT_FUNCTION;
    
    return (ret == 0);
}

bool Thread::stop()
{
    void *result;    
    int ret = -1;

    ENTER_FUNCTION;
        
    lock();
    m_ThreadQuit = true;
    unlock();
    ret = pthread_cond_signal(m_Cond);
    if (ret == 0);
    {
        ret = pthread_join(m_ThreadId, &result);
        if (ret == 0) // succeed
            m_ThreadId = 0;
    }

    EXIT_FUNCTION;
    
    return (ret == 0);
}

bool Thread::isRunning()
{
    bool running;

    ENTER_FUNCTION;
    
    lock();
    running = !m_ThreadQuit;
    unlock();

    EXIT_FUNCTION;
    
    return running;
}

void* Thread::threadFunc(void *args)
{
    Thread *pThis;

    pThis = static_cast<Thread *>(args);
    
    if (!pThis->onThreadStart())
        return 0L;

    pThis->lock();
    while (!pThis->m_ThreadQuit)
    {
        pthread_cond_wait(pThis->m_Cond, pThis->m_Mutex);
        pThis->runOnce();        
    }
    pThis->unlock();

    pThis->onThreadStop();

    return 0L;
}

bool Thread::onThreadStart()
{
    prctl(PR_SET_NAME, getObjectName());    
    m_ThreadQuit = false;

    logging_debug("0x%X Thread %s started.\r\n", m_ThreadId, getObjectName());
    return true;
}

void Thread::onThreadStop()
{
    logging_debug("0x%X Thread %s stopped.\r\n", m_ThreadId, getObjectName());    
}

bool Thread::wakeUp()
{
    int ret = -1;

    if (m_Cond)
        ret  = pthread_cond_signal(m_Cond);
    
    return (ret == 0);
}