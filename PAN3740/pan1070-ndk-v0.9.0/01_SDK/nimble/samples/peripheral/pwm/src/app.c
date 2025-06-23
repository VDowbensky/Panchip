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


#define PWM_LOW_POWER_TEST          0


static void hal_bsp_init(void)
{
    PWM_InitOpt init_opt;

    init_opt.frequency = 1000;
    init_opt.dutyCycle = 30;
    init_opt.inverter = 1;
    init_opt.lowPowerEn = PWM_LOW_POWER_TEST;
    init_opt.mode = PWM_MODE_INDEPENDENT;
    init_opt.operateType = OPERATION_EDGE_ALIGNED;

    HAL_PWM_PinConfiguration(P0, 3, PWM_CH2);

    int handle = HAL_PWM_Init(PWM0_CH2, &init_opt);

    if (handle >= 0) {
        HAL_PWM_Start(handle);
    }
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    /* Init PWM module */
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
        // No nothing here
    }
}
