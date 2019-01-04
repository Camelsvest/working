#include <assert.h>
#include <string.h>
#include "bus_module.h"
#include "bus_event.h"
#include "utils/zalloc.h"

#define LOCK_MODULE(module)     pthread_mutex_lock(&module->mutex)
#define UNLOCK_MODULE(module)   pthread_mutex_unlock(&module->mutex)

static int32_t bus_module_init(bus_module_t *module, int32_t id, const char *desc)
{
    size_t      length;    
    int32_t     ret = -1;
    
    if (module != NULL)
    {
        module->id = id;
        if (desc != NULL)
        {
            length = strlen(desc);
            if (length > 0)
            {
                module->desc = (char *)zmalloc(length + 1);
                if (module->desc != NULL)
                {
                    strcpy(module->desc, desc);
                }
            }
            else
            {
                module->desc = NULL;
            }            
        }

        INIT_LIST_HEAD(&module->event_list_head);
        pthread_mutex_init(&module->mutex, NULL);

        ret = 0;
    }

    return ret;    
}

static void bus_module_uninit(bus_module_t *module)
{
    struct list_head *pos, *next;
    bus_event_t *event;
    
    if (module != NULL)
    {
        if (module->desc != NULL)
            zfree(module->desc);

        list_for_each_safe(pos, next, &module->event_list_head)
        {
            event = list_entry(pos, struct _bus_event_t, list);
            list_del(pos);
            destroy_bus_event(event);
        }

        pthread_mutex_destroy(&module->mutex);
    }
}


static bus_module_vtable_t module_vtable =
{
    .uninit_func = bus_module_uninit
};

bus_module_t* create_bus_module(int32_t id, const char *desc)
{
    bus_module_t    *module;

    module = (bus_module_t *)zmalloc(sizeof(bus_module_t));
    if (module != NULL)
    {
        module->init_func   = bus_module_init;        
        module->_vptr       = &module_vtable;
        
        if (module->init_func(module, id, desc) != 0)
        {
            zfree(module);
            module = NULL;
        }
    }
    
    return module;
}

void destroy_bus_module(bus_module_t *module)
{
    if (module != NULL && module->_vptr != NULL)
    {
        module->_vptr->uninit_func(module);
        zfree(module);    
    }
}

void set_bus_module_id(bus_module_t *module, int32_t id)
{
    if (module != NULL)
        module->id = id;
}


int32_t set_bus_module_desc(bus_module_t *module, const char *desc)
{    
    size_t length;
    int32_t ret = -1;

    if (desc != NULL)
    {
        length = strlen(desc);
        if (length > 0)
        {
            if (module->desc != NULL)
                zfree(module->desc);

            module->desc = (char *)zmalloc(length + 1);
            strcpy(module->desc, desc);

            ret = 0;
        }       
    }

    return ret;    
}

int32_t bus_module_dispatch_event(bus_module_t *module, bus_event_t *event, void *param)
{
    int32_t             ret;
    bus_event_t         *source;
    struct list_head    *pos, *next;

    ret = -1;
    if (module != NULL)
    {
        LOCK_MODULE(module);
        list_for_each_safe(pos, next, &module->event_list_head)
        {
            source = list_entry(pos, struct _bus_event_t, list);
            if (event->id == source->id)
            {
				list_del(pos);	// removed dispatched event
                ret = activate_bus_event(event, param);
				destroy_bus_event(source);
            }
        }
        UNLOCK_MODULE(module);
    }

    return ret;
}

void bus_module_subscribe_event(bus_module_t *module, bus_event_t* event)
{
	if (module != NULL)
	{
            list_add(&event->list, &module->event_list_head);
	}
}


