#ifndef _CLOCK_MODULE_H_
#define _CLOCK_MODULE_H_

#include <uv.h>
#include <map>

#include "bus.h"

class Clock;
typedef void (*timer_notify_func_t)(int32_t timer_id, void *args);
typedef struct _add_timer_params_t add_timer_params_t;
struct _add_timer_params_t
{
    int32_t     timer_id;
    uint32_t    millseconds;
    bool        repeat;
    timer_notify_func_t func;
    void*       func_param;
    Clock*      instance;
};

class Clock : public Thread
{
public:
    Clock();
    virtual ~Clock();

    int32_t addTimer(uint32_t millseconds, bool repeat, timer_notify_func_t func, void *func_args);
    int32_t delTimer(int32_t timerId);

    int32_t addTimer(add_timer_params_t *param);

protected:
    virtual bool onThreadStart();
    virtual void runOnce();
    virtual void onThreadStop();

    static void processAsyncEvent(uv_async_t *handle);
    static void onTimer(uv_timer_t *timer);

private:
    int32_t generateUniqueTimerId();

private:
    uv_async_t* m_Async;
    uv_loop_t*  m_Loop;

    static int32_t m_TimerIndex;
};

class ClockModule : public BusModule
{
public:
    ClockModule();
    virtual ~ClockModule();
    
protected:
    virtual int activateEvent(BusEvent *event);
    static void timerNotify(int32_t timer_id, void *args);
    void timerNotify(int32_t timer_id);

    void onTimer(int32_t timerId);

private:
    void postTimerSetupResponse(int32_t timer_id, BusModule *dest);
    void postTimerResponse(int32_t timer_id, BusModule *dest);

private:
    Clock *m_Clock;
    std::map<int32_t, BusModule*> m_timerId2BusModule;
};

#endif
