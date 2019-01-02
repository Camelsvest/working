#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "bus_module.h"
#include "bus_event.h"

#define LOCK_MODULE(module)     pthread_mutex_lock(&module->mutex)
#define UNLOCK_MODULE(module)   pthread_mutex_unlock(&module->mutex)

module_t* create_bus_module(int32_t id, const char *desc)
{
    module_t    *module;
    size_t      length;

    module = (module_t *)malloc(sizeof(module_t));
    if (module != NULL)
    {
        module->id = id;
        if (desc != NULL)
        {
            length = strlen(desc);
            if (length > 0)
            {
                module->desc = (char *)malloc(length + 1);
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
    }

    return module;
}

void destroy_bus_module(module_t *module)
{
    struct list_head *pos, *next;
    bus_event_t *event;
    
    if (module != NULL)
    {
        if (module->desc != NULL)
            free(module->desc);

        list_for_each_safe(pos, next, &module->event_list_head)
        {
            event = list_entry(pos, struct _bus_event_t, list);
            list_del(pos);
            destroy_bus_event(event);
        }

        pthread_mutex_destroy(&module->mutex);
        free(module);
    }
}

int32_t bus_module_dispatch_event(module_t *module, bus_event_t *event)
{
    int32_t             ret;
    bus_event_t         *target;
    struct list_head    *pos, *next;

    ret = -1;
    if (module != NULL)
    {
        LOCK_MODULE(module);
        list_for_each_safe(pos, next, &module->event_list_head)
        {
            target = list_entry(pos, struct _bus_event_t, list);
            if (event->id == target->id)
            {
                if (target->callback != NULL)
                {
                    target->callback(target, event->data);
                    ret = 0;
                }
            }
        }
        UNLOCK_MODULE(module);
    }

    return ret;
}

