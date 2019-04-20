#include <stdint.h>
#include "events.h"

const char* str_event(int32_t id)
{
    const char *event_str[] = {
        "EVENT_MINIMUM_ID",
        "TIMER_SETUP_REQUEST",
        "TIMER_SETUP_RESPONSE",
        "TIMER_DELETE_REQUEST",
        "TIMER_DELETE_RESPONSE",
        "TIMER_RESPONSE",     // timer expired, timer activate
        "EVENT_MAXIMUM_ID"
    };

    if (id >= EVENT_MINIMUM_ID && id <= EVENT_MAXIMUM_ID)
        return event_str[id];
    else
    {
        return "Unknow event";
    }
}
