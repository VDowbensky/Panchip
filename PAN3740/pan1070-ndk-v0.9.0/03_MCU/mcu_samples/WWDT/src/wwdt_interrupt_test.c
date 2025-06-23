/**************************************************************************//**
 * @file     wwdt_interrupt_test.c
 * @version  V1.0
 * $Date:    19/11/14 19:54 $
 * @brief    WWDT test case 3, test WWDT Interrupt.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "wwdt_common.h"


static void WWDT_PrintInfoCase3(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                     | \n");
    SYS_TEST("|                                                             | \n");
    SYS_TEST("|    Input 'A'    Enable WWDT Interrupt.                      | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.        | \n");
    SYS_TEST("+-------------------------------------------------------------+ \n");
}

static void WWDT_InterruptProc(void)
{

    WWDT_PrescaleDef Prescaler = WWDT_PRESCALER_32;

    uint32_t u32CmpValue = 25;

    SYS_TEST("\nStart WDT Counting (APB1_CLK = %dHz)...\n", CLK_GetPCLK1Freq());
    SYS_TEST("Prescaler: %d (PSCSEL = %d), Compare Value: %d\n",
        WWDT_GetPrescaleFromConfig(Prescaler >> WWDT_CTL_PSCSEL_Pos),
        Prescaler >> WWDT_CTL_PSCSEL_Pos, u32CmpValue);

    // Enable WWDT, enable Int
    WWDT_Open(Prescaler, u32CmpValue, TRUE);

    // Enable NVIC WWDT IRQ
    NVIC_EnableIRQ(WWDT_IRQn);

    // Pull down LA_INPUT_PIN to indicate WWDT counting start
    LA_INPUT_PIN = 0;

    // Delay 400ms
    TIMER_Delay(TIMER0, 300000);


    // Pull up LA_INPUT_PIN to indicate WWDT INT is going to disabled
    LA_INPUT_PIN = 1;

    // Disable NVIC WWDT IRQ, thus WWDT will stop reloading counter
    NVIC_DisableIRQ(WWDT_IRQn);

    // Wait for reset
    SYS_TEST("Wait For Reset...\n");
    while(1);
}

T_WWDT_TEST_RESULT WWDT_InterruptTestCase3(void)
{
    char c;
    while(1)
    {
        WWDT_PrintInfoCase3();
        switch(c = getchar())
        {
        /* A. Enable WWDT Interrupt */
        case 'A':
        case 'a':
            WWDT_InterruptProc();
            break;
        case 0x1B:  // Keyboard code <ESC>
            goto OUT;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
OUT:
    return WWDT_TST_OK;
}
