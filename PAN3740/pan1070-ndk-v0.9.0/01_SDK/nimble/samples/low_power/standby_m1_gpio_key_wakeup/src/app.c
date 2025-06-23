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

static uint32_t wkup_cnt = 0;

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

static void parse_stbm1_gpio_wakeup_source(void)
{
    uint32_t src;

    src = soc_stbm1_gpio_wakeup_src_get();

    APP_LOG_INFO("gpio wakeup src flag = 0x%08x\n", src);

    for (size_t i = 0; i < 32; i++) {
        if (src & (1u << i)) {
            APP_LOG_INFO("SoC is waked up by GPIO P%d_%d.\n", i / 8, i % 8);
        }
    }
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

    /*
     * Clear possible GPIO pending IRQs to avoid unexpected GPIO isr run
     * after NVIC GPIO IRQ enabled.
     * [NOTE] GPIO pending IRQs would be generated right after SoC waking
     *        up from Standby Mode 1 by GPIO interrupt.
     */
    NVIC_ClearPendingIRQ(GPIO0_IRQn);
    NVIC_ClearPendingIRQ(GPIO1_IRQn);

    /* Enable GPIO IRQs in NVIC */
    NVIC_EnableIRQ(GPIO0_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);
}

#if CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET
void restore_hw_peripherals(void)
{
#if CONFIG_UART_LOG_ENABLE
    UART_InitTypeDef Init_Struct = {
        .UART_BaudRate = 921600,
        .UART_LineCtrl = Uart_Line_8n1,
    };
    /* Re-init UART */
    UART_Init(UART0, &Init_Struct);
    UART_EnableFifo(UART0);
    /* Set IO pinmux to UART again */
    set_uart_io();
#endif
    /* Re-enable NVIC GPIO IRQs */
    NVIC_EnableIRQ(GPIO0_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);
    /* Re-init GPIO keys */
    wakeup_gpio_keys_init();
}
#endif /* CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET */

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    uint8_t rst_reason;

    /* Get the last reset reason */
    APP_LOG_INFO("Reset Reason: ");
    rst_reason = soc_reset_reason_get();
    switch (rst_reason) {
    case SOC_RST_REASON_POR_RESET:
        APP_LOG("Power On Reset.\n");
        break;
    case SOC_RST_REASON_PIN_RESET:
        APP_LOG("nRESET Pin Reset.\n");
        break;
    case SOC_RST_REASON_SYS_RESET:
        APP_LOG("NVIC System Reset.\n");
        break;
    case SOC_RST_REASON_STBM1_GPIO_WAKEUP:
        APP_LOG("Standby Mode 1 GPIO Wakeup (cnt = %d).\n", ++wkup_cnt);
        parse_stbm1_gpio_wakeup_source();
        break;
    default:
        APP_LOG("Unhandled Reset Reason (%d), refer to more reason define in soc_api.h!\n", rst_reason);
    }
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

    /* Enable 3 GPIO keys low-level wakeup for standby mode 1 */
    wakeup_gpio_keys_init();

    APP_LOG_INFO("Try to enter SoC sleep/deepsleep mode and wait for 3-times key pressing..\n");
    /*
     * Wait to be notified that gpio key is pressed (gpio irq occured). Note the first parameter
     * is pdFALSE, which has the effect of decrease the task's notification value by 1, making
     * the notification value act like a counting semaphore.
     */
    ulNotificationValue = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    APP_LOG_INFO("First key pressed, notify value = %d.\n", ulNotificationValue);
    ulNotificationValue = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    APP_LOG_INFO("Second key pressed, notify value = %d.\n", ulNotificationValue);
    ulNotificationValue = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    APP_LOG_INFO("Third key pressed, notify value = %d.\n", ulNotificationValue);

    APP_LOG_INFO("Busy wait 500ms to keep SoC in active mode..\n");
    soc_busy_wait(500000);

#if !CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET
    APP_LOG_INFO("Now try to enter SoC standby mode 1 (wakeup reset)..\n\n");
#if CONFIG_UART_LOG_ENABLE
    /* Waiting for UART Tx done and re-set UART IO before entering standby mode 1 to avoid current leakage */
    reset_uart_io();
#endif
    /* Enter standby mode1 with all sram power off */
    soc_enter_standby_mode_1(STBM1_WAKEUP_SRC_GPIO, STBM1_RETENTION_SRAM_NONE);

    APP_LOG("WARNING: Failed to enter SoC standby mode 1 due to unexpected interrupt detected.\n");
    APP_LOG("         Please check if there is an unhandled interrupt during the standby mode 1\n");
    APP_LOG("         entering flow.\n");

    while (1) {
        /* Busy wait */
    }
#else
    while (1) {
        APP_LOG_INFO("Now try to enter SoC standby mode 1 (wakeup continuous run)..\n\n");
#if CONFIG_UART_LOG_ENABLE
        /* Waiting for UART Tx done and re-set UART IO before entering standby mode 1 to avoid current leakage */
        reset_uart_io();
#endif
        /* Enter standby mode1 with all common 48KB sram retention */
        soc_enter_standby_mode_1(STBM1_WAKEUP_SRC_GPIO, STBM1_RETENTION_SRAM_BLOCK0 | STBM1_RETENTION_SRAM_BLOCK1);
        /* Restore hardware peripherals manually */
        restore_hw_peripherals();
        APP_LOG_INFO("Waked up from SoC standby mode 1 and continue run (cnt = %d)..\n", ++wkup_cnt);
    }
#endif /* CONFIG_PM_STANDBY_M1_WAKEUP_WITHOUT_RESET */
}
