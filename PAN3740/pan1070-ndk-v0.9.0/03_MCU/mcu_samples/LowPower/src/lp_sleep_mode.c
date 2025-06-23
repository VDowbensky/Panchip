/*******************************************************************************
 * @note      Copyright (C) 2024 Shanghai Panchip Microelectronics Co., Ltd.
 *            All rights reserved.
 * @file      lp_sleep_mode.c
 * @brief     Low power Sleep Mode test code.
*******************************************************************************/
#include "PanSeries.h"
#include "platform.h"

static void LP_PrintSleepTestInfo(void)
{
    SYS_TEST("\n");
    SYS_TEST("+---------------------------------------------------------+\n");
    SYS_TEST("|    Press key to choose a sleep wakeup source:           |\n");
    SYS_TEST("|                                                         |\n");
    SYS_TEST("|    Input 'A'    Wake up by GPIO interrupt.              |\n");
    SYS_TEST("|    Input 'B'    Wake up by UART interrupt.              |\n");
    SYS_TEST("|    Input 'C'    Wake up by TIMER interrupt.             |\n");
    SYS_TEST("|    Input 'D'    Wake up by SLPTMR interrupt.            |\n");
    SYS_TEST("|    Press ESC key to back to the top level case list.    |\n");
    SYS_TEST("+---------------------------------------------------------+\n");
}

void LP_SleepGpioWakeup(void)
{
    SYS_TEST("\nConfigure GPIO P06 as input mode and enable falling edge interupt detection..\n");

    // Confiure GPIO P06 (EVB KEY1) before enter low power mode
    SYS_SET_MFP(P0, 6, GPIO);
    GPIO_EnablePullupPath(P0, BIT6);
    GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);
    GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);

    // Wait for a while to ensure the internal pullup resistor is stable before entering low power mode
    // This delay is not needed if internal pull-up or pull-down resistor is not in use.
    SYS_delay_10nop(50000);

    // Enable related GPIO IRQ
    NVIC_EnableIRQ(GPIO0_IRQn);

    // Try to enter SoC Sleep Mode
    LP_EnterSleepMode(ANA, false);

    // Re-set GPIO back to default analog input state
    NVIC_DisableIRQ(GPIO0_IRQn);
    GPIO_DisableInt(P0, 6);
    GPIO_DisablePullupPath(P0, BIT6);
    GPIO_DisableDigitalPath(P0, BIT6); // Disable digital input path to avoid possible current leakage
}

void LP_SleepUartRxWakeup(void)
{
    SYS_TEST("\nConfigure UART1 receive data with interrupt..\n");

    /* Enable UART1 Clock */
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);

    /* Configure UART Pinmux */
//    SYS_SET_MFP(P1, 0, UART1_TX);
    SYS_SET_MFP(P0, 7, UART1_RX);
    /* Enable digital input path of UART Rx Pin */
    GPIO_EnableDigitalPath(P0, BIT7);

    /* Init UART1 */
    UART_InitTypeDef Init_Struct = {
        .UART_BaudRate = 115200,
        .UART_LineCtrl = Uart_Line_8n1,
    };
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);

    /* Configure interrupt of UART1 */
    UART_SetRxTrigger(UART1, UART_RX_FIFO_HALF_FULL);
    UART_EnableIrq(UART1, UART_IRQ_RECV_DATA_AVL);      // Enable RDA Interrupt
    NVIC_EnableIRQ(UART1_IRQn);                         // Enable target UART INT in NVIC

    // Try to enter SoC Sleep Mode
    LP_EnterSleepMode(ANA, false);

    // Switch pinmix back to default state and disable UART1 clock
    NVIC_DisableIRQ(UART1_IRQn);
    UART_DisableIrq(UART1, UART_IRQ_RECV_DATA_AVL);
    GPIO_DisableDigitalPath(P0, BIT7); // Disable digital input path to avoid possible current leakage
    SYS_SET_MFP(P0, 7, GPIO);
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, DISABLE);
}

void LP_SleepApbTimerWakeup(void)
{
    uint32_t slpcnt = 10;

    SYS_TEST("\nConfigure TIMER0 with timeout interupt..\n");

    /*
     * Configure timeout
     * timeout = TIMER_CMP_VAL * (TIMER_PRESCALE + 1) / SYS_CLOCK_APB1
     *         = (48000000 / 10) * (0 + 1) / 24000000 (s)
     *         = 0.2 s = 200 ms
     * Note that the TIMER_CMP_VAL should not exceed the 24-bit register limit
     */
    const uint32_t TIMER_PRESCALE = 0;
    const uint32_t TIMER_CMP_VAL = SystemCoreClock / 10;

    /* Enable HW Timer0 Module clock */
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_TMR0, ENABLE);
    /* Select Timer counting clock source to APB clock */
    CLK_SetTmrClkSrc(TIMER0, CLK_APB1_TMR0SEL_APB1CLK);
    /* Set Timer to periodic mode */
    TIMER_SetCountingMode(TIMER0, TIMER_PERIODIC_MODE);
    /* Enable Timer interrupt */
    TIMER_EnableInt(TIMER0);
    /* Enable Timer0 interrupt flag0 signal */
    TIMER0->CTL |= BIT8;
    /* Set timeout value */
    TIMER_SetPrescaleValue(TIMER0, TIMER_PRESCALE);
    TIMER_SetCmpValue(TIMER0, TMR0_COMPARATOR_SEL_CMP, TIMER_CMP_VAL);
    /* Start Timer */
    TIMER_Start(TIMER0);

    /* Enable NVIC IRQ for Timer */
    NVIC_EnableIRQ(TMR0_IRQn);

    for (size_t i = 0; i < slpcnt; i++) {
        // Try to enter SoC Sleep Mode
        SYS_TEST("Enter sleep mode, cnt = %d\n", i);
        LP_EnterSleepMode(ANA, false);
        SYS_TEST("Wakeup, cnt = %d\n", i);
    }

    // Re-set Timer and disable Timer0 module clock
    NVIC_DisableIRQ(TMR0_IRQn);
    TIMER_Reset(TIMER0);
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_TMR0, DISABLE);
}

void LP_SleepSlptmrWakeup(void)
{
    SYS_TEST("\nConfigure SleepTimer 0 with timeout interrupt..\n");

    /*
     * Configure timeout:
     * timeout = SLPTMR0_TIMEOUT_CNT / LOW_SPEED_CLK_FREQ
     *         = (32000 * 2) / 32000 (s)
     *         = 2 s
     */
    const uint32_t SLPTMR0_TIMEOUT_CNT = 32000 * 2;

    /* Set timeout of SleepTimer 0 */
    LP_SetSleepTime(ANA, SLPTMR0_TIMEOUT_CNT, LP_SLPTMR_CH0);

    /* Enable NVIC IRQ for SleepTimer */
    NVIC_EnableIRQ(SLPTMR_IRQn);

    // Try to enter SoC Sleep Mode
    LP_EnterSleepMode(ANA, false);

    // Disable slptmr 0
    NVIC_DisableIRQ(SLPTMR_IRQn);
    LP_SetSleepTime(ANA, 0x0, LP_SLPTMR_CH0);
}

void LP_TestSleepMode(void)
{
    char c;

    while (1) {
        LP_PrintSleepTestInfo();
        /* Force clear UART Rx FIFO before getchar() */
#if CONFIG_LOG_UART_RX_PIN < 4
        UART_ResetRxFifo(UART0);
#else
        UART_ResetRxFifo(UART1);
#endif
        switch (c = getchar()) {
        case 'A':
        case 'a':
            LP_SleepGpioWakeup();
            break;
        case 'B':
        case 'b':
            LP_SleepUartRxWakeup();
            break;
        case 'C':
        case 'c':
            LP_SleepApbTimerWakeup();
            break;
        case 'D':
        case 'd':
            LP_SleepSlptmrWakeup();
            break;
        case 0x1B:  // Keyboard code <ESC>
            return;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
}
