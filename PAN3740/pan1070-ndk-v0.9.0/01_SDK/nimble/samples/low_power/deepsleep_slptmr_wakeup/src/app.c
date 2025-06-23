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

static void indication_gpio_init(void)
{
    /* Set pinmux func of P13/P14/P15 as GPIO */
    SYS_SET_MFP(P1, 3, GPIO);
    SYS_SET_MFP(P1, 4, GPIO);
    SYS_SET_MFP(P1, 5, GPIO);

    /* Configure GPIO P13/P14/P15 to push-pull output mode (with init high level) */
    HAL_GPIO_InitTypeDef GPIO_InitStruct = {
        .mode   = HAL_GPIO_MODE_OUTPUT_PUSHPULL,
        .level   = HAL_GPIO_LEVEL_HIGH,
    };
    HAL_GPIO_Init(P1_3, &GPIO_InitStruct);
    HAL_GPIO_Init(P1_4, &GPIO_InitStruct);
    HAL_GPIO_Init(P1_5, &GPIO_InitStruct);
}

/* This function overrides the reserved weak function with same name in soc_pm.c */
void sleep_timer1_handler(void)
{
    HAL_GPIO_WritePin(P1_4, HAL_GPIO_LEVEL_LOW);
    HAL_GPIO_WritePin(P1_4, HAL_GPIO_LEVEL_HIGH);
    APP_LOG_INFO("[Uptime: %d ms] SleepTimer 1 IRQ triggered.\n", soc_lptmr_uptime_get_ms());
}

/* This function overrides the reserved weak function with same name in soc_pm.c */
void sleep_timer2_handler(void)
{
    HAL_GPIO_WritePin(P1_5, HAL_GPIO_LEVEL_LOW);
    HAL_GPIO_WritePin(P1_5, HAL_GPIO_LEVEL_HIGH);
    APP_LOG_INFO("[Uptime: %d ms] SleepTimer 2 IRQ triggered.\n", soc_lptmr_uptime_get_ms());
}

static void slptmr_init(void)
{
    /* Configure timeout of SleepTimer 1&2 with interrupt and wakeup enabled */

    /*
     * Configure timeout:
     * timeout1 = SLPTMR1_TIMEOUT_CNT / PAN_RCC_CLOCK_FREQUENCY_SLOW
     *         = (32000 / 2) / 32000 (s)
     *         = 0.5 s = 500 ms
     * timeout2 = SLPTMR2_TIMEOUT_CNT / PAN_RCC_CLOCK_FREQUENCY_SLOW
     *         = 32000 / 32000 (s)
     *         = 1 s
     */
    const uint32_t SLPTMR1_TIMEOUT_CNT = soc_32k_clock_freq_get() / 2;
    const uint32_t SLPTMR2_TIMEOUT_CNT = soc_32k_clock_freq_get();

    /* Set timeout of SleepTimer 1 */
    LP_SetSleepTime(ANA, SLPTMR1_TIMEOUT_CNT, LP_SLPTMR_CH1);
    /* Set timeout of SleepTimer 2 */
    LP_SetSleepTime(ANA, SLPTMR2_TIMEOUT_CNT, LP_SLPTMR_CH2);
}


/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    /* Init specific GPIO as output for ISR indication use */
    indication_gpio_init();
    /* Init Sleep Timers for wake up use */
    slptmr_init();
}

/**
 *******************************************************************************
 * @brief application main loop
 *******************************************************************************
 */
void app_main_loop(void)
{
    while (1) {
        APP_LOG_INFO("[Uptime: %d ms] Main Thread sleep..\n", soc_lptmr_uptime_get_ms());
        /* Set OS Task Delay (Affects timeout of SleepTimer 0 */
        vTaskDelay(pdMS_TO_TICKS(5000));
        HAL_GPIO_WritePin(P1_3, HAL_GPIO_LEVEL_LOW);
        HAL_GPIO_WritePin(P1_3, HAL_GPIO_LEVEL_HIGH);
        APP_LOG_INFO("[Uptime: %d ms] Main Thread waked up.\n", soc_lptmr_uptime_get_ms());
    }
}
