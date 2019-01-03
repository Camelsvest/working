#ifndef _ZALLOC_H_
#define _ZALLOC_H_

#include <stdlib.h>

#if defined(ZALLOC_DEBUG) || defined(DEBUG) || defined(_DEBUG)
        #define zalloc_init()                 _zalloc_init()
        #define zalloc_uninit()               _zalloc_uninit()
        #define zalloc_show_statistics()      _zalloc_show_statistics()

        #define zmalloc(size)       _zmalloc(size, __FILE__, __LINE__)
        #define zcalloc(a, b)       _zcalloc(a, b, __FILE__, __LINE__)
        #define zrealloc(a, b)      _zrealloc(a, b, __FILE__, __LINE__)
        #define zfree(p)            _zfree(p)
		#define zstrdup(p)			_zstrdup(p, __FILE__, __LINE__)

#else
        #define zalloc_init()
        #define zalloc_uninit()
        #define zalloc_show_statistics()
        #define zmalloc(size)		malloc(size)
        #define zcalloc(a, b)       calloc(a, b)
        #define zrealloc(a, b)      realloc(a, b)
        #define zfree(p)            free(p)
        #define zstrdup(p)			strdup(p)

        #define _zmalloc(a, b, c)     malloc(a)
        #define _zcalloc(a, b, c, d)  calloc(a, b)
        #define _zfree(p)             free(p)
#endif

#ifdef __cplusplus
extern "C" {
#endif

void    _zalloc_init();
void    _zalloc_uninit();

void*   _zmalloc(size_t size, char *filename, unsigned int line);
void*   _zcalloc(size_t num, size_t size, char *filename, unsigned int line);
void*   _zrealloc(void *ptr, size_t size, char *filename, unsigned int line);
void    _zfree(void *memblock);
char*   _zstrdup(const char *s, char *filename, unsigned int line);

#ifdef __cplusplus
}
#endif

        
#endif
