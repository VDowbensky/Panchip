/**
 *******************************************************************************
 * @file     app.c
 * @create   2024-12-10
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "FreeRTOS.h"
#include "soc_api.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include "app.h"
#include "app_log.h"


// #define TIMER0_MULTI_CMP

volatile uint32_t tmr0_evt0_cnt = 0;
volatile uint32_t tmr0_evt1_cnt = 0;
volatile uint32_t tmr0_evt2_cnt = 0;

static void timer0_event_handler(TIMER_Cb_Flag_Opt evt_flag)
{
    uint32_t timer0_flag = TIMER_GetTFFlag(TIMER0_OBJ.TIMERx);

    TIMER_ClearTFFlag(TIMER0_OBJ.TIMERx, timer0_flag);
    if(timer0_flag == TIMER_INTSTS_TF0_Msk) {
        tmr0_evt0_cnt++;
        APP_LOG_INFO("tmr0 cnt0 %d\n", tmr0_evt0_cnt);
    } else if(timer0_flag == TIMER_INTSTS_TF1_Msk) {
        tmr0_evt1_cnt++;
        APP_LOG_INFO("tmr0 cnt1 %d\n", tmr0_evt1_cnt);
    } else if(timer0_flag == TIMER_INTSTS_TF2_Msk) {
        tmr0_evt2_cnt++;
        APP_LOG_INFO("tmr0 cnt2 %d\n", tmr0_evt2_cnt);
    }
}

static void timer1_event_handler(TIMER_Cb_Flag_Opt evt_flag)
{
    static bool tmr0_ctl_flag = false;

    if(tmr0_ctl_flag) {
        HAL_TIMER_Start(&TIMER0_OBJ);
        APP_LOG_INFO("tmr0 start\n");
    } else {
        HAL_TIMER_Stop(&TIMER0_OBJ);
        APP_LOG_INFO("tmr0 stop\n");
    }

    tmr0_ctl_flag = !tmr0_ctl_flag;
}

static void timer0_init(void)
{
    TIMER0_OBJ.initObj.mode = TIMER_MODE_BASECNT;
    TIMER0_OBJ.initObj.cntMode = TIMER_PERIODIC_MODE;
    TIMER0_OBJ.initObj.freq = FREQ_16MHZ;
    TIMER0_OBJ.initObj.tmr0CmpSel = TMR0_COMPARATOR_SEL_CMP;
    TIMER0_OBJ.initObj.cmpValue = HAL_TIMER_MS_TO_CNT(1000, TIMER0_OBJ.initObj.freq);          //1s
    TIMER0_OBJ.callback = timer0_event_handler;
    HAL_TIMER_Init(&TIMER0_OBJ);

#ifdef TIMER0_MULTI_CMP
    TIMER0_OBJ.InitObj.CntMode = TIMER_CONTINUOUS_MODE;
    TIMER0_OBJ.InitObj.Tmr0CmpSel = TMR0_COMPARATOR_SEL_CMP1;
    TIMER0_OBJ.InitObj.CmpValue = HAL_TIMER_MS_TO_CNT(2000, TIMER0_OBJ.InitObj.Freq);
    HAL_TIMER_Init(&TIMER0_OBJ);

    TIMER0_OBJ.InitObj.Tmr0CmpSel = TMR0_COMPARATOR_SEL_CMP2;
    TIMER0_OBJ.InitObj.CmpValue = HAL_TIMER_MS_TO_CNT(3000, TIMER0_OBJ.InitObj.Freq);
    HAL_TIMER_Init(&TIMER0_OBJ);
#endif

    HAL_TIMER_Start(&TIMER0_OBJ);
}

static void timer1_init(void)
{
    TIMER1_OBJ.initObj.mode = TIMER_MODE_BASECNT;
    TIMER1_OBJ.initObj.cntMode = TIMER_PERIODIC_MODE;
    TIMER1_OBJ.initObj.freq = FREQ_1MHZ;
    TIMER1_OBJ.initObj.cmpValue = HAL_TIMER_MS_TO_CNT(5000, TIMER1_OBJ.initObj.freq);        //5s
    TIMER1_OBJ.callback = timer1_event_handler;

    HAL_TIMER_Init(&TIMER1_OBJ);

    HAL_TIMER_Start(&TIMER1_OBJ);
}

static void hal_bsp_init(void)
{
    timer0_init();
    timer1_init();
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    /* Init APB Timer module */
    hal_bsp_init();
}

/**
 *******************************************************************************
 * @brief application main loop
 *******************************************************************************
 */
void app_main_loop(void)
{
    while(1) {
        // Do nothing here
    }
}
