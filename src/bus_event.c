#include <stdlib.h>
#include <string.h>
#include "bus_event.h"


bus_event_t* create_bus_event(int32_t id, const char *desc, event_callback_t callback, void *data)
{
    bus_event_t *event;
    size_t length;

    event = (bus_event_t *)malloc(sizeof(bus_event_t));
    if (event != NULL)
    {
        event->id = id;
        event->callback = callback;
        event->data = data;
        
        if (desc != NULL)
        {
            length = strlen(desc);
            if (length > 0)
            {
                event->desc = (char *)malloc(length + 1);
                if (event->desc != NULL)
                {
                    strcpy(event->desc, desc);
                }
                else
                {
                    free(event);
                    event = NULL;
                }
            }
        }
        else
            event->desc = NULL;
    }

    return event;
}



void destroy_bus_event(bus_event_t * event)
{
    if (event != NULL)
    {
        if (event->desc != NULL)
            free(event->desc);

        free(event);
    }

    return;
}

