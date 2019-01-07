#ifndef _EVENTS_H_
#define _EVENTS_H_

typedef enum _event_id event_id;
enum _event_id {
    EVENT_MINIMUM_ID = 0,
    SETUP_TIMER_EVENT,

    EVENT_MAXIMUM_ID
};

struct _setup_timer_t {
    
};

#ifdef __cplusplus
extern "C" {
#endif

const char* str_event(event_id id);

#ifdef __cplusplus
}
#endif

#endif
