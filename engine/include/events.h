#ifndef _EVENTS_H_
#define _EVENTS_H_

typedef enum _event_id event_id;
enum _event_id {
    EVENT_MINIMUM_ID = 0,
    TIMER_SETUP_REQUEST,
    TIMER_SETUP_RESPONSE,

    EVENT_MAXIMUM_ID
};

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
    unsigned int    seq_no; // activated event No of TIMER_SETUP_REQUEST, Module maybe request multiple timers 
                            // at one time, it can be used to distingue which timer is activated                            

    int             error;  // < 0, failure reason; 0 - succeed;
};

#ifdef __cplusplus
extern "C" {
#endif

const char* str_event(event_id id);

#ifdef __cplusplus
}
#endif

#endif
