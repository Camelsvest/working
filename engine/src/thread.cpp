#include <stdexcept>
#include <sys/prctl.h>
#include "zalloc.h"
#include "thread.h"
#include "logging/logging.h"

Thread::Thread(const char *thread_name)
    : m_ThreadId(0)
    , m_ThreadQuit(true)
{
    if (thread_name)
        m_ThreadName = thread_name;
    else
        m_ThreadName = "UNKNOWN_THREAD";
    
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

    if (m_ThreadId != 0)
        return false;

    ret = pthread_create(&m_ThreadId, NULL, threadFunc, this);
    if (ret == 0)
        pthread_cond_signal(m_Cond);    // trigger once
    
    return (ret == 0);
}

bool Thread::stop()
{
    void *result;    
    int ret = -1;

    lock();
    m_ThreadQuit = true;
    pthread_cond_signal(m_Cond);
    unlock();

    ret = pthread_join(m_ThreadId, &result);
    if (ret == 0) // succeed
        m_ThreadId = 0;
    
    return (ret == 0);
}

bool Thread::isRunning()
{
    bool running;
    lock();
    running = !m_ThreadQuit;
    unlock();

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
    prctl(PR_SET_NAME, m_ThreadName);    
    m_ThreadQuit = false;

    logging_verbos("0x%X Thread %s started.\r\n", m_ThreadId, m_ThreadName.c_str());
    return true;
}

void Thread::onThreadStop()
{
    logging_verbos("0x%X Thread %s stopped.\r\n", m_ThreadId, m_ThreadName.c_str());    
}

bool Thread::wakeUp()
{
    int ret = -1;

    if (m_Cond)
        ret  = pthread_cond_signal(m_Cond);
    
    return (ret == 0);
}