#include <stdexcept>
using namespace std;

#include "zalloc.h"
#include "object.h"

Object::Object()
    : m_Mutex(NULL)
{
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
