#include "eventUnit.h"
#include "stm32f10x.h"

volatile eventParams_ts eventParams[EVENT_INDEX_MAX];
volatile uint32_t timerEventMask;
uint32_t getEventMask;
bool eventDriverSta;

void eventDriver(void)
{
    int i;
    
    for ( i = 0; i < EVENT_INDEX_MAX; i++)
    {
        if (eventParams[i].countSet)
        {
        if (++ eventParams[i].count >= eventParams[i].countSet)
        {
            eventParams[i].count = 0;
            if (eventParams[i].reload == false)
            {
            eventParams[i].countSet = 0;
            }
            timerEventMask |= (1 << i);
        }
        }
    }
}
void setEvent(uint32_t mask, bool reload, uint32_t timeOut_ms)
{
    __set_PRIMASK(1);
    
    eventParams[mask].countSet = timeOut_ms;
    eventParams[mask].count = 0;
    eventParams[mask].reload = reload;
    if (timeOut_ms == 0)
    {
        timerEventMask |= (1 << mask);
    }

    __set_PRIMASK(0);//开总中断
}
void event_post(uint32_t mask)
{
    __set_PRIMASK(1);
    eventParams[mask].countSet = 0;
    eventParams[mask].count = 0;
    eventParams[mask].reload = false;

    timerEventMask |= (1 << mask);
    __set_PRIMASK(0);//开总中断
}
void event_clear(uint32_t mask)
{
    __set_PRIMASK(1);
    eventParams[mask].countSet = 0;
    eventParams[mask].count = 0;
    eventParams[mask].reload = false;
    __set_PRIMASK(0);//开总中断
}
uint32_t event_pend(void)
{
    uint32_t ret = 0;
    __set_PRIMASK(1);
    ret = timerEventMask;
    getEventMask = ret;
    timerEventMask = 0;
    __set_PRIMASK(0);//开总中断
    return ret;
}
uint32_t getEvent(uint32_t mask)
{
    return getEventMask & (1 << mask);
}



