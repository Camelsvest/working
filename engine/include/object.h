#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <pthread.h>
#include <assert.h>
#include <string>

#include "zalloc.h"

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
    Object(const char *obj_name = NULL);
    virtual ~Object();

    void lock();

    void unlock();

    const char *    getObjectName() { return m_ObjectName.c_str(); }
    bool            setObjectName(const char *desc);

#if defined(ZALLOC_DEBUG) || defined(DEBUG) || defined(_DEBUG)

    static void*   operator new(size_t nSize, const char *pszFilename, unsigned int nLine);
    static void    operator delete(void *p, const char *pszFilename, unsigned int nLine);

    static void*   operator new[](size_t nSize, const char *pszFilename, unsigned int nLine);
    static void    operator delete[](void *p, const char *pszFilename, unsigned int nLine);
    
    static void*   operator new(size_t nSize);
    static void    operator delete(void *p);

    static void*   operator new[](size_t nSize);
    static void    operator delete[](void *p);
#endif

protected:
    std::string     m_ObjectName;    
    pthread_mutex_t *m_Mutex;
};

#if defined(ZALLOC_DEBUG) || defined(DEBUG) || defined(_DEBUG)
#define zdebug_new new(__FILE__, __LINE__)

void*   operator new(size_t size, const char *filename, unsigned int line);
void    operator delete(void* p, const char *filename, unsigned int line);

void*   operator new[](size_t size, const char *filename, unsigned int line);
void    operator delete[](void* p, const char *filename, unsigned int line);
#endif


#endif