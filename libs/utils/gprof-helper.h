#ifndef _GPROF_HELPER_H_
#define _GPROF_HELPER_H_

#include <pthread.h>

int gprof_pthread_create(pthread_t *__restrict thread,
                   __const pthread_attr_t *__restrict attr,
                   void * (*start_routine)(void *),
                   void *__restrict arg);

#endif
