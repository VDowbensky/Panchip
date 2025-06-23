/*******************************************************************************
 * @note      Copyright (C) 2024 Shanghai Panchip Microelectronics Co., Ltd.
 *            All rights reserved.
 * @file      lp_standby_mode1.c
 * @brief     Low power Standby Mode 1 test code.
*******************************************************************************/
#include "PanSeries.h"
#include "platform.h"

static void LP_PrintStbM1TestInfo(void)
{
    SYS_TEST("\n");
    SYS_TEST("+---------------------------------------------------------+\n");
    SYS_TEST("|    Press key to choose a standby mode 1 wakeup source:  |\n");
    SYS_TEST("|                                                         |\n");
    SYS_TEST("|    Input 'A'    Wake up by GPIO.                        |\n");
    SYS_TEST("|    Input 'B'    Wake up by SLPTMR.                      |\n");
    SYS_TEST("|    Press ESC key to back to the top level case list.    |\n");
    SYS_TEST("+---------------------------------------------------------+\n");
}

void LP_StandbyMode1GpioWakeup(void)
{
    bool continue_run_after_wakeup = false;
    uint8_t sram_power_ctrl = continue_run_after_wakeup ? LP_RETENTION_SRAM_ALL : LP_RETENTION_SRAM_NONE;

    SYS_TEST("\nConfigure GPIO P06 as input mode and enable falling edge interupt detection..\n");

    // Confiure GPIO P06 (EVB KEY1) before enter low power mode
    SYS_SET_MFP(P0, 6, GPIO);
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_RCL, GPIO_DBCTL_DBCLKSEL_4);   // Configure debounce clock to 32K to support goio edge wakeup
    GPIO_EnablePullupPath(P0, BIT6);
    GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);
    GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);

    // Wait for a while to ensure the internal pullup resistor is stable before entering low power mode
    // This delay is not needed if internal pull-up or pull-down resistor is not in use.
    SYS_delay_10nop(50000);

    // Enable related GPIO IRQ
    NVIC_EnableIRQ(GPIO0_IRQn);

#if CONFIG_UART_LOG_ENABLE
    // Disable digital path of Log UART Rx IO to avoid possible current leakage before enter DeepSleep mode
    SOC_DisableLogUartRxPath();
#endif

    // Try to enter SoC Standby Mode 1
    LP_EnterStandbyMode1(ANA, sram_power_ctrl, continue_run_after_wakeup);

    if (continue_run_after_wakeup) {
        // Re-set GPIO back to default analog input state
        NVIC_DisableIRQ(GPIO0_IRQn);
        GPIO_DisableInt(P0, 6);
        GPIO_DisablePullupPath(P0, BIT6);
        GPIO_DisableDigitalPath(P0, BIT6); // Disable digital input path to avoid possible current leakage

        // Re-Init log uart if needed
#if CONFIG_UART_LOG_ENABLE
        extern void SOC_LogUartInit();
        SOC_LogUartInit();
#endif
        SYS_TEST("\nSuccessfully waked up from Standby Mode 1 without reset!\n");
    }
}

void LP_StandbyMode1SlptmrWakeup(void)
{
    bool continue_run_after_wakeup = false;
    uint8_t sram_power_ctrl = continue_run_after_wakeup ? LP_RETENTION_SRAM_ALL : LP_RETENTION_SRAM_NONE;

    SYS_TEST("\nConfigure SleepTimer 2 with timeout interrupt..\n");

    /*
     * Configure timeout:
     * timeout = SLPTMR2_TIMEOUT_CNT / LOW_SPEED_CLK_FREQ
     *         = (32000 * 1) / 32000 (s)
     *         = 1 s
     */
    const uint32_t SLPTMR2_TIMEOUT_CNT = 32000 * 1;

    /* Set timeout of SleepTimer 1 */
    LP_SetSleepTime(ANA, SLPTMR2_TIMEOUT_CNT, LP_SLPTMR_CH2);

#if CONFIG_UART_LOG_ENABLE
    // Disable digital path of Log UART Rx IO to avoid possible current leakage before enter DeepSleep mode
    SOC_DisableLogUartRxPath();
#endif

    // Try to enter SoC Standby Mode 1
    LP_EnterStandbyMode1(ANA, sram_power_ctrl, continue_run_after_wakeup);

    if (continue_run_after_wakeup) {
#if CONFIG_UART_LOG_ENABLE
        extern void SOC_LogUartInit();
        SOC_LogUartInit();
#endif
		NVIC_EnableIRQ(SLPTMR_IRQn);
        // Disable slptmr 2
        NVIC_DisableIRQ(SLPTMR_IRQn);
        LP_SetSleepTime(ANA, 0x0, LP_SLPTMR_CH2);

        SYS_TEST("\nSuccessfully waked up from Standby Mode 1 without reset!\n");
    } else {
        SYS_TEST("WARNING: Failed to enter SoC standby mode 1 due to unexpected interrupt detected.\n");
        SYS_TEST("         Please check if there is an unhandled interrupt during the standby mode 1\n");
        SYS_TEST("         entering flow.\n");
        while (1) {
            /* Busy wait */
        }
    }
}

void LP_TestStandbyMode1(void)
{
    char c;

    while (1) {
        LP_PrintStbM1TestInfo();
        /* Force clear UART Rx FIFO before getchar() */
#if CONFIG_LOG_UART_RX_PIN < 4
        UART_ResetRxFifo(UART0);
#else
        UART_ResetRxFifo(UART1);
#endif
        switch (c = getchar()) {
        case 'A':
        case 'a':
            LP_StandbyMode1GpioWakeup();
            break;
        case 'B':
        case 'b':
            LP_StandbyMode1SlptmrWakeup();
            break;
        case 0x1B:  // Keyboard code <ESC>
            return;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
}
