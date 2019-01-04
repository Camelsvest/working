#include <assert.h>
#include <pthread.h>
#include <string.h>
#include "bus.h"
#include "bus_event.h"
#include "bus_module.h"
#include "linux-like-list/list.h"
#include "utils/zalloc.h"

#define LOCK_BUS(bus)       pthread_mutex_lock(&bus->mutex)
#define UNLOCK_BUS(bus)     pthread_mutex_unlock(&bus->mutex)

#define MODULE_ARRAY_MINIMUM_SIZE   8

struct _bus_t {
    bus_module_t        **module_array;
    uint32_t            module_array_size;
    uint32_t            module_index;
    
    pthread_mutex_t     mutex;
    pthread_cond_t      cond;

    char                running;    // 0 -- false, 1 -- true
    struct list_head    event_list_head;    // received event list

    pthread_t           thread_id;
};

bus_t* create_bus(uint32_t module_count)
{
    bus_t *bus;

    bus = (bus_t *)zmalloc(sizeof(bus_t));
    if (bus != NULL)
    {
        pthread_mutex_init(&bus->mutex, NULL);
        pthread_cond_init(&bus->cond, NULL);

        bus->thread_id = 0;

        if (module_count >= MODULE_ARRAY_MINIMUM_SIZE)
            bus->module_array_size = module_count;
        else
            bus->module_array_size = MODULE_ARRAY_MINIMUM_SIZE;
        
        bus->module_array = (bus_module_t **)zcalloc(bus->module_array_size, sizeof(bus_module_t *));
        bus->module_index = 0;
        
        INIT_LIST_HEAD(&bus->event_list_head);
    }
   
    return bus;
}

void destroy_bus(bus_t *bus)
{
    if (bus != NULL)
    {
        pthread_cond_destroy(&bus->cond);
        pthread_mutex_destroy(&bus->mutex);

        zfree(bus->module_array);
        zfree(bus);
    }

    return;
}

int32_t bus_attach_module(bus_t *bus, bus_module_t *module)
{

    LOCK_BUS(bus);

    if (bus->module_array_size > (bus->module_index + 1))
    {
        if (module != NULL)
        {
            set_bus_module_id(module, bus->module_index++);
            bus->module_array[module->id] = module;
        }
        else
        {
            bus->module_index--;
        }
    }
    UNLOCK_BUS(bus);

    return (module != NULL) ? BUS_MODULE_ID(module) : -1;
}

static int32_t bus_detach_module2(bus_t *bus, int32_t id)
{
    LOCK_BUS(bus);
    if (id <= bus->module_index) 
    {
        bus->module_array[id] = NULL;
    }
    else
    {
        // ... To do, illegal paramter
    }    
    UNLOCK_BUS(bus);

    return 0;
}

int32_t bus_detach_module(bus_t *bus, bus_module_t *module)
{
    return bus_detach_module2(bus, BUS_MODULE_ID(module));
}

static int32_t bus_subscribe_event2(bus_t *bus, int32_t module_id, bus_event_t *event)
{
    int32_t ret;
    bus_module_t *module;

    ret = -1;
    LOCK_BUS(bus);
    if ((bus != NULL) && (module_id <= bus->module_index))
    {
        module = bus->module_array[module_id];
        if (module != NULL)
        {
			bus_module_subscribe_event(module, event);
            ret = 0;
        }
    }
    UNLOCK_BUS(bus);

    return ret;
}

int32_t bus_subscribe_event(bus_t *bus, bus_module_t *module, bus_event_t *event)
{
    return bus_subscribe_event2(bus, BUS_MODULE_ID(module), event);
}

static int32_t bus_unsubscribe_event2(bus_t *bus, int32_t module_id, bus_event_t *event)
{
    int32_t ret;
    bus_module_t *module;
    bus_event_t *node;
    struct list_head *pos, *next;

    ret = -1;
    LOCK_BUS(bus);
    if ((bus != NULL) && (module_id <= bus->module_index))
    {
        module = bus->module_array[module_id];
        if (module != NULL)
        {
            list_for_each_safe(pos, next, &module->event_list_head)
            {
                node = list_entry(pos, struct _bus_event_t, list);
                if (node == event)
                {
                    list_del(&event->list);
                    ret = 0;

                    break;
                }
            }
        }
    }
    UNLOCK_BUS(bus);
    
    return ret;
}

int32_t bus_unsubscribe_event(bus_t *bus, bus_module_t *module, bus_event_t *event)
{
    return bus_unsubscribe_event2(bus, BUS_MODULE_ID(module), event);
}

static void* bus_thread_func(void *param)
{
    bus_t               *bus;
    bus_event_t         *node;
    bus_module_t            *module;
    struct list_head    *pos, *next;
    uint32_t            index;

    bus = (bus_t *)param;
    if (bus != NULL)
    {
        LOCK_BUS(bus);
        while(bus->running)
        {
            pthread_cond_wait(&bus->cond, &bus->mutex);
            
            list_for_each_safe(pos, next, &bus->event_list_head)
            {
                node = list_entry(pos, struct _bus_event_t, list);                
                list_del(pos);

                for (index = 0; index < bus->module_index; index++)
                {
                    module = bus->module_array[index];
                    if (module != NULL)
                    {
                        bus_module_dispatch_event(module, node, 0);
                    }
                }
            }
        } 
        UNLOCK_BUS(bus);
    }

    return 0;
}


int32_t	bus_dispatch_module_event(bus_t *bus, bus_module_t *destination, bus_event_t *event, void *param)
{
	bus_module_t *module;
	int32_t index, ret = -1;

	
	LOCK_BUS(bus);

    for (index = 0; index < bus->module_index; index++)
    {
        module = bus->module_array[index];
		if (destination == NULL)	// deliver event to each module
		{
			ret = bus_module_dispatch_event(module, event, 0);
		}
		else if (destination == module)
		{
			ret = bus_module_dispatch_event(module, event, param);
			break;	// deliver ONLY ONCE
		}
    }
	
	UNLOCK_BUS(bus);

	return ret;

}


int32_t	bus_dispatch_event(bus_t *bus, bus_event_t *event, void *param)
{
	return bus_dispatch_module_event(bus, NULL, event, param);
}

int32_t bus_start(bus_t *bus)
{
    int32_t ret;

    ret = -1;
    if (bus != NULL) {
        bus->running = 1;
        ret = pthread_create(&bus->thread_id, NULL, bus_thread_func, bus);
    }

    return ret;
}

int32_t bus_stop(bus_t *bus)
{
    int32_t ret = -1;
    
    if (bus != NULL)
    {
        LOCK_BUS(bus);
        bus->running = 0;
        UNLOCK_BUS(bus);

        pthread_cond_signal(&bus->cond);
        ret = 0;
    }

    return ret;
}



