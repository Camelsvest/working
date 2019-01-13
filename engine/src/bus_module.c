#include <assert.h>
#include <string.h>
#include "bus_module.h"
#include "bus_event.h"
#include "utils/zalloc.h"

static void bus_module_uninit(bus_module_t *module)
{
    struct list_head *pos, *next;
    event_id_list_t *event;

    ENTER_FUNCTION;
    
    if (module != NULL)
    {
        if (module->desc != NULL)
            zfree(module->desc);

        if (module->_vptr != NULL)
            zfree(module->_vptr);
        
        list_for_each_safe(pos, next, &module->event_list_head.list)
        {
            event = list_entry(pos, event_id_list_t, list);
            list_del(pos);
            zfree(event);
        }

        pthread_mutex_destroy(&module->mutex);
    }

    EXIT_FUNCTION;
}

static int32_t bus_module_activate_event(bus_module_t *module, bus_event_t *event, void *param)
{
    logging_error("You must override function %s in derived classes.\r\n", __FUNCTION__);

    return -1;
}

static int32_t bus_module_init(bus_module_t *module, uint32_t id, const char *desc)
{
    size_t      length;  
    int32_t     ret = -1;

    ENTER_FUNCTION;
    
    if (module != NULL)
    {
        module->bus = NULL;       
        module->id = id;

        module->_vptr = (bus_module_vtable_t *)zmalloc(sizeof(bus_module_vtable_t));       
        module->_vptr->callback_func = bus_module_activate_event;
        module->_vptr->uninit_func = bus_module_uninit;

        
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

        INIT_LIST_HEAD(&module->event_list_head.list);
        pthread_mutex_init(&module->mutex, NULL);

        ret = 0;
    }

    EXIT_FUNCTION;
    
    return ret;    
}

int32_t init_bus_module(bus_module_t *module, uint32_t id, const char *desc)
{
    int32_t ret = -1;

    ENTER_FUNCTION;
    
    if (module != NULL)
    {
        module->init_func   = bus_module_init;    
        ret = module->init_func(module, id, desc);
    }

    EXIT_FUNCTION;
    
    return ret;
}

bus_module_t* create_bus_module(uint32_t id, const char *desc)
{
    bus_module_t    *module;

    ENTER_FUNCTION;
    
    module = (bus_module_t *)zmalloc(sizeof(bus_module_t));
    if (init_bus_module(module, id, desc) != 0)
    {
        zfree(module);
        module = NULL;
    }

    EXIT_FUNCTION;
    
    return module;
}

void destroy_bus_module(bus_module_t *module)
{
    ENTER_FUNCTION;
    
    if (module != NULL && module->_vptr != NULL)
    {
        module->_vptr->uninit_func(module);
        zfree(module);    
    }

    EXIT_FUNCTION;    
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

int32_t set_bus_module(bus_module_t *module, bus_t *bus)
{
    int32_t ret = -1;
    
    if (module != NULL)
    {
        module->bus = bus;
        ret = 0;
    }

    return ret;
}

int32_t bus_module_dispatch_event(bus_module_t *module, bus_event_t *event)
{
    int32_t             ret;
    event_id_list_t     *source;
    struct list_head    *pos, *next;

    ret = -1;
    if (module != NULL)
    {
        LOCK_MODULE(module);
        list_for_each_safe(pos, next, &module->event_list_head.list)
        {
            source = list_entry(pos,  event_id_list_t, list);
            if (event->id == source->id)
            {
				list_del(pos);	// removed dispatched event

                if (module->_vptr->callback_func != NULL)
                    ret = module->_vptr->callback_func(module, event);
                
				zfree(source);
            }
        }
        UNLOCK_MODULE(module);
    }

    return ret;
}

int32_t bus_module_subscribe_event(bus_module_t *module, int32_t event_id)
{
    event_id_list_t *event;
    int32_t ret = -1;
    
	if (module != NULL)
	{
        event = (event_id_list_t *)zmalloc(sizeof(event_id_list_t));
        if (event != NULL)
        {
            event->id = event_id;
            
            LOCK_MODULE(module);
            list_add(&event->list, &module->event_list_head.list);
            UNLOCK_MODULE(module);

            ret = 0;
        }
	}

    return ret;
}

int32_t bus_module_unsubscribe_event(bus_module_t *module, int32_t event_id)
{
    event_id_list_t *target;
    struct list_head *pos, *next;
    int32_t ret = -1;
    
    LOCK_MODULE(module);
    list_for_each_safe(pos, next, &module->event_list_head.list)
    {
        target = list_entry(pos,  event_id_list_t, list);
        if (event_id == target->id)
        {
            list_del(pos);  // removed dispatched event              
            zfree(target);

            ret = 0;
            break;
        }
    }
    
    UNLOCK_MODULE(module);

    return ret;
}


