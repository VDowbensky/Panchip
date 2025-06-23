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


void hal_bsp_init(void)
{
    /* Set pinmux of P06 to GPIO (default) */
    SYS_SET_MFP(P0, 6, GPIO);
    /* Init GPIO P06 to digital input mode */
    HAL_GPIO_InitTypeDef GPIO_InitStruct = {
        .mode   = HAL_GPIO_MODE_INPUT_DIGITAL,
        .pull   = HAL_GPIO_PULL_UP,
    };
    HAL_GPIO_Init(P0_6, &GPIO_InitStruct);
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    /* Init GPIO P06 as Digital Input */
    hal_bsp_init();
}

/**
 *******************************************************************************
 * @brief application main loop
 *******************************************************************************
 */
void app_main_loop(void)
{
    HAL_GPIO_Level curr_level;
    HAL_GPIO_Level last_level = HAL_GPIO_LEVEL_HIGH;

    /* Polling EVB KEY1 (Connected to P06), and print key status when status changed */
    while (1) {
        curr_level = HAL_GPIO_ReadPin(P0_6);
        if (last_level != curr_level) {
            soc_busy_wait(1000); // Delay 1ms and read IO again for Input Debounce
            curr_level = HAL_GPIO_ReadPin(P0_6);
            if (last_level != curr_level) {
                last_level = curr_level;
                if (curr_level == HAL_GPIO_LEVEL_LOW) {
                    APP_LOG_INFO("KEY1 Pressed!\n");
                } else {
                    APP_LOG_INFO("KEY1 Released!\n");
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Delay 10ms for other task scheduling
    }
}
