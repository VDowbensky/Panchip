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
    /* Set pinmux of P10 to GPIO (default) */
    SYS_SET_MFP(P1, 0, GPIO);
    /* Init GPIO P10 to push-pull mode */
    HAL_GPIO_InitTypeDef GPIO_InitStruct = {
        .mode   = HAL_GPIO_MODE_OUTPUT_PUSHPULL,
        .pull   = HAL_GPIO_PULL_DISABLE,
        .level  = HAL_GPIO_LEVEL_HIGH,
    };
    HAL_GPIO_Init(P1_0, &GPIO_InitStruct);
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    /* Init GPIO P10 as Open-Drain Output */
    hal_bsp_init();
}

/**
 *******************************************************************************
 * @brief application main loop
 *******************************************************************************
 */
void app_main_loop(void)
{
    /* Blink EVB LED3 (Connected to P10) */
    while (1) {
        HAL_GPIO_WritePin(P1_0, HAL_GPIO_LEVEL_HIGH);
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay 500ms
        HAL_GPIO_WritePin(P1_0, HAL_GPIO_LEVEL_LOW);
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay 500ms
    }
}
