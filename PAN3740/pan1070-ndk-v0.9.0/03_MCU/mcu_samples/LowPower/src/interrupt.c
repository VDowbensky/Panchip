/*******************************************************************************
 * @note      Copyright (C) 2024 Shanghai Panchip Microelectronics Co., Ltd.
 *            All rights reserved.
 * @file      interrupt.c
 * @brief     Implenentation of interrupt handlers.
*******************************************************************************/
#include "PanSeries.h"

void UART0_IRQHandler(void)
{
    while (!UART_IsRxFifoEmpty(UART0)) {
        uint8_t tmp = UART_ReceiveData(UART0);
        SYS_TEST("UART0 INT Rx: 0x%02x.\n", tmp);
    }
}

void UART1_IRQHandler(void)
{
    while (!UART_IsRxFifoEmpty(UART1)) {
        uint8_t tmp = UART_ReceiveData(UART1);
        SYS_TEST("UART1 INT Rx: 0x%02x.\n", tmp);
    }
}

void GPIO0_IRQHandler(void)
{
    for (size_t i = 0; i < 8; i++) {
        if (GPIO_GetIntFlag(P0, BIT(i))) {
            GPIO_ClrIntFlag(P0, BIT(i));
            SYS_TEST("P0_%d INT occurred.\n", i);
        }
    }
}

void GPIO1_IRQHandler(void)
{
    for (size_t i = 0; i < 8; i++) {
        if (GPIO_GetIntFlag(P1, BIT(i))) {
            GPIO_ClrIntFlag(P1, BIT(i));
            SYS_TEST("P1_%d INT occurred.\n", i);
        }
    }
}

void GPIO2_IRQHandler(void)
{
    for (size_t i = 0; i < 8; i++) {
        if (GPIO_GetIntFlag(P2, BIT(i))) {
            GPIO_ClrIntFlag(P2, BIT(i));
            SYS_TEST("P2_%d INT occurred.\n", i);
        }
    }
}

void GPIO3_IRQHandler(void)
{
    for (size_t i = 0; i < 8; i++) {
        if (GPIO_GetIntFlag(P3, BIT(i))) {
            GPIO_ClrIntFlag(P3, BIT(i));
            SYS_TEST("P3_%d INT occurred.\n", i);
        }
    }
}

void TMR0_IRQHandler(void)
{
    /* Handle timer interrupt event */
    if (TIMER_GetIntFlag(TIMER0)) {
        /* Clear timer int flags */
        TIMER_ClearTFFlag(TIMER0, TIMER_GetTFFlag(TIMER0));
        TIMER_ClearIntFlag(TIMER0);
        SYS_TEST("TIMER0 INT occurred.\n");
    }

    /* Clear wakeup flag if there is. */
    TIMER_ClearWakeupFlag(TIMER0, TIMER_GetWakeupFlag(TIMER0));
}

void TMR1_IRQHandler(void)
{
    /* Handle timer interrupt event */
    if (TIMER_GetIntFlag(TIMER1)) {
        /* Clear timer int flags */
        TIMER_ClearTFFlag(TIMER1, TIMER_GetTFFlag(TIMER1));
        TIMER_ClearIntFlag(TIMER1);
        SYS_TEST("TIMER1 INT occurred.\n");
    }

    /* Clear wakeup flag if there is. */
    TIMER_ClearWakeupFlag(TIMER1, TIMER_GetWakeupFlag(TIMER1));
}

void SLPTMR_IRQHandler(void)
{
    /* Handle os clock timeout */
    if (ANA->LP_INT_CTRL & ANAC_INT_SLEEP_TMR0_Msk) {
        /*
         * Clear sleep timer 0 interrupt flags (write 1 to clear) in this register,
         * and retain other settings / flags.
         */
        ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_SLEEP_TMR0_Msk)
            & ~(ANAC_INT_SLEEP_TMR1_Msk | ANAC_INT_SLEEP_TMR2_Msk
            | ANAC_INT_DP_FLAG_Msk | ANAC_INT_STANDBY_M1_FLAG_Msk
            | ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);
        SYS_TEST("SleepTimer0 INT occurred.\n");
    }

    /* Handle custom sleep timer1 event */
    if (ANA->LP_INT_CTRL & ANAC_INT_SLEEP_TMR1_Msk) {
        /*
         * Clear sleep timer 1 interrupt flags (write 1 to clear) in this register,
         * and retain other settings / flags.
         */
        ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_SLEEP_TMR1_Msk)
            & ~(ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_SLEEP_TMR2_Msk
            | ANAC_INT_DP_FLAG_Msk | ANAC_INT_STANDBY_M1_FLAG_Msk
            | ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);
        SYS_TEST("SleepTimer1 INT occurred.\n");
    }

    /* Handle custom sleep timer2 event */
    if (ANA->LP_INT_CTRL & ANAC_INT_SLEEP_TMR2_Msk) {
        /*
         * Clear sleep timer 2 interrupt flags (write 1 to clear) in this register,
         * and retain other settings / flags.
         */
        ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_SLEEP_TMR2_Msk)
            & ~(ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_SLEEP_TMR1_Msk
            | ANAC_INT_DP_FLAG_Msk | ANAC_INT_STANDBY_M1_FLAG_Msk
            | ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);
        SYS_TEST("SleepTimer2 INT occurred.\n");
    }
}

/*
 * NOTE: LP IRQ Handler is essential for waking up from DeepSleep Mode!
 */
void LP_IRQHandler(void)
{
    /*
     * Clear DeepSleep int flag (write 1 to clear) in this register, but still
     * retain all other ctrl/status flags.
     */
    ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_DP_FLAG_Msk)
        & ~(ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_SLEEP_TMR1_Msk | ANAC_INT_SLEEP_TMR2_Msk
        | ANAC_INT_STANDBY_M1_FLAG_Msk | ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);

    /* Re-disable LP IRQ after use */
    NVIC_DisableIRQ(LP_IRQn);
}
