#ifndef _EVENTS_H_
#define _EVENTS_H_

typedef enum _event_id {
    EVENT_MINIMUM_ID = 0,
    TIMER_SETUP_REQUEST,
    TIMER_SETUP_RESPONSE,
    TIMER_DELETE_REQUEST,
    TIMER_DELETE_RESPONSE,
    TIMER_RESPONSE,     // timer expired, timer activate

    EVENT_MAXIMUM_ID
} event_id;

/*
 * Used by TIMER_SETUP_REQUEST, put into event->data
 */
typedef struct _timer_param_t timer_param_t;
struct _timer_param_t {
    unsigned int    millseconds;
    char            repeat; // 0 - don't repeat; 1 - repeat
};

typedef struct _timer_resp_t timer_resp_t;
struct _timer_resp_t {
    int     timer_id;  // < 0, failure reason; > 0 - succeed, timer id;
    bool    activated;
};

const char* str_event(int32_t id);

#endif
