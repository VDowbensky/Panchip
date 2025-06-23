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


#define WDT_CLK_SRC        CLK_APB1_WDTSEL_MILLI_PULSE

static void WDT_INT_Callback()
{
    HAL_WDT_Feed(NULL);
    APP_LOG_INFO("Feed Dog\n");
}

void hal_bsp_init(void)
{
    WDT_Init_Opt WDT_Init_Obj;

    WDT_Init_Obj.ClockSource = WDT_CLK_SRC;
    WDT_Init_Obj.Timeout = WDT_TIMEOUT_2POW12;
    WDT_Init_Obj.ResetDelay = WDT_RESET_DELAY_1025CLK;
    WDT_Init_Obj.ResetSwitch = true;
    WDT_Init_Obj.WakeupSwitch = false;
    HAL_WDT_Init(&WDT_Init_Obj);

    WDT_Interrupt_Opt WDT_Interrupt_Obj;

    WDT_Interrupt_Obj.CallbackFunc = WDT_INT_Callback;
    HAL_WDT_Init_INT(&WDT_Interrupt_Obj);
}


/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    /* Init peripheral module */
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
