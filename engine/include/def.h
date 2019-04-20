#ifndef __DEF_H__
#define __DEF_H__

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "logging/logging.h"

#ifdef __WIN32_WINNT
    #define FILENAME(x) (strrchr(x,'\\')?strrchr(x,'\\')+1:x)
#else
    #define FILENAME(x) (strrchr(x,'/')?strrchr(x,'/')+1:x)
#endif

#define __FILENAME__     (FILENAME(__FILE__))

#define ENTER_FUNCTION  logging_verbos("%s:%u, Enter %s\r\n", __FILENAME__, __LINE__, __FUNCTION__)
#define EXIT_FUNCTION   logging_verbos("%s:%u, Exit %s\r\n", __FILENAME__, __LINE__, __FUNCTION__);

#endif
