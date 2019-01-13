#include "events.h"

const char* str_event(event_id id)
{
    char *event_str[] = {
        "EVENT_MINIMUM_ID",
        "TIMER_SETUP_REQUEST",
        "EVENT_MAXIMUM_ID"
    };

    return event_str[id];
}
