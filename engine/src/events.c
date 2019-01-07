#include "events.h"

const char* str_event(event_id id)
{
    char *event_str[] = {
        "EVENT_MINIMUM_ID",
        "SETUP_TIMER_EVENT",
        "EVENT_MAXIMUM_ID"
    };

    return event_str[id];
}
