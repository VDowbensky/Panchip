/**************************************************************************//**
 * @file     wwdt_reload_and_reset_test.c
 * @version  V1.0
 * $Date:    19/11/14 15:31 $
 * @brief    WWDT test case 2, test WWDT Reload/Reset function.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "wwdt_common.h"


static void WWDT_PrintInfoCase2(void)
{
    SYS_TEST("\n");
    SYS_TEST("+---------------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                             | \n");
    SYS_TEST("|                                                                     | \n");
    SYS_TEST("|    Input 'A'    Reload Counter before CNTDAT is reduced to CMPDAT.  | \n");
    SYS_TEST("|    Input 'B'    Reload Counter when   CNTDAT is equal to   CMPDAT.  | \n");
    SYS_TEST("|    Input 'C'    Reload Counter while  CNTDAT is less than  CMPDAT.  | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.                | \n");
    SYS_TEST("+---------------------------------------------------------------------+ \n");
}


static void WWDT_ReloadCauseResetProc(void)
{
    WWDT_PrescaleDef Prescaler = WWDT_PRESCALER_32;
    uint32_t u32CmpValue = 25;

    // Pull down LA_INPUT_PIN
    LA_INPUT_PIN = 0;

    SYS_TEST("\nStart WDT Counting (APB1_CLK = %dHz)...\n", CLK_GetPCLK1Freq());
    SYS_TEST("Prescaler: %d (PSCSEL = %d), Compare Value: %d\n",
        WWDT_GetPrescaleFromConfig(Prescaler >> WWDT_CTL_PSCSEL_Pos),
        Prescaler >> WWDT_CTL_PSCSEL_Pos, u32CmpValue);

    // Enable WWDT, disable Int
    WWDT_Open(Prescaler, u32CmpValue, FALSE);

    // Pull up LA_INPUT_PIN to indicate WWDT counting start
    LA_INPUT_PIN = 1;

    // Delay 150ms
    TIMER_Delay(TIMER0, 25000);

    // Pull down LA_INPUT_PIN to indicate delay finished and is going to check CNTDAT and CMPDAT
    LA_INPUT_PIN = 0;

    // Check if CNTDAT is larger than CMPDAT
    if (WWDT_GetCounter() > WWDT_GetCompareValue())
    {
        SYS_TEST("\nWWDT Counter Value = %d, and is larger than Compare Value(%d)\n",
            WWDT_GetCounter(), WWDT_GetCompareValue());
        SYS_TEST("About to Reload Counter...\n");
//        SYS_delay_10nop(100000);

        //Pull up LA_INPUT_PIN to indicate WWDT is prepared to reload counter
        LA_INPUT_PIN = 1;

        // Reload Counter
        WWDT_ReloadCounter();

        while(1)
        {
            SYS_TEST("Wait For Reset...\n");
        }
    }

    while(1);
}

static void WWDT_ReloadToFeedProc1(void)
{
    WWDT_PrescaleDef Prescaler = WWDT_PRESCALER_32;
    uint32_t u32CmpValue = 25;

    // Pull down LA_INPUT_PIN
    LA_INPUT_PIN = 0;

    SYS_TEST("\nStart WDT Counting (APB1_CLK = %dHz)...\n", CLK_GetPCLK1Freq());
    SYS_TEST("Prescaler: %d (PSCSEL = %d), Compare Value: %d\n\n",
        WWDT_GetPrescaleFromConfig(Prescaler >> WWDT_CTL_PSCSEL_Pos),
        Prescaler >> WWDT_CTL_PSCSEL_Pos, u32CmpValue);

    // Enable WWDT, disable Int
    WWDT_Open(Prescaler, u32CmpValue, FALSE);

    // Pull up LA_INPUT_PIN to indicate WWDT counting start
    LA_INPUT_PIN = 1;

    size_t rld_cnt = 5;
    while(rld_cnt--)
    {
        // Wait until CNTDAT is reduced to CMPDAT (WWDTF Flag is set by hardware)
        while(!WWDT_GetWWDTFFlag());
        WWDT_ClearWWDTFFlag();

        // Pull down LA_INPUT_PIN to indicate CNTDAT is reduced to CMPDAT
        LA_INPUT_PIN = 0;

        // Check if CNTDAT is equal to CMPDAT
        if (WWDT_GetCounter() <= WWDT_GetCompareValue())
        {
            // Reload Counter
            WWDT_ReloadCounter();

            //Pull up LA_INPUT_PIN to indicate WWDT counter is reload
            LA_INPUT_PIN = 1;

					#ifndef FPGA_MODE
            SYS_TEST("WWDT Counter Value = %d (Equal to Compare Value), Reload Counter...\n",
                WWDT_GetCounter());
					#endif
        }
        else
        {
            SYS_TEST("Error, CNTDAT is not equal to CMPDAT!\n");
        }
    }

    SYS_TEST("\nWait For Reset...\n");
    while(1);
}

static void WWDT_ReloadToFeedProc2(void)
{
    WWDT_PrescaleDef Prescaler = WWDT_PRESCALER_32;
    uint32_t u32CmpValue = 25;

    // Pull down LA_INPUT_PIN
    LA_INPUT_PIN = 0;

    SYS_TEST("\nStart WDT Counting (APB1_CLK = %dHz)...\n", CLK_GetPCLK1Freq());
    SYS_TEST("Prescaler: %d (PSCSEL = %d), Compare Value: %d\n",
        WWDT_GetPrescaleFromConfig(Prescaler >> WWDT_CTL_PSCSEL_Pos),
        Prescaler >> WWDT_CTL_PSCSEL_Pos, u32CmpValue);

    // Enable WWDT, disable Int
    WWDT_Open(Prescaler, u32CmpValue, FALSE);

    // Pull up LA_INPUT_PIN to indicate WWDT counting start
    LA_INPUT_PIN = 1;

    size_t rld_cnt = 5;
    while(rld_cnt--)
    {
        // Wait until CNTDAT is reduced to CMPDAT (WWDTF Flag is set by hardware)
        while(!WWDT_GetWWDTFFlag());
        WWDT_ClearWWDTFFlag();

			#ifndef FPGA_MODE
        // Delay 12ms
        TIMER_Delay(TIMER0, 12000);
			#endif

        // Reload Counter
        WWDT_ReloadCounter();

        //Pulse LA_INPUT_PIN to indicate WWDT counter is reload
        LA_INPUT_PIN = 0;
        SYS_delay_10nop(1);
        LA_INPUT_PIN = 1;

			#ifndef FPGA_MODE
        SYS_TEST("\nWWDT Counter Value = %d, and is less than Compare Value(%d)\n",
            WWDT_GetCounter(), WWDT_GetCompareValue());
			#endif
    }

    SYS_TEST("\nWait For Reset...\n");
    while(1);
}

T_WWDT_TEST_RESULT WWDT_ReloadAndResetTestCase2(void)
{
    char c;
    while(1)
    {
        WWDT_PrintInfoCase2();
        switch(c = getchar())
        {
        /* A. Reload Counter before CNTDAT is reduced to CMPDAT */
        case 'A':
        case 'a':
            WWDT_ReloadCauseResetProc();
            break;
        /* B. Reload Counter when CNTDAT is equal to CMPDAT */
        case 'B':
        case 'b':
            WWDT_ReloadToFeedProc1();
            break;
        /* C. Reload Counter while CNTDAT is less than CMPDAT */
        case 'C':
        case 'c':
            WWDT_ReloadToFeedProc2();
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
