#ifndef __DEF_H__
#define __DEF_H__

#include <stdint.h>
#include <pthread.h>

#define TRUE    1
#define FALSE   0

typedef unsigned char   BOOL;

typedef struct _bus_module_t bus_module_t;
typedef struct _bus_event_t  bus_event_t;
typedef struct _bus_t        bus_t;

typedef int32_t (*module_init_func_t)(bus_module_t *module, uint32_t id, const char *desc);
typedef void (*module_uninit_func_t)(bus_module_t *module);

typedef struct _bus_module_vtable_t  bus_module_vtable_t;
struct _bus_module_vtable_t {
    module_uninit_func_t uninit_func;    
};

struct _bus_module_t {
    struct list_head        list;
    int32_t                 id;
    char                    *desc;

    pthread_mutex_t         mutex;
    struct list_head        event_list_head;
    bus_t                   *bus;

    module_init_func_t      init_func;

    bus_module_vtable_t     *_vptr;
};

struct _bus_t {
    bus_module_t        **module_array;
    uint32_t            module_array_size;
    uint32_t            module_index;
    
    pthread_mutex_t     mutex;
    pthread_cond_t      cond;

    BOOL                running;    // 0 -- false, 1 -- true
    struct list_head    event_list_head;    // received event list

    pthread_t           thread_id;
};


typedef int32_t (*event_init_func_t)(bus_event_t *event, int32_t id, const char *desc, void *data);
typedef void    (*event_callback_t)(bus_event_t *event, void *param);
typedef void    (*event_uninit_func_t)(bus_event_t *event);

typedef struct _bus_event_vtable_t  bus_event_vtable_t;
struct _bus_event_vtable_t {
    event_callback_t    callback;
    event_uninit_func_t uninit_func;    
};

struct _bus_event_t {
    struct list_head    list;
    int32_t             id;
    void                *data;
    
    char                *desc;
	int32_t				ref_count;
	pthread_mutex_t		*mutex;
    
    event_init_func_t   init_func;
    bus_event_vtable_t  *_vptr;
};

#endif
