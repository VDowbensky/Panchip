/**************************************************************************//**
 * @file     wwdt_common.c
 * @version  V1.0
 * $Date:    19/11/14 11:17 $
 * @brief    Common source file for WWDT test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <ctype.h>
#include "PanSeries.h"
#include "wwdt_common.h"


T_WWDT_TEST_RESULT (*const WWDT_TestCase[])(void) = 
{
    WWDT_RegisterDefaultValueCheckCase0,
    WWDT_TimeoutWindowTestCase1,
    WWDT_ReloadAndResetTestCase2,
    WWDT_InterruptTestCase3,
};

void WWDT_TestFunctionEnter(uint16_t TcIdx)
{
    T_WWDT_TEST_RESULT r = WWDT_TST_OK;

    if (TcIdx >= sizeof(WWDT_TestCase) / sizeof(void*))
    {
        SYS_TEST("Error, cannot find Testcase %d!", TcIdx);
        return;
    }

    r = (WWDT_TestCase[TcIdx])();
    if (r != WWDT_TST_OK)
    {
        SYS_TEST("WWDT Test Fail, Fail case: %d, Error Code: %d\n", TcIdx, r);
    }
    else
    {
        SYS_TEST("WWDT Test OK, Success case: %d\n", TcIdx);
    }
}


void WWDT_IRQHandler(void)
{
    // Clear Timeout Int Flag
    WWDT_ClearIntFlag();

    // Pulse LA_INPUT_PIN to indicate WWDT Int occurred
    LA_INPUT_PIN = 1;
    SYS_delay_10nop(1);
    LA_INPUT_PIN = 0;
    SYS_TEST("WWDT INT\n");
    // Reload Counter
    WWDT_ReloadCounter();


}

void WWDT_TestModuleInit(void)
{
    // Enable Clock
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_WDT, ENABLE);     //WWDT share the same ClockEnable bit with WDT
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_TMR0, ENABLE);    //Enalbe Timer0 Clock for TIMER_Delay()

    // Config Pinmux of Auxiliary GPIO for Timing Measure by Logic Analyzer
    SYS_SET_MFP(P2, 0, GPIO);
    GPIO_SetMode(P2, BIT0, GPIO_MODE_OUTPUT);
    /* NOTE: We should make usre the GPIO corresponds to the config in wwdt_common.h */
}

uint32_t WWDT_GetPrescaleFromConfig(uint32_t ConfigValue)
{
    switch (ConfigValue)
    {
    case 0:
        return 1;
    case 1:
        return 2;
    case 2:
        return 4;
    case 3:
        return 8;
    case 4:
        return 16;
    case 5:
        return 32;
    case 6:
        return 64;
    case 7:
        return 128;
    case 8:
        return 192;
    case 9:
        return 256;
    case 10:
        return 384;
    case 11:
        return 512;
    case 12:
        return 768;
    case 13:
        return 1024;
    case 14:
        return 1536;
    case 15:
        return 2048;
    default:
        return NULL;
    }
}


T_WWDT_TEST_RESULT WWDT_RegisterDefaultValueCheckCase0(void)
{
    SYS_TEST("WWDT Register Default Values:\n");
    SYS_TEST("---------------------------\n");
    SYS_TEST("RLDCNT        = 0x%08x\n", WWDT->RLDCNT  );
    SYS_TEST("CTL           = 0x%08x\n", WWDT->CTL     );
    SYS_TEST("STATUS        = 0x%08x\n", WWDT->STATUS  );
    SYS_TEST("CNT           = 0x%08x\n", WWDT->CNT     );

    SYS_TEST("\nWWDT Flags: WWDTF=%d, WWDTRF=%d, WWDTIF=%d\n",
            WWDT_GetWWDTFFlag(), WWDT_GetResetFlag(), WWDT_GetIntFlag());
    SYS_TEST("---------------------------\n");

    return WWDT_TST_OK;
}

