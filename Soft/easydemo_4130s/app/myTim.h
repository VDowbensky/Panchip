#ifndef __MYTIM_H
#define __MYTIM_H
#include "stm32f10x.h"

typedef void (*TIM_CALLBACK)(void);

/**
 * tim3定时器初始化
 * @period_us：定时器定时周期，以微秒为单位
 * @cb：定时器回调函数，定时器计时溢出时调用该函数，
        相当于向外抛出中断，方便主函数调用
*/
void myTim3_init(uint32_t period_us, TIM_CALLBACK cb);
/**
 * tim1定时器初始化
 * @period_us：定时器定时周期，以微秒为单位
 * @cb：定时器回调函数，定时器计时溢出时调用该函数，
        相当于向外抛出中断，方便主函数调用
*/
void myTim1_init(uint32_t period_us, TIM_CALLBACK cb);
#endif

