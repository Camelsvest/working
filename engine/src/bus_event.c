#include <assert.h>
#include <string.h>
#include "bus_event.h"
#include "utils/zalloc.h"

static void bus_event_uninit(bus_event_t *event)
{
	if (event != NULL)
	{	
		if (event->ref_count > 0)
			event->ref_count--;
		else if (event->desc != NULL)
			zfree(event->desc);
	}
	

    return;
}

static bus_event_vtable_t bus_event_vfuncs = 
{
    .uninit_func    = bus_event_uninit,
};

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static uint32_t global_sequence_no = 0;

static uint32_t generate_event_sequence_no()
{
    uint32_t seq_no;
    
    pthread_mutex_lock(&mutex);
    seq_no = global_sequence_no++;
    pthread_mutex_unlock(&mutex);

    return seq_no;
}

static int32_t bus_event_init(bus_module_t *sender, bus_event_t *event, int32_t id, const char *desc, void *data)
{
    size_t length;    
    int32_t ret = -1;
    
    if (event != NULL)
    {
        assert(sender != NULL);
        event->seq_no = generate_event_sequence_no();
        event->from = sender;
        event->dest = NULL;
        
		event->ref_count = 0;		
        event->id = id;
        event->data = data;
        event->_vptr = &bus_event_vfuncs;

	        
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

    return ret;
}


bus_event_t* create_bus_event(bus_module_t *sender, int32_t id, const char *desc, void *data)
{
    bus_event_t *event;


    event = (bus_event_t *)zmalloc(sizeof(bus_event_t));
    if (event != NULL)
    {
        event->init_func = bus_event_init;

        if (event->init_func(sender, event, id, desc, data) != 0)
        {
            zfree(event);
            event = NULL;
        }        
    }

    return event;
}

static void destroy_bus_event(bus_event_t * event)
{
    if ((event != NULL) && (event->_vptr != NULL))
    {
        event->_vptr->uninit_func(event);

        assert(event->ref_count == 0);
        zfree(event);
    }

    return;
}

int32_t	bus_event_addref(bus_event_t *event)
{
	int32_t count;
	
	if (event)
	{
//      LOCK_EVENT(event);
	    count = ++event->ref_count;
//      UNLOCK_EVENT(event);
	}

	return count;
}

int32_t bus_event_release(bus_event_t *event)
{
    int32_t count;

    if (event)
    {
//      LOCK_EVENT(event);
        count = --event->ref_count;
//      UNLOCK_EVENT(event);

        if (count == 0)
            destroy_bus_event(event);
    }

    return count;
}

