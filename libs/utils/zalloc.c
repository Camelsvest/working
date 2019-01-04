#if defined(WIN32) || defined(_WIN32)
        #include <Windows.h>
        #include <process.h>
#else
        #ifndef _GNU_SOURCE
                #define _GNU_SOURCE
        #endif
        #include <pthread.h>
#endif

#include <assert.h>
#include <string.h>
#include "zalloc.h"
#include "hash.h"
#include "logging/logging.h"

#define ALIGN(size, alignment) (((size) + (alignment) - 1) & ~((alignment) - 1))
#define MAGIC   0x000399B6

#if !defined(WIN32) && !defined(_WIN32)
    #define strcpy_s(a, b, c)	strcpy(a, c)
	#define strncpy_s(a, b, c, d)	strncpy(a, c, d)
    #define strcat_s(a, b, c)	strcat(a, c)

	#define FALSE			0
	#define TRUE			1
#endif

typedef struct _alloc_head
{
        unsigned int    magic;
        char            filename[256];
        unsigned int    line;
        size_t          length;
        unsigned int    reserved;
} alloc_head_t;

typedef struct _alloc_tail
{
        unsigned int    magic;
} alloc_tail_t;

typedef struct _alloc_statistic
{
#if defined(WIN32) || defined(_WIN32)
        HANDLE          mutex;
#else
        pthread_mutex_t *mutex;
#endif

        hash_t       	*hash_table;
        unsigned int    total_allocated_bytes;
        unsigned int    total_free_bytes;
        unsigned int    maximum_memory_bytes;
        unsigned int    total_allocated_times;
        unsigned int    total_free_times;
} alloc_stat_t;

static alloc_stat_t     *alloc_stat = NULL;

int _zalloc_init()
{
    int ret = -1;
    
    if (alloc_stat == NULL)
    {
        alloc_stat = (alloc_stat_t *)malloc(sizeof(alloc_stat_t));
        if (alloc_stat)
        {
        #if defined(WIN32) || defined(_WIN32)                    
            alloc_stat->mutex = CreateMutex(NULL, FALSE, NULL);
        #else
            alloc_stat->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
            pthread_mutex_init(alloc_stat->mutex, NULL);
        #endif

            alloc_stat->hash_table = hash_init();
            alloc_stat->total_allocated_bytes       = 0;
            alloc_stat->total_free_bytes            = 0;
            alloc_stat->maximum_memory_bytes        = 0;
            alloc_stat->total_allocated_times       = 0;
            alloc_stat->total_free_times            = 0;

            ret = 0;
        }
    }

    return ret;
}

void alloc_visit_func(unsigned int key, void *data)
{
        alloc_head_t *head;

        head = (alloc_head_t *)data;
        logging_error("memblock 0x%p, %u bytes, %s:%u\r\n", 
                head+1, head->length, head->filename, head->line);

        free(head);
}

void _zalloc_show_statistics()
{
	logging_trace("======= ZALLOC operation statistics ========\r\n");
	logging_trace(".....Total allocated bytes:         %u bytes\r\n", alloc_stat->total_allocated_bytes);
	logging_trace(".....Total free bytes:              %u bytes\r\n", alloc_stat->total_free_bytes);
	logging_trace(".....Maximum occupied memory:       %u bytes\r\n", alloc_stat->maximum_memory_bytes);
	logging_trace(".....Total allocated times:         %u\r\n", alloc_stat->total_allocated_times);
	logging_trace(".....Total free times:              %u\r\n", alloc_stat->total_free_times);
	logging_trace("============================================\r\n");	
}

void _zalloc_uninit()
{
    if (alloc_stat == NULL)
        return;

    if (hash_amount(alloc_stat->hash_table) > 0)
    {
        logging_error("There are still following memblocks weren't free!\r\n");

        hash_foreach(alloc_stat->hash_table, alloc_visit_func, NULL);
        hash_uninit(alloc_stat->hash_table);
    }

	_zalloc_show_statistics();

#if defined(WIN32) || defined(_WIN32)
    CloseHandle(alloc_stat->mutex);
#else
    pthread_mutex_destroy(alloc_stat->mutex);
    free(alloc_stat->mutex);
#endif
    free(alloc_stat);
}

void* _zmalloc(size_t size, char *filename, unsigned int line)
{
    alloc_head_t    *head;
    alloc_tail_t    *tail;
    size_t          real_size, str_len;
    unsigned int    bytes;

#if defined(WIN32) || defined(_WIN32)
    DWORD           result;
#else
	int		result;
#endif

    size = ALIGN(size, sizeof(void *));
    real_size = size;
    real_size += sizeof(alloc_head_t);
    real_size += sizeof(alloc_tail_t);

    head = (alloc_head_t *)malloc(real_size);
    if (head)
    {
            head->magic     = MAGIC;
            head->length    = size;
            head->line      = line;

            if (filename != NULL)
                str_len = strlen(filename);
            else
                str_len = 0;

            if (str_len > 0 && str_len < sizeof(head->filename))
            {
                    strcpy_s(head->filename, sizeof(head->filename), filename);
            }
            else if (str_len > 0)
            {
                    str_len = sizeof(head->filename) - 1;
                    strncpy_s(head->filename, sizeof(head->filename), filename, str_len);
                    head->filename[str_len - 1] = '\0';
            }
            else
                    head->filename[0] = '\0';

            tail = (alloc_tail_t *)((char *)(head+1) + size);
            tail->magic = MAGIC;

            if (alloc_stat != NULL)
            {
            #if defined(WIN32) || defined(_WIN32)
                    result = WaitForSingleObject(alloc_stat->mutex, INFINITE);
                    assert(result == WAIT_OBJECT_0);
            #else
                    result = pthread_mutex_lock(alloc_stat->mutex);
                    assert(result == 0);
            #endif                        
                    
                    hash_insert(alloc_stat->hash_table, head);
                    
                    alloc_stat->total_allocated_times++;
                    alloc_stat->total_allocated_bytes += head->length;
                    
                    bytes = alloc_stat->total_allocated_bytes - alloc_stat->total_free_bytes;
                    if (bytes > alloc_stat->maximum_memory_bytes)
                            alloc_stat->maximum_memory_bytes = bytes;

            #if defined(WIN32) || defined(_WIN32)
                    ReleaseMutex(alloc_stat->mutex);
            #else
                    result = pthread_mutex_unlock(alloc_stat->mutex);
					assert(result == 0);
            #endif
            }

            return head + 1;
    }

    return NULL;
}


void* _zcalloc(size_t num, size_t size, char *filename, unsigned int line)
{
    alloc_head_t    *head;
    alloc_tail_t    *tail;
    size_t          real_size, str_len;
    unsigned int    bytes;

#if defined(WIN32) || defined(_WIN32)
    DWORD           result;
#else
int		result;
#endif

    size = ALIGN((num *size), sizeof(void *));
    real_size = size;
    real_size += sizeof(alloc_head_t);
    real_size += sizeof(alloc_tail_t);

    head = (alloc_head_t *)calloc(real_size, 1);
    if (head)
    {
            head->magic     = MAGIC;
            head->length    = size;
            head->line      = line;

            if (filename != NULL)
                    str_len = strlen(filename);
            else
                    str_len = 0;

            if (str_len > 0 && str_len < sizeof(head->filename))
            {
                    strncpy_s(head->filename, sizeof(head->filename), filename, str_len);
            }
            else if (str_len > 0)
            {
                    str_len = sizeof(head->filename) - 1;
                    strncpy_s(head->filename, sizeof(head->filename), filename, str_len);
                    head->filename[str_len - 1] = '\0';
            }
            else
                    head->filename[0] = '\0';

            tail = (alloc_tail_t *)((char *)(head+1) + size);
            tail->magic = MAGIC;

            if (alloc_stat != NULL)
            {
                #if defined(WIN32) || defined(_WIN32)                    
                    result = WaitForSingleObject(alloc_stat->mutex, INFINITE);
                    assert(result == WAIT_OBJECT_0);
                #else
                    result = pthread_mutex_lock(alloc_stat->mutex);
                    assert(result == 0);
                #endif
                
                    hash_insert(alloc_stat->hash_table, head);

                    alloc_stat->total_allocated_times++;
                    alloc_stat->total_allocated_bytes += head->length;
                    
                    bytes = alloc_stat->total_allocated_bytes - alloc_stat->total_free_bytes;
                    if (bytes > alloc_stat->maximum_memory_bytes)
                            alloc_stat->maximum_memory_bytes = bytes;

                #if defined(WIN32) || defined(_WIN32)     
                    ReleaseMutex(alloc_stat->mutex);
                #else
                    result = pthread_mutex_unlock(alloc_stat->mutex);
                    assert(result == 0);
                #endif
            }

            return head + 1;
    }

    return NULL;
}


void* _zrealloc(void *ptr, size_t size, char *filename, unsigned int line)
{
    alloc_head_t *head;
    alloc_tail_t *tail;
    size_t str_len, real_size;
	unsigned int bytes;

#if defined(WIN32) || defined(_WIN32)
    DWORD result;
#else
    int result;
#endif

	if (ptr == NULL)
	{
		return _zmalloc(size, filename, line);
	}

    head = (alloc_head_t *)ptr;
    head -= 1;

    if (head->magic != MAGIC)
            assert(FALSE);

    tail = (alloc_tail_t *)((char *)ptr + head->length);
    if (tail->magic != MAGIC)
            assert(FALSE);

    if (alloc_stat != NULL)
    {
        #if defined(WIN32) || defined(_WIN32)                
            result = WaitForSingleObject(alloc_stat->mutex, INFINITE);
            assert(result == WAIT_OBJECT_0);
        #else
            result = pthread_mutex_lock(alloc_stat->mutex);
            assert(result == 0);        
        #endif               
        
            hash_remove(alloc_stat->hash_table, head);
            alloc_stat->total_free_bytes += head->length;
            alloc_stat->total_free_times++;

        #if defined(WIN32) || defined(_WIN32)  
            ReleaseMutex(alloc_stat->mutex);
        #else
            result = pthread_mutex_unlock(alloc_stat->mutex);
            assert(result == 0);            
        #endif            
    }

    size = ALIGN(size, sizeof(void *));
    real_size = size;
    real_size += sizeof(alloc_head_t);
    real_size += sizeof(alloc_tail_t);

    head = (alloc_head_t *)realloc(head, real_size);
    if (head)
    {
            assert(head->magic == MAGIC);

            head->length    = size;
            head->line      = line;

            if (filename != NULL)
                    str_len = strlen(filename);
            else
                    str_len = 0;

            if (str_len > 0 && str_len < sizeof(head->filename))
            {
                    strncpy_s(head->filename, sizeof(head->filename), filename, str_len);
            }
            else if (str_len > 0)
            {
                    str_len = sizeof(head->filename) - 1;
                    strncpy_s(head->filename, sizeof(head->filename), filename, str_len);
                    head->filename[str_len - 1] = '\0';
            }
            else
                    head->filename[0] = '\0';

            tail = (alloc_tail_t *)((char *)(head+1) + size);
            tail->magic = MAGIC;

            if (alloc_stat != NULL)
            {
                #if defined(WIN32) || defined(_WIN32)                    
                    result = WaitForSingleObject(alloc_stat->mutex, INFINITE);
                    assert(result == WAIT_OBJECT_0);
                #else
                    result = pthread_mutex_lock(alloc_stat->mutex);
                    assert(result == 0);
                #endif
		
                    hash_insert(alloc_stat->hash_table, head);

                    alloc_stat->total_allocated_times++;
                    alloc_stat->total_allocated_bytes += head->length;
                    
                    bytes = alloc_stat->total_allocated_bytes - alloc_stat->total_free_bytes;
                    if (bytes > alloc_stat->maximum_memory_bytes)
                            alloc_stat->maximum_memory_bytes = bytes;

                #if defined(WIN32) || defined(_WIN32)     
                    ReleaseMutex(alloc_stat->mutex);
                #else
                    result = pthread_mutex_unlock(alloc_stat->mutex);
                    assert(result == 0);
                #endif
            }

            return head + 1;
    }

    return NULL;        
}


void _zfree(void *memblock)
{
    alloc_head_t *head;
    alloc_tail_t *tail;

#if defined(WIN32) || defined(_WIN32)
    DWORD result;
#else
	int result;
#endif

	if (memblock == NULL)
		return;

    head = (alloc_head_t *)memblock;
    head -= 1;

    if (head->magic != MAGIC)
            assert(FALSE);

    tail = (alloc_tail_t *)((char *)memblock + head->length);
    if (tail->magic != MAGIC)
            assert(FALSE);

    if (alloc_stat != NULL)
    {
        #if defined(WIN32) || defined(_WIN32)                
            result = WaitForSingleObject(alloc_stat->mutex, INFINITE);
            assert(result == WAIT_OBJECT_0);
        #else
            result = pthread_mutex_lock(alloc_stat->mutex);
            assert(result == 0);
        #endif               
        
            hash_remove(alloc_stat->hash_table, head);

            alloc_stat->total_free_bytes += head->length;
            alloc_stat->total_free_times++;

        #if defined(WIN32) || defined(_WIN32)  
            ReleaseMutex(alloc_stat->mutex);
        #else
            result = pthread_mutex_unlock(alloc_stat->mutex);
            assert(result == 0);
        #endif            
    }

    free(head);

}

char * _zstrdup(const char *s, char *filename, unsigned int line)
{
	size_t len;
	char *buf = NULL;

	len = strlen(s) + 1;
	if (len > 1)
	{
		buf = (char *)_zmalloc(len, filename, line);
		if (buf)
		{
                #if defined(WIN32) || defined(_WIN32)
                    strcpy_s(buf, len, s);
                #else
					strcpy(buf, s);
                #endif
		}
	}

	return buf;
}
