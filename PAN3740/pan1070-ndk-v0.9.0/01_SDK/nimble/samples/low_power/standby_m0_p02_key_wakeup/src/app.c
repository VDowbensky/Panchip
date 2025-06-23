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

static void wakeup_p02_key_init(uint8_t wakeup_edge)
{
    /* Configure GPIO P02 (WKUP Key) as Lowlevel Wakeup */

    /* Configure P02 wakeup level due to wakeup_edge */
    LP_SetExternalWake(ANA, wakeup_edge);

    /* Set pinmux func of P02 as GPIO */
    SYS_SET_MFP(P0, 2, GPIO);

    /*
     * Construct GPIO init structure and Init GPIO P02:
     * - Set IO to digital input mode
     * - Enable internal pull-up or pull-down resistor depends the wakeup_edge
     */
    HAL_GPIO_InitTypeDef GPIO_InitStruct = {
        .mode   = HAL_GPIO_MODE_INPUT_DIGITAL,
    };
    if (wakeup_edge == 0) {
        /* Enable internal pull-up resistor if P02 is low level wakeup */
        GPIO_InitStruct.pull = HAL_GPIO_PULL_UP;
    } else {
        /* Enable internal pull-down resistor if P02 is high level wakeup */
        GPIO_InitStruct.pull = HAL_GPIO_PULL_DOWN;
    }
    HAL_GPIO_Init(P0_2, &GPIO_InitStruct);

    /* Wait for a while to ensure the internal pullup is stable before entering low power mode */
    soc_busy_wait(10000);
}

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
    case SOC_RST_REASON_STBM0_EXTIO_WAKEUP:
        APP_LOG("Standby Mode 0 EXTIO Wakeup (cnt = %d).\n", ++wkup_cnt);
        break;
    default:
        APP_LOG("Unhandled Reset Reason (%d), refer to more reason define in soc_api.h!\n", rst_reason);
    }

    /* Enable P02 low level wakeup for standby mode 0 */
    wakeup_p02_key_init(0);

    APP_LOG_INFO("Busy wait 100ms to keep SoC in active mode..\n");
    soc_busy_wait(100000);

    APP_LOG_INFO("Try to enter SoC sleep/deepsleep mode for 1000ms..\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
    APP_LOG_INFO("Waked up from SoC sleep/deepsleep mode.\n");

    APP_LOG_INFO("Try to enter SoC standby mode 0..\n\n");
    soc_busy_wait(1000); /* Wait for all log print done */
    soc_enter_standby_mode_0();

    APP_LOG("WARNING: Failed to enter SoC standby mode 0 due to unexpected interrupt detected.\n");
    APP_LOG("         Please check if there is an unhandled interrupt during the standby mode 0\n");
    APP_LOG("         entering flow.\n");
}

/**
 *******************************************************************************
 * @brief application main loop
 *******************************************************************************
 */
void app_main_loop(void)
{
    while (1) {
        /* Busy wait */
    }
}
