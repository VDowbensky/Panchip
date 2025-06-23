/*******************************************************************************
 * @note      Copyright (C) 2024 Shanghai Panchip Microelectronics Co., Ltd.
 *            All rights reserved.
 * @file      lp_deepsleep_mode.c
 * @brief     Low power DeepSleep Mode test code.
*******************************************************************************/
#include "PanSeries.h"
#include "platform.h"

static void LP_PrintDeepSleepTestInfo(void)
{
    SYS_TEST("\n");
    SYS_TEST("+---------------------------------------------------------+\n");
    SYS_TEST("|    Press key to choose a deepsleep wakeup source:       |\n");
    SYS_TEST("|                                                         |\n");
    SYS_TEST("|    Input 'A'    Wake up by GPIO.                        |\n");
    SYS_TEST("|    Input 'B'    Wake up by TIMER.                       |\n");
    SYS_TEST("|    Input 'C'    Wake up by SLPTMR.                      |\n");
    SYS_TEST("|    Press ESC key to back to the top level case list.    |\n");
    SYS_TEST("+---------------------------------------------------------+\n");
}

 void LP_DeepSleepGpioWakeup(void)
{
    SYS_TEST("\nConfigure GPIO P06 as input mode and enable falling edge interupt detection..\n");

    // Confiure GPIO P06 (EVB KEY1) before enter low power mode
    SYS_SET_MFP(P0, 6, GPIO);
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_RCL, GPIO_DBCTL_DBCLKSEL_4);   // Configure debounce clock to 32K to support goio edge wakeup
    GPIO_EnableDebounce(P0, BIT6);  // Enable debouce of specified IO
    GPIO_EnablePullupPath(P0, BIT6);
    GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);
    GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);

    // Wait for a while to ensure the internal pullup resistor is stable before entering low power mode
    // This delay is not needed if internal pull-up or pull-down resistor is not in use.
    SYS_delay_10nop(0x10000);

    // Enable related GPIO IRQ
    NVIC_EnableIRQ(GPIO0_IRQn);

#if CONFIG_UART_LOG_ENABLE
    // Disable digital path of Log UART Rx IO to avoid possible current leakage before enter DeepSleep mode
    SOC_DisableLogUartRxPath();
#endif

    // Try to enter SoC DeepSleep Mode
    LP_EnterDeepSleepMode(ANA, false, LP_RETENTION_SRAM_ALL, LP_DEEPSLEEP_MODE1); // Enter LP_DEEPSLEEP_MODE1 for most power saving

#if CONFIG_UART_LOG_ENABLE
    // Re-enable digital path of Log UART Rx IO so that uart log would output properly then
    SOC_ReenableLogUartRxPath();
#endif

    // Here we release all interrupt ISRs if any
    // Note that this is operation is necessary, as all IRQs have been masked in the DeepSleep Mode entering flow
    __enable_irq();

    // Re-set GPIO back to default analog input state
    NVIC_DisableIRQ(GPIO0_IRQn);
    GPIO_DisableInt(P0, 6);
    GPIO_DisablePullupPath(P0, BIT6);
    GPIO_DisableDigitalPath(P0, BIT6); // Disable digital input path to avoid possible current leakage
}

void LP_DeepSleepApbTimerWakeup(void)
{
    uint32_t slpcnt = 10;

    SYS_TEST("\nConfigure TIMER0 with timeout interupt..\n");

    /*
     * Configure timeout
     * timeout = TIMER_CMP_VAL * (TIMER_PRESCALE + 1) / SYS_CLOCK_APB1
     *         = (32000 / 5) * (0 + 1) / 32000 (s)
     *         = 0.2 s = 200 ms
     * Note that the TIMER_CMP_VAL should not exceed the 24-bit register limit
     */
    const uint32_t TIMER_PRESCALE = 0;
    const uint32_t TIMER_CMP_VAL = 32000 / 5;

    /* Enable HW Timer0 Module clock */
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_TMR0, ENABLE);
    /* Select Timer counting clock source to the low speed 32K clock */
    CLK_SetTmrClkSrc(TIMER0, CLK_APB1_TMR0SEL_RCL32K);
    /* Set Timer to periodic mode */
    TIMER_SetCountingMode(TIMER0, TIMER_PERIODIC_MODE);
    /* Enable Timer interrupt */
    TIMER_EnableInt(TIMER0);
    /* Enable Timer wakeup */
    TIMER_EnableWakeup(TIMER0);
    /* Enable Timer0 interrupt flag0 and wakeup flag0 signal */
    TIMER0->CTL |= BIT8 | BIT11;
    /* Set timeout value */
    TIMER_SetPrescaleValue(TIMER0, TIMER_PRESCALE);
    TIMER_SetCmpValue(TIMER0, TMR0_COMPARATOR_SEL_CMP, TIMER_CMP_VAL);
    /* Start Timer */
    TIMER_Start(TIMER0);

    /* Enable NVIC IRQ for Timer */
    NVIC_EnableIRQ(TMR0_IRQn);

    for (size_t i = 0; i < slpcnt; i++) {
        // Try to enter SoC DeepSleep Mode
        SYS_TEST("Enter deepsleep mode, cnt = %d\n", i);

#if CONFIG_UART_LOG_ENABLE
        // Disable digital path of Log UART Rx IO to avoid possible current leakage before enter DeepSleep mode
        SOC_DisableLogUartRxPath();
#endif

        LP_EnterDeepSleepMode(ANA, false, LP_RETENTION_SRAM_ALL, LP_DEEPSLEEP_MODE2); // Enter LP_DEEPSLEEP_MODE2 for APB Timer wakeup

#if CONFIG_UART_LOG_ENABLE
        // Re-enable digital path of Log UART Rx IO so that uart log would output properly then
        SOC_ReenableLogUartRxPath();
#endif
        // Here we release all interrupt ISRs if any
        // Note that this is operation is necessary, as all IRQs have been masked in the DeepSleep Mode entering flow
        __enable_irq();
        SYS_TEST("Waked up.\n");
    }

    // Re-set Timer and disable Timer0 module clock
    NVIC_DisableIRQ(TMR0_IRQn);
    TIMER_Reset(TIMER0);
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_TMR0, DISABLE);
}

void LP_DeepSleepSlptmrWakeup(void)
{
    SYS_TEST("\nConfigure SleepTimer 1 with timeout interrupt..\n");

    /*
     * Configure timeout:
     * timeout = SLPTMR1_TIMEOUT_CNT / LOW_SPEED_CLK_FREQ
     *         = (32000 / 2) / 32000 (s)
     *         = 0.5 s
     */
    const uint32_t SLPTMR1_TIMEOUT_CNT = 32000 / 2;

    /* Set timeout of SleepTimer 1 */
    LP_SetSleepTime(ANA, SLPTMR1_TIMEOUT_CNT, LP_SLPTMR_CH1);

    /* Enable NVIC IRQ for SleepTimer */
    NVIC_EnableIRQ(SLPTMR_IRQn);

#if CONFIG_UART_LOG_ENABLE
    // Disable digital path of Log UART Rx IO to avoid possible current leakage before enter DeepSleep mode
    SOC_DisableLogUartRxPath();
#endif

    // Try to enter SoC DeepSleep Mode
    LP_EnterDeepSleepMode(ANA, false, LP_RETENTION_SRAM_ALL, LP_DEEPSLEEP_MODE1); // Enter LP_DEEPSLEEP_MODE1 for most power saving

#if CONFIG_UART_LOG_ENABLE
    // Re-enable digital path of Log UART Rx IO so that uart log would output properly then
    SOC_ReenableLogUartRxPath();
#endif

    // Here we release all interrupt ISRs if any
    // Note that this is operation is necessary, as all IRQs have been masked in the DeepSleep Mode entering flow
    __enable_irq();

    // Disable slptmr 1
    NVIC_DisableIRQ(SLPTMR_IRQn);
    LP_SetSleepTime(ANA, 0x0, LP_SLPTMR_CH1);
}

void LP_TestDeepSleepMode(void)
{
    char c;

    while (1) {
        LP_PrintDeepSleepTestInfo();
        /* Force clear UART Rx FIFO before getchar() */
#if CONFIG_LOG_UART_RX_PIN < 4
        UART_ResetRxFifo(UART0);
#else
        UART_ResetRxFifo(UART1);
#endif
        switch (c = getchar()) {
        case 'A':
        case 'a':
            LP_DeepSleepGpioWakeup();
            break;
        case 'B':
        case 'b':
            LP_DeepSleepApbTimerWakeup();
            break;
        case 'C':
        case 'c':
            LP_DeepSleepSlptmrWakeup();
            break;
        case 0x1B:  // Keyboard code <ESC>
            return;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
}
