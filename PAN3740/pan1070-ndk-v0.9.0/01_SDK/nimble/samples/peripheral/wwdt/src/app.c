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


#define WWDT_CLK_SRC        CLK_APB1_WWDTSEL_MILLI_PULSE

static void WWDT_INT_Callback()
{
    HAL_WWDT_Feed(NULL);
    APP_LOG_INFO("Feed Dog\n");
}

void hal_bsp_init(void)
{
    WWDT_Init_Opt WWDT_Init_Obj;
    CLK_SetWwdtClkSrc(CLK_APB1_WWDTSEL_MILLI_PULSE);

    WWDT_Init_Obj.ClockSource = WWDT_CLK_SRC;
    WWDT_Init_Obj.Prescaler = WWDT_PRESCALER_32;
    WWDT_Init_Obj.CmpValue = 25;
    HAL_WWDT_Init(&WWDT_Init_Obj);

    WWDT_Interrupt_Opt WWDT_Interrupt_Obj;

    WWDT_Interrupt_Obj.CallbackFunc = WWDT_INT_Callback;
    HAL_WWDT_Init_INT(&WWDT_Interrupt_Obj);
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
