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
#include "soc_api.h"

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

static void gpio_p13_input_callback(HAL_GPIO_IntMode intMode)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (intMode == HAL_GPIO_INT_RISING) {
        /* Notify the task that P13 rising edge detected. */
        vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
        /* Toggle IO P15 */
        HAL_GPIO_WritePin(P1_5, HAL_GPIO_LEVEL_HIGH);
        HAL_GPIO_WritePin(P1_5, HAL_GPIO_LEVEL_LOW);
        APP_LOG_INFO("[Uptime: %d ms] GPIO P13 IRQ triggered.\n", soc_lptmr_uptime_get_ms());
    }

    /* Yield if xHigherPriorityTaskWoken is true. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void gpio_p14_input_callback(HAL_GPIO_IntMode intMode)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (intMode == HAL_GPIO_INT_RISING) {
        /* Notify the task that P14 rising edge detected. */
        vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
        /* Toggle IO P15 */
        HAL_GPIO_WritePin(P1_5, HAL_GPIO_LEVEL_HIGH);
        HAL_GPIO_WritePin(P1_5, HAL_GPIO_LEVEL_LOW);
        APP_LOG_INFO("[Uptime: %d ms] GPIO P14 IRQ triggered.\n", soc_lptmr_uptime_get_ms());
    }

    /* Yield if xHigherPriorityTaskWoken is true. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void gpio_init(void)
{
    /* Configure GPIO P13/P14 as Rising Edge Interrupt/Wakeup */

    /* Set pinmux func as GPIO */
    SYS_SET_MFP(P1, 3, GPIO);
    SYS_SET_MFP(P1, 4, GPIO);

    /* Configure debounce clock to 32K clock */
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_RCL, GPIO_DBCTL_DBCLKSEL_4);

    /*
     * Construct GPIO init structure and Init GPIO P13/P14:
     * - Set IO to digital input mode
     * - Disable internal pull-up resistor
     */
    HAL_GPIO_InitTypeDef GPIO_InitStruct = {
        .mode   = HAL_GPIO_MODE_INPUT_DIGITAL,
        .pull   = HAL_GPIO_PULL_DISABLE,
    };
    HAL_GPIO_Init(P1_3, &GPIO_InitStruct);
    HAL_GPIO_Init(P1_4, &GPIO_InitStruct);

    /* Construct GPIO interrupt init structure */
    HAL_GPIO_IntInitTypeDef GPIO_IntInitStruct = {
        .intMode        = HAL_GPIO_INT_RISING,
        .debounce       = DISABLE,
    };
    /* Init GPIO P13 interrupt */
    GPIO_IntInitStruct.callbackFunc = gpio_p13_input_callback;
    HAL_GPIO_InterruptInit(P1_3, &GPIO_IntInitStruct);
    /* Init GPIO P14 interrupt */
    GPIO_IntInitStruct.callbackFunc = gpio_p14_input_callback;
    HAL_GPIO_InterruptInit(P1_4, &GPIO_IntInitStruct);

    /* Enable GPIO IRQ in NVIC */
    NVIC_EnableIRQ(GPIO1_IRQn);

    /* Configure GPIO P15 to push-pull output mode (with init low level) */
    SYS_SET_MFP(P1, 5, GPIO);
    GPIO_InitStruct.mode = HAL_GPIO_MODE_OUTPUT_PUSHPULL;
    GPIO_InitStruct.level = HAL_GPIO_LEVEL_LOW;
    HAL_GPIO_Init(P1_5, &GPIO_InitStruct);
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    /*
     * Init specific GPIOs:
     * - to input mode for wake up use
     * - to output mode for ISR indication use
     */
    gpio_init();
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
         * Wait to be notified that gpio gpio irq occured. Note the first parameter is pdTRUE,
         * which has the effect of clearing the task's notification value back to 0, making
         * the notification value act like a binary (rather than a counting) semaphore.
         */
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        APP_LOG_INFO("A notification received, value: %d.\n\n", ulNotificationValue);
    }
}
