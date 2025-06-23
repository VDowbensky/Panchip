/*******************************************************************************
 * @note      Copyright (C) 2024 Shanghai Panchip Microelectronics Co., Ltd.
 *            All rights reserved.
 * @file      main.c
 * @brief     PAN10xx Low Power Mode Demo
*******************************************************************************/
#include "PanSeries.h"
#include "platform.h"
#include "main.h"

void LP_PrintSampleInfo(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+\n");
    SYS_TEST("|                   LowPower Sample Code.                         |\n");
    SYS_TEST("+-----------------------------------------------------------------+\n");
    SYS_TEST("|    Press specific key to start test..                           |\n");
    SYS_TEST("|    Input'1'   Enter and Wakeup from SoC Sleep Mode.             |\n");
    SYS_TEST("|    Input'2'   Enter and Wakeup from SoC DeepSleep Mode.         |\n");
    SYS_TEST("|    Input'3'   Enter and Wakeup from SoC Standby Mode 1.         |\n");
    SYS_TEST("|    Input'4'   Enter and Wakeup from SoC Standby Mode 0.         |\n");
    SYS_TEST("+-----------------------------------------------------------------+\n");
}

static void LP_CheckAndClearGpioIntStatus(void)
{
#if 0
    SYS_TEST("P0->INTSRC=0x%08x.\n", P0->INTSRC);
    SYS_TEST("P1->INTSRC=0x%08x.\n", P1->INTSRC);
    SYS_TEST("P2->INTSRC=0x%08x.\n", P2->INTSRC);
    SYS_TEST("P3->INTSRC=0x%08x.\n", P3->INTSRC);
#endif

    for (size_t i = 0; i < 8; i++) {
        if (GPIO_GetIntFlag(P0, BIT(i))) {
            GPIO_DisableInt(P0, i);
            GPIO_DisableDigitalPath(P0, BIT(i));
            GPIO_DisablePullupPath(P0, BIT(i)); // Disable internal pullup resistor if any
            SYS_TEST("SoC is waked up by GPIO P0_%d.\n", i);
        }
    }
    GPIO_ClrAllIntFlag(P0);

    for (size_t i = 0; i < 8; i++) {
        if (GPIO_GetIntFlag(P1, BIT(i))) {
            GPIO_DisableInt(P1, i);
            GPIO_DisableDigitalPath(P1, BIT(i));
            GPIO_DisablePullupPath(P1, BIT(i)); // Disable internal pullup resistor if any
            SYS_TEST("SoC is waked up by GPIO P1_%d.\n", i);
        }
    }
    GPIO_ClrAllIntFlag(P1);

    for (size_t i = 0; i < 8; i++) {
        if (GPIO_GetIntFlag(P2, BIT(i))) {
            GPIO_DisableInt(P2, i);
            GPIO_DisableDigitalPath(P2, BIT(i));
            GPIO_DisablePullupPath(P2, BIT(i)); // Disable internal pullup resistor if any
            SYS_TEST("SoC is waked up by GPIO P2_%d.\n", i);
        }
    }
    GPIO_ClrAllIntFlag(P2);

    for (size_t i = 0; i < 8; i++) {
        if (GPIO_GetIntFlag(P3, BIT(i))) {
            GPIO_DisableInt(P3, i);
            GPIO_DisableDigitalPath(P3, BIT(i));
            GPIO_DisablePullupPath(P3, BIT(i)); // Disable internal pullup resistor if any
            SYS_TEST("SoC is waked up by GPIO P3_%d.\n", i);
        }
    }
    GPIO_ClrAllIntFlag(P3);

    // Clear GPIO pending IRQs if any
    NVIC_ClearPendingIRQ(GPIO0_IRQn);
    NVIC_ClearPendingIRQ(GPIO1_IRQn);
    NVIC_ClearPendingIRQ(GPIO2_IRQn);
    NVIC_ClearPendingIRQ(GPIO3_IRQn);
}

int main(void)
{
    char c;
    uint8_t rst_reason;

#if BOOT_WITH_LONG_DELAY
    /* Set a long delay for debugging use */
    SYS_delay_10nop(1000000);
    SYS_delay_10nop(1000000);
    SYS_delay_10nop(1000000);
    SYS_delay_10nop(1000000);
#endif

    SOC_PlatformInit();

    SYS_TEST("CPU @ %dHz\n", SystemCoreClock);

    /* Get the last reset reason */
    SYS_TEST("\nReset Reason: ");
    rst_reason = SOC_ResetReasonGet();
    switch (rst_reason) {
    case SOC_RST_REASON_POR_RESET:
        SYS_TEST("Power On Reset.\n");
        break;
    case SOC_RST_REASON_PIN_RESET:
        SYS_TEST("nRESET Pin Reset.\n");
        break;
    case SOC_RST_REASON_SYS_RESET:
        SYS_TEST("NVIC System Reset.\n");
        break;
    case SOC_RST_REASON_CHIP_RESET:
        SYS_TEST("SoC Software Chip Reset.\n");
        break;
    case SOC_RST_REASON_STBM1_GPIO_WAKEUP:
        SYS_TEST("Standby Mode 1 GPIO Wakeup.\n");
        LP_CheckAndClearGpioIntStatus();
        break;
    case SOC_RST_REASON_STBM1_SLPTMR0_WAKEUP:
        SYS_TEST("Standby Mode 1 Slptmr0 Wakeup.\n");
        // Disable slptmr 0 to avoid unexpected timeout irq
        LP_SetSleepTime(ANA, 0x0, LP_SLPTMR_CH0);
        break;
    case SOC_RST_REASON_STBM1_SLPTMR1_WAKEUP:
        SYS_TEST("Standby Mode 1 Slptmr1 Wakeup.\n");
        // Disable slptmr 1 to avoid unexpected timeout irq
        LP_SetSleepTime(ANA, 0x0, LP_SLPTMR_CH1);
        break;
    case SOC_RST_REASON_STBM1_SLPTMR2_WAKEUP:
        SYS_TEST("Standby Mode 1 Slptmr2 Wakeup.\n");
        // Disable slptmr 2 to avoid unexpected timeout irq
        LP_SetSleepTime(ANA, 0x0, LP_SLPTMR_CH2);
        break;
    case SOC_RST_REASON_STBM0_EXTIO_WAKEUP:
        SYS_TEST("Standby Mode 0 Special IO (P00/P01/P02) Wakeup.\n");
        break;
    default:
        SYS_TEST("Unhandled Reset Reason (%d), refer to more reason define in platform.h!\n", rst_reason);
    }

    while (1) {
        LP_PrintSampleInfo();
        /* Force clear UART Rx FIFO before getchar() */
#if CONFIG_LOG_UART_RX_PIN < 4
        UART_ResetRxFifo(UART0);
#else
        UART_ResetRxFifo(UART1);
#endif
        switch (c = getchar()) {
            case '1':
                LP_TestSleepMode();
                break;
            case '2':
                LP_TestDeepSleepMode();
                break;
            case '3':
                LP_TestStandbyMode1();
                break;
            case '4':
                LP_TestStandbyMode0();
                break;
            default:
                SYS_WRN("Cannot find testcase %c!\n", c);
                break;
        }
    }
}
