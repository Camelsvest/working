#include <stdexcept>
#include <exception>

#include "zalloc.h"
#include "object.h"
#include "logging/logging.h"

Object::Object(const char *obj_name)
{
    if (obj_name == NULL)
        m_ObjectName = "UNKNOWN_OBJECT";
    else
        m_ObjectName = obj_name;
    
    m_Mutex = (pthread_mutex_t *)zmalloc(sizeof(pthread_mutex_t));
    if (m_Mutex != NULL)
    {
        pthread_mutex_init(m_Mutex, NULL);
    }
    else
    {
        throw std::runtime_error("memory is not enough!");
    }
}

Object::~Object()
{
    if (m_Mutex)
    {
        pthread_mutex_destroy(m_Mutex);
        zfree(m_Mutex);
    }
}

bool Object::setObjectName(const char *desc)
{
    bool succeed = true;
    
    try
    {
        m_ObjectName = desc;
    }
    catch (std::exception &e)
    {
        logging_error("exception caught: %s.\r\n", e.what());
        
        succeed = false;
    }

    return succeed;
}

void Object::lock()
{
    logging_debug("Locking %s.\r\n", m_ObjectName.c_str());
    chk_mutex_lock(m_Mutex);
}

void Object::unlock()
{
    logging_debug("Unlocking %s.\r\n", m_ObjectName.c_str());
    chk_mutex_unlock(m_Mutex);
}

#if defined(ZALLOC_DEBUG) || defined(DEBUG) || defined(_DEBUG)

void* Object::operator new(size_t nSize, const char *pszFilename, unsigned int nLine)
{
    return zmalloc(nSize);
}

void Object::operator delete(void *p, const char *pszFilename, unsigned int nLine)
{
    Object::operator delete(p);
}

void* Object::operator new[](size_t nSize, const char *pszFilename, unsigned int nLine)
{
    return zmalloc(nSize);
}

void Object::operator delete[](void *p, const char *pszFilename, unsigned int nLine)
{
    zfree(p);
}


void* Object::operator new(size_t nSize)
{
    return zmalloc(nSize);
}

void Object::operator delete(void *p)
{
    zfree(p);
}

void* Object::operator new[](size_t nSize)
{
    return zmalloc(nSize);
}

void Object::operator delete[](void *p)
{
    zfree(p);
}

void* operator new(size_t size, const char *filename, unsigned int line)
{
    return ::_zmalloc(size, filename, line);
}

void operator delete(void* p, const char *filename, unsigned int line)
{
    _zfree(p);
}

void* operator new[](size_t size, const char *filename, unsigned int line)
{
    return operator new(size, filename, line);
}

void operator delete[](void* p, const char *filename, unsigned int line)
{
    return operator delete(p, filename, line);
}

#endif


