/**************************************************************************//**
 * @file     wwdt_timeout_window_test.c
 * @version  V1.0
 * $Date:    19/11/14 13:23 $
 * @brief    WWDT test case 1, test different Timeout Window Configurations
 *           with Prescaler(PSCSEL) and CompareValue(CMPDAT) changing, and
 *           test WWDT in different Clock Sources(APB/LIRC).
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "wwdt_common.h"


static void WWDT_PrintInfoCase1(void)
{
    SYS_TEST("\n");
    SYS_TEST("+---------------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                             | \n");
    SYS_TEST("|                                                                     | \n");
    SYS_TEST("|    Input 'A'    Test Timeout Window and set APB  as WWDT_CLK.       | \n");
    SYS_TEST("|    Input 'B'    Test Timeout Window and set LIRC as WWDT_CLK.       | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.                | \n");
    SYS_TEST("+---------------------------------------------------------------------+ \n");
}

static uint32_t GetInputNumberFromDebugPort(void)
{
    char in_str[10];
    size_t i = 0;

    while((in_str[i] = getchar()) != '\n')
    {
        if (i >= sizeof(in_str) - 1)    //Truncated if input string is too long
        {
            break;
        }
        i++;
    }
    in_str[i] = '\0';

    return atoi(in_str);
}

static void WWDT_TimeoutWindowProc(uint32_t u32ClkSel, WWDT_PrescaleDef Prescaler, uint32_t u32CmpValue)
{
    // Unlock Regs
    SYS_UnlockReg();

    // Select Clock Source
    CLK_SetWwdtClkSrc(u32ClkSel);

    // Delay some nops before Re-lock Regs
    SYS_delay_10nop(1);
    // Re-Lock Regs
    SYS_LockReg();

    if (u32ClkSel == WDT_CLK_SRC_SEL_APBDIV)
        SYS_TEST("Start WWDT Counting (Clock Source: APB1, Freq = %dHz)...\n", CLK_GetPCLK1Freq());
    else if (u32ClkSel == WDT_CLK_SRC_SEL_RCL)
        SYS_TEST("Start WWDT Counting (Clock Source: 32KHz LIRC)...\n");

    // Enable WWDT, disable Int
    WWDT_Open(Prescaler, u32CmpValue, FALSE);

    // Pull down LA_INPUT_PIN to indicate WWDT counting start
    LA_INPUT_PIN = 0;

    // Wait for Compare Match Event occur
    while(!WWDT_GetWWDTFFlag());
    // Clear Compare Match Flag
    WWDT_ClearWWDTFFlag();

    // Pull up LA_INPUT_PIN to indicate WWDT Compare Match Event occurred
    LA_INPUT_PIN = 1;

    SYS_TEST("\nTimeout Window reached, wait for reset...\n");

#if 0   //WWDT is not able to disabled once enabled
    // Disable WWDT
    WWDT_Close();
#endif

    while(1);
}

T_WWDT_TEST_RESULT WWDT_TimeoutWindowTestCase1(void)
{
    char c;
    while(1)
    {
        WWDT_PrintInfoCase1();
        switch(c = getchar())
        {
        /* A. Test Timeout Window and set APB as WWDT_CLK */
        case 'A':
        case 'a':
            {
                uint32_t pscsel, cmpdat;
                SYS_TEST("\nPlease input PSCSEL (Range 0 ~ 15, Press <Enter> to confirm):\n");
                while(1)
                {
                    pscsel = GetInputNumberFromDebugPort();
                    if (pscsel > 15)
                        SYS_TEST("Error, please input a valid PSCSEL (0 ~ 15):\n");
                    else
                        break;
                }
                SYS_TEST("Please input Compare Value (Range 0 ~ 63, Press <Enter> to confirm):\n");
                while(1)
                {
                    cmpdat = GetInputNumberFromDebugPort();
                    if (cmpdat > 63)
                        SYS_TEST("Error, please input a valid Compare Value (0 ~ 63):\n");
                    else
                        break;
                }
                SYS_TEST("Timeout Window setting done, Prescaler: %d (PSCSEL = %d), Compare Value: %d\n\n",
                        WWDT_GetPrescaleFromConfig(pscsel), pscsel, cmpdat);

                WWDT_TimeoutWindowProc(WDT_CLK_SRC_SEL_APBDIV, (WWDT_PrescaleDef)(pscsel << WWDT_CTL_PSCSEL_Pos), cmpdat);
            }
            break;
        /* B. Test Timeout Window and set LIRC as WWDT_CLK */
        case 'B':
        case 'b':
            {
                uint32_t pscsel, cmpdat;
                SYS_TEST("\nPlease input PSCSEL (Range 0 ~ 15, Press <Enter> to confirm):\n");
                while(1)
                {
                    pscsel = GetInputNumberFromDebugPort();
                    if (pscsel > 15)
                        SYS_TEST("Error, please input a valid PSCSEL (0 ~ 15):\n");
                    else
                        break;
                }
                SYS_TEST("Please input Compare Value (Range 0 ~ 63, Press <Enter> to confirm):\n");
                while(1)
                {
                    cmpdat = GetInputNumberFromDebugPort();
                    if (cmpdat > 63)
                        SYS_TEST("Error, please input a valid Compare Value (0 ~ 63):\n");
                    else
                        break;
                }
                SYS_TEST("Timeout Window setting done, Prescaler: %d (PSCSEL = %d), Compare Value: %d\n\n",
                        WWDT_GetPrescaleFromConfig(pscsel), pscsel, cmpdat);

                WWDT_TimeoutWindowProc(WDT_CLK_SRC_SEL_RCL, (WWDT_PrescaleDef)(pscsel << WWDT_CTL_PSCSEL_Pos), cmpdat);
            }
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
