#include <assert.h>
#include <string.h>
#include "bus_event.h"
#include "utils/zalloc.h"

static void bus_event_uninit(bus_event_t *event)
{
	if (event != NULL)
	{
		LOCK_EVENT(event);
		
		if (event->ref_count > 0)
			event->ref_count--;
		else if (event->desc != NULL)
			zfree(event->desc);
		
		UNLOCK_EVENT(event);
	}
	

    return;
}

static void bus_event_callback(bus_event_t *event, void *data)
{
    assert(0);  // you must overload this function
}

static bus_event_vtable_t bus_event_vfuncs = 
{
    .uninit_func    = bus_event_uninit,
    .callback       = bus_event_callback
};

static int32_t bus_event_init(bus_event_t *event, int32_t id, const char *desc, void *data)
{
    size_t length;    
    int32_t ret = -1;
    
    if (event != NULL)
    {
		event->ref_count = 0;		
        event->id = id;
        event->data = data;
        event->_vptr = &bus_event_vfuncs;

		event->mutex = (pthread_mutex_t *)zmalloc(sizeof(pthread_mutex_t));
		if (event->mutex != NULL)
		{
			pthread_mutex_init(event->mutex, NULL);
	        
	        if (desc != NULL)
	        {
	            length = strlen(desc);
	            if (length > 0)
	            {
	                event->desc = (char *)zmalloc(length + 1);
	                if (event->desc != NULL)
	                {
	                    strcpy(event->desc, desc);
	                }
	                else
	                {
	                    zfree(event);
	                    event = NULL;
	                }
	            }
	        }
	        else
	            event->desc = NULL;

	        ret = 0;

		}

    }

    return ret;
}


bus_event_t* create_bus_event(int32_t id, const char *desc, void *data)
{
    bus_event_t *event;


    event = (bus_event_t *)zmalloc(sizeof(bus_event_t));
    if (event != NULL)
    {
        event->init_func = bus_event_init;

        if (event->init_func(event, id, desc, data) != 0)
        {
            zfree(event);
            event = NULL;
        }        
    }

    return event;
}

void destroy_bus_event(bus_event_t * event)
{
    if ((event != NULL) && (event->_vptr != NULL))
    {
        event->_vptr->uninit_func(event);
        zfree(event);
    }

    return;
}

int32_t	bus_event_addref(bus_event_t *event)
{
	int32_t count;
	
	if (event)
	{
		LOCK_EVENT(event);
		count = ++event->ref_count;
		UNLOCK_EVENT(event);
	}

	return count;
}


int32_t activate_bus_event(bus_event_t *event, void *data)
{
	int32_t ret = -1;
	
	if ((event != NULL) && (event->_vptr->callback != NULL))
	{
		event->_vptr->callback(event, data);
		ret = 0;
	}

	return ret;
}

