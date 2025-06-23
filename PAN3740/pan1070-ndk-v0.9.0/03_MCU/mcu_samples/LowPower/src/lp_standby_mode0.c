/*******************************************************************************
 * @note      Copyright (C) 2024 Shanghai Panchip Microelectronics Co., Ltd.
 *            All rights reserved.
 * @file      lp_standby_mode0.c
 * @brief     Low power Standby Mode 0 test code.
*******************************************************************************/
#include "PanSeries.h"
#include "platform.h"

static void LP_PrintStbM0TestInfo(void)
{
    SYS_TEST("\n");
    SYS_TEST("+---------------------------------------------------------+\n");
    SYS_TEST("|    Press key to choose a standby mode 0 wakeup source:  |\n");
    SYS_TEST("|                                                         |\n");
    SYS_TEST("|    Input 'A'    Wake up by special IO P02.              |\n");
    SYS_TEST("|    Press ESC key to back to the top level case list.    |\n");
    SYS_TEST("+---------------------------------------------------------+\n");
}

void wakeup_p02_key_init(uint8_t wakeup_edge)
{
    /* Configure GPIO P02 (WKUP Key) as Lowlevel Wakeup */

    /* Configure P02 wakeup level due to wakeup_edge */
    LP_SetExternalWake(ANA, wakeup_edge);

    /* Set pinmux func of P56 as GPIO */
    SYS_SET_MFP(P0, 2, GPIO);
    /* Set P02 to input mode */
    GPIO_SetMode(P0, BIT2, GPIO_MODE_INPUT);

    if (wakeup_edge == 0) {
        /* Enable internal pull-up resistor if P02 is low level wakeup */
        GPIO_EnablePullupPath(P0, BIT2);
    } else {
        /* Enable internal pull-down resistor if P02 is high level wakeup */
        GPIO_EnablePulldownPath(P0, BIT2);
    }

    /* Necessary for P02 to do manual 3v aon sync */
    CLK_Wait3vSyncReady();

    // Wait for a while to ensure the internal pullup resistor is stable before entering low power mode
    // This delay is not needed if internal pull-up or pull-down resistor is not in use.
    SYS_delay_10nop(50000);
}

/*
 * The 3 special IOs (P00/P01/P02) can be configured for waking up SoC from Standby Mode 0.
 */
static void LP_StandbyMode0SpecialIoWakeup(void)
{
    SYS_TEST("\nConfigure Special IO P02 as input mode and enable low-level wakeup detection..\n");

    /* Enable P02 low level wakeup for standby mode 0 */
    wakeup_p02_key_init(0);

#if CONFIG_UART_LOG_ENABLE
    // Disable digital path of Log UART Rx IO to avoid possible current leakage before enter DeepSleep mode
    SOC_DisableLogUartRxPath();
#endif

    // Try to enter SoC Standby Mode 0
    LP_EnterStandbyMode0(ANA, false);

    SYS_TEST("WARNING: Failed to enter SoC standby mode 0 due to unexpected interrupt detected.\n");
    SYS_TEST("         Please check if there is an unhandled interrupt during the standby mode 0\n");
    SYS_TEST("         entering flow.\n");

    while (1) {
        /* Busy wait */
    }
}

void LP_TestStandbyMode0(void)
{
    char c;

    while (1) {
        LP_PrintStbM0TestInfo();
        /* Force clear UART Rx FIFO before getchar() */
#if CONFIG_LOG_UART_RX_PIN < 4
        UART_ResetRxFifo(UART0);
#else
        UART_ResetRxFifo(UART1);
#endif
        switch (c = getchar()) {
        case 'A':
        case 'a':
            LP_StandbyMode0SpecialIoWakeup();
            break;
        case 0x1B:  // Keyboard code <ESC>
            return;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
}
