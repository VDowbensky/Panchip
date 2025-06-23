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
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include "app.h"
#include "app_log.h"

/* Configure App Tasks */
#define APP_TASK_1_STACK_SIZE   256
#define APP_TASK_1_PRIORITY     1
#define APP_TASK_2_STACK_SIZE   256
#define APP_TASK_2_PRIORITY     2

void app_task1(void *arg)
{
    while (1) {
        APP_LOG_INFO("Hello from %s!\n", __func__);
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay 500ms
    }
}

void app_task2(void *arg)
{
    while (1) {
        APP_LOG_INFO("Hello from %s!\n", __func__);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1s
    }
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    BaseType_t r;

    APP_LOG_INFO("App started..\n\n");

    /* Create an App Task 1 */
    r = xTaskCreate(app_task1,              // Task Function
                    "App Task 1",           // Task Name
                    APP_TASK_1_STACK_SIZE,  // Task Stack Size in Word
                    NULL,                   // Task Parameter
                    APP_TASK_1_PRIORITY,    // Task Priority
                    NULL                    // Task Handle
    );

    /* Check if task has been successfully created */
    if (r != pdPASS) {
        APP_LOG_ERR("Error, App Task 1 created failed!\n");
    }

    /* Create an App Task 2 */
    r = xTaskCreate(app_task2,              // Task Function
                    "App Task 2",           // Task Name
                    APP_TASK_2_STACK_SIZE,  // Task Stack Size in Word
                    NULL,                   // Task Parameter
                    APP_TASK_2_PRIORITY,    // Task Priority
                    NULL                    // Task Handle
    );

    /* Check if task has been successfully created */
    if (r != pdPASS) {
        APP_LOG_ERR("Error, App Task 2 created failed!\n");
    }
}

/**
 *******************************************************************************
 * @brief application main loop
 *******************************************************************************
 */
void app_main_loop(void)
{
    // Do nothing here.
    // We just omit the main task loop and let main() function return
    // back to the main_task() in os_setup.c and destroy itself then.
}
