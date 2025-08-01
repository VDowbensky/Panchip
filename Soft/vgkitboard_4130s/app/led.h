#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
#include "stm32f10x.h"

typedef union 
{
    uint8_t value;
    struct 
    {
        uint8_t led1 : 1; 
        uint8_t led2 : 1; 
        uint8_t led3 : 1; 
        uint8_t led4 : 1; 
        uint8_t led5 : 1; 
    }unit;
}ledSta_tn;
typedef struct 
{
    uint16_t count;    //led闪烁亮灯时间计数，时间单位：1ms
    uint16_t countSet;    //亮灯时间，时间单位：1ms
    uint16_t intervalCnting;    //闪烁间隔时间计数，时间单位：1ms
    uint16_t intervalSet;    //闪烁间隔时间，时间单位：1ms
    uint16_t flashCount;    //连续闪烁次数
    void (*ledOn)(void);
    void (*ledOff)(void);
}ledCtrlParams_ts;
#define MAX_LED_COUNT 2
typedef struct 
{
    ledSta_tn sta;
    ledCtrlParams_ts list[MAX_LED_COUNT];
}ledParams_ts;
void LED_Init(void);//初始化
void testAllLed(void);
void LED1_ON(void);
void LED1_OFF(void);
void LED1_TOGGLE(void);
void LED1_ON_ONE(void);

void LED2_ON(void);
void LED2_OFF(void);
void LED2_TOGGLE(void);
void LED2_ON_ONE(void);

void beep_init(void);
void beep_onDriver(void);
void beep_longBeep(void);
void beep_shortBeep(void);
void beep_setFreq(uint8_t freq);
#endif
