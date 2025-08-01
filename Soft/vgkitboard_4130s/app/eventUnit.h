#ifndef __EVENTUNIT_H
#define __EVENTUNIT_H
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define EVENT_INDEX_MAX 32
typedef struct
{
    bool reload;            //
    uint32_t count;          //
    uint32_t countSet;            //
}eventParams_ts;

void eventDriver(void);
void setEvent(uint32_t mask, bool reload, uint32_t timeOut_ms);
void event_post(uint32_t mask);
void event_clear(uint32_t mask);
uint32_t event_pend(void);
uint32_t getEvent(uint32_t mask);
#endif

