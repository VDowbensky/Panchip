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

/* Stores the handle of the task that will be notified when the transmission is complete. */
static TaskHandle_t xTaskToNotify = NULL;

#if CONFIG_UART_LOG_ENABLE
CONFIG_RAM_CODE static void reset_uart_io(void)
{
    /* Wait until all UART0 data sending done before entering deepsleep mode */
    while (!(UART_GetLineStatus(UART0) & UART_LINE_TXSR_EMPTY)) {
        /* Busy wait */
    }

    /*
     * Reset UART PINs to GPIO function and disable digital input path of UART Rx PIN
     * to avoid possible current leakage.
     */
    SYS_SET_MFP(P1, 6, GPIO);
    SYS_SET_MFP(P1, 7, GPIO);
    GPIO_DisableDigitalPath(P1, BIT7);
}

CONFIG_RAM_CODE static void set_uart_io(void)
{
    /* Resume UART PIN Configurations to reenable UART function */
    SYS_SET_MFP(P1, 6, UART0_TX);
    SYS_SET_MFP(P1, 7, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT7);
}
#endif /* CONFIG_UART_LOG_ENABLE */


CONFIG_RAM_CODE void vSocDeepSleepEnterHook(void)
{
#if CONFIG_UART_LOG_ENABLE
    reset_uart_io();
#endif
}

CONFIG_RAM_CODE void vSocDeepSleepExitHook(void)
{
#if CONFIG_UART_LOG_ENABLE
    set_uart_io();
#endif
}

static void gpio_p06_input_callback(HAL_GPIO_IntMode intMode)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (intMode == HAL_GPIO_INT_FALLING) {
        /* Notify the task that KEY1 is pressed (P06 falling edge detected). */
        vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
        APP_LOG_INFO("==== KEY1 Pressed! (P0_6 falling edge detected) ====\n");
    }

    /* Yield if xHigherPriorityTaskWoken is true. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void gpio_p12_input_callback(HAL_GPIO_IntMode intMode)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (intMode == HAL_GPIO_INT_FALLING) {
        /* Notify the task that KEY1 is pressed (P06 falling edge detected). */
        vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
        APP_LOG_INFO("==== KEY2 Pressed! (P1_2 falling edge detected) ====\n");
    }

    /* Yield if xHigherPriorityTaskWoken is true. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void gpio_p02_input_callback(HAL_GPIO_IntMode intMode)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (intMode == HAL_GPIO_INT_FALLING) {
        /* Notify the task that KEY1 is pressed (P06 falling edge detected). */
        vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
        APP_LOG_INFO("==== WKUP Pressed! (P0_2 falling edge detected) ====\n");
    }

    /* Yield if xHigherPriorityTaskWoken is true. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void wakeup_gpio_keys_init(void)
{
    /* Configure GPIO P06 (KEY1) / P12 (KEY2) / P02 (WKUP) as Falling Edge Interrupt/Wakeup */

    /* Set pinmux func as GPIO */
    SYS_SET_MFP(P0, 6, GPIO);
    SYS_SET_MFP(P1, 2, GPIO);
    SYS_SET_MFP(P0, 2, GPIO);

    /* Configure debounce clock to 32K clock */
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_RCL, GPIO_DBCTL_DBCLKSEL_4);

    /*
     * Construct GPIO init structure and Init GPIO P06/P12/P02:
     * - Set IO to digital input mode
     * - Enable internal pull-up resistor
     */
    HAL_GPIO_InitTypeDef GPIO_InitStruct = {
        .mode   = HAL_GPIO_MODE_INPUT_DIGITAL,
        .pull   = HAL_GPIO_PULL_UP,
    };
    HAL_GPIO_Init(P0_6, &GPIO_InitStruct);
    HAL_GPIO_Init(P1_2, &GPIO_InitStruct);
    HAL_GPIO_Init(P0_2, &GPIO_InitStruct);

    /* Construct GPIO interrupt init structure */
    HAL_GPIO_IntInitTypeDef GPIO_IntInitStruct = {
        .intMode        = HAL_GPIO_INT_FALLING,
        .debounce       = ENABLE,
    };
    /* Init GPIO P06 interrupt */
    GPIO_IntInitStruct.callbackFunc = gpio_p06_input_callback;
    HAL_GPIO_InterruptInit(P0_6, &GPIO_IntInitStruct);
    /* Init GPIO P12 interrupt */
    GPIO_IntInitStruct.callbackFunc = gpio_p12_input_callback;
    HAL_GPIO_InterruptInit(P1_2, &GPIO_IntInitStruct);
    /* Init GPIO P02 interrupt */
    GPIO_IntInitStruct.callbackFunc = gpio_p02_input_callback;
    HAL_GPIO_InterruptInit(P0_2, &GPIO_IntInitStruct);

    /* Enable GPIO IRQs in NVIC */
    NVIC_EnableIRQ(GPIO0_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    wakeup_gpio_keys_init();
}

/**
 *******************************************************************************
 * @brief application main loop
 *******************************************************************************
 */
void app_main_loop(void)
{
    uint32_t ulNotificationValue;

    /* Store the handle of current task. */
    xTaskToNotify = xTaskGetCurrentTaskHandle();
    if(xTaskToNotify == NULL) {
        APP_LOG_ERR("Error, get current task handle failed!\n");
    }

    while (1) {
        APP_LOG_INFO("Wait for Task Notifications..\n");

        /*
         * Wait to be notified that gpio key is pressed (gpio irq occured). Note the first parameter
         * is pdTRUE, which has the effect of clearing the task's notification value back to 0, making
         * the notification value act like a binary (rather than a counting) semaphore.
         */
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        APP_LOG_INFO("A notification received, value: %d.\n\n", ulNotificationValue);
    }
}
