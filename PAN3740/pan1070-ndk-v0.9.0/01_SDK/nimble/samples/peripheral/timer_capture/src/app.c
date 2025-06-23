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


static void timer0_cap_event_handler(TIMER_Cb_Flag_Opt evt_flag)
{
    static bool cap_flag = false;

    if(cap_flag) {
        APP_LOG_INFO("high level %lldus\n",(uint64_t)TIMER_GetCaptureData(TIMER0_OBJ.TIMERx) * 1000000 / TIMER0_OBJ.initObj.freq);
    } else {
        APP_LOG_INFO("low level %lldus\n",(uint64_t)TIMER_GetCaptureData(TIMER0_OBJ.TIMERx) * 1000000 / TIMER0_OBJ.initObj.freq);
    }
    cap_flag = !cap_flag;
}

static void timer0_capture_init(void)
{
    SYS_SET_MFP(P1, 5, TIMER0_EXT);
    GPIO_EnableDigitalPath(P1, BIT5);

    TIMER0_OBJ.initObj.mode = TIMER_MODE_INCAP;
    TIMER0_OBJ.initObj.cntMode = TIMER_CONTINUOUS_MODE;
    TIMER0_OBJ.initObj.freq = FREQ_1MHZ;
    TIMER0_OBJ.initObj.tmr0CmpSel = TMR0_COMPARATOR_SEL_CMP;
    TIMER0_OBJ.initObj.capSrc = TIMER_CAPTURE_SOURCE_EXT_PIN;
    TIMER0_OBJ.initObj.capMode =  TIMER_CAPTURE_COUNTER_RESET_MODE;
    TIMER0_OBJ.initObj.capEdge =  TIMER_CAPTURE_BOTH_EDGE;
    TIMER0_OBJ.callback = timer0_cap_event_handler;

    HAL_TIMER_Init(&TIMER0_OBJ);

    HAL_TIMER_Start(&TIMER0_OBJ);
}

static void pwm_init(void)
{
    HAL_PWM_PinConfiguration(P2, 2, PWM_CH0);

    PWM_InitOpt pwm_init_obj  = {
        .frequency = 20,
        .dutyCycle = 30,
        .operateType = OPERATION_EDGE_ALIGNED,
        .lowPowerEn = DISABLE,
    };

    int handle = HAL_PWM_Init(PWM0_CH0, &pwm_init_obj);

    if (handle >= 0) {
        HAL_PWM_Start(handle);
    }
}

static void hal_bsp_init(void)
{
    pwm_init();

    timer0_capture_init();
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
