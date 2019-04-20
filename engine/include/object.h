#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <pthread.h>
#include <assert.h>

#define chk_mutex_lock(mutex) \
{ \
    int ret; \
    ret = pthread_mutex_lock(mutex); \
    assert(ret == 0); \
}

#define chk_mutex_unlock(mutex) \
{ \
    int ret; \
    ret = pthread_mutex_unlock(mutex); \
    assert(ret == 0); \
}

class Object {
public:
    Object();
    virtual ~Object();

    void lock() {
        chk_mutex_lock(m_Mutex);
    }

    void unlock() {
        chk_mutex_unlock(m_Mutex);
    }

protected:
    pthread_mutex_t *m_Mutex;
};

#endif