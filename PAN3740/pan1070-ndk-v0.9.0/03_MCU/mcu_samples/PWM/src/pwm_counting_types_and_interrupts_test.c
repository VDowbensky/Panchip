/**************************************************************************//**
 * @file     pwm_counting_types_and_interrupts_test.c
 * @version  V1.0
 * $Date:    19/11/07 14:20 $
 * @brief    PWM test case 2. Test PWM Counting types including Edge-Aligned PWM,
 *           Center-Aligned PWM and Precise Center-Aligned PWM. Test corresponding
 *           interrupts including Zero Point Interrupt (ZI), Compare Up Interrupt
 *           (CMPUI), Period Interrupt (PI) and Compare Down Interrupt (CMPDI).
 *           And lastly test Asymmetric Mode.
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "pwm_common.h"


static void PWM_PrintInfoCase2(void)
{
    SYS_TEST("\n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to test specific function:                         | \n");
    SYS_TEST("|                                                                 | \n");
    SYS_TEST("|    Input 'A'    Edge-Aligned PWM.                               | \n");
    SYS_TEST("|    Input 'B'    Center-Aligned PWM.                             | \n");
    SYS_TEST("|    Input 'C'    Precise Center-Aligned PWM.                     | \n");
    SYS_TEST("|    Input 'D'    Center-Aligned PWM with Asymmetric Mode.        | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.            | \n");
    SYS_TEST("+-----------------------------------------------------------------+ \n");
}


static void PWM_OutputProc(PWM_OperateTypeDef OperateType, bool bAsymmetric)
{
    // Reset Prescaler before Config Output Channel
    PWM_ResetPrescaler(TGT_PWM, TGT_PWM_CH);

    // Config Target Output Channel
    PWM_ConfigOutputChannel(TGT_PWM, TGT_PWM_CH, 2500, 30, OperateType);

    if (bAsymmetric)
    {
        // Enable Asymmetric Mode
        PWM_EnableAsymmetricMode(TGT_PWM);
        // Set CmpDown value as 1/4 CmpUp
        PWM_SetCMRD(TGT_PWM, TGT_PWM_CH, (&TGT_PWM->CMPDAT0)[TGT_PWM_CH] / 4);
        SYS_TEST("\nAsymmetric Enabled, CMPD: %d\n", ((&TGT_PWM->CMPDAT0)[TGT_PWM_CH] & 0xFFFF0000) >> 16);
    }

    // Enable output of TGT_PWM channel
    PWM_EnableOutput(TGT_PWM, BIT(TGT_PWM_CH));

    // Enable interrupts
    PWM_EnableZeroInt(TGT_PWM, TGT_PWM_CH);                                 //Zero Interrupt
    PWM_EnableCMPUInt(TGT_PWM, TGT_PWM_CH);                                 //CoMPare Up Interrupt
    PWM_EnableCenterInt(TGT_PWM, TGT_PWM_CH, PWM_PERIOD_INT_MATCH_CNR);     //Period Interrupt
    PWM_EnableCMPDInt(TGT_PWM, TGT_PWM_CH);                                 //CoMPare Down Interrupt
    NVIC_EnableIRQ(TGT_PWM_IRQ);

    // Start TGT_PWM
    FLAG_PRD_CMP = 0;
    FLAG_ZIF = 0, FLAG_CMPUIF = 0, FLAG_PIF = 0, FLAG_CMPDIF = 0;
    PWM_Start(TGT_PWM, BIT(TGT_PWM_CH));
    SYS_TEST("TGT_PWM Channel%d start...\n", TGT_PWM_CH);
    SYS_TEST("APB CLK Freq: %dHz, CLKPSC: %d, CLKDIV: %d, PERIOD: %d, CMP: %d\n",
        CLK_GetPCLK1Freq(), ((uint8_t*)(&TGT_PWM->CLKPSC))[TGT_PWM_CH/2],
        (TGT_PWM->CLKDIV & (uint32_t)(PWM_CLKDIV_CLKDIV0_Msk << (TGT_PWM_CH*4))) >> (TGT_PWM_CH*4),
        (&TGT_PWM->PERIOD0)[TGT_PWM_CH], (&TGT_PWM->CMPDAT0)[TGT_PWM_CH] & 0x0000FFFF);

    // Force Stop TGT_PWM
    PWM_ForceStop(TGT_PWM, BIT(TGT_PWM_CH));
    FLAG_PRD_CMP = 1;
    FLAG_ZIF = 1, FLAG_CMPUIF = 1, FLAG_PIF = 1, FLAG_CMPDIF = 1;
    SYS_TEST("TGT_PWM stopped.\n");

    // Disable interrupts
    NVIC_DisableIRQ(TGT_PWM_IRQ);
    PWM_DisableZeroInt(TGT_PWM, TGT_PWM_CH);
    PWM_DisableCMPUInt(TGT_PWM, TGT_PWM_CH);
    PWM_DisableCenterInt(TGT_PWM, TGT_PWM_CH);
    PWM_DisableCMPDInt(TGT_PWM, TGT_PWM_CH);

    if (bAsymmetric)
    {
        // Disable Asymmetric Mode
        PWM_DisableAsymmetricMode(TGT_PWM);
    }
}

T_PWM_TEST_RESULT PWM_CountingTypesAndInteruptsTestCase2(void)
{
    char c;

    while(1)
    {
        PWM_PrintInfoCase2();
        switch(c = getchar())
        {
        /* A. Edge-Aligned TGT_PWM */
        case 'A':
        case 'a':
            PWM_OutputProc(OPERATION_EDGE_ALIGNED, false);
            break;
        /* B. Center-Aligned TGT_PWM */
        case 'B':
        case 'b':
            PWM_OutputProc(OPERATION_CENTER_ALIGNED, false);
            break;
        /* C. Precise Center-Aligned TGT_PWM */
        case 'C':
        case 'c':
            PWM_OutputProc(OPERATION_PRECISE_CENTER_ALIGNED, false);
            break;
        /* D. Center-Aligned TGT_PWM with Asymmetric Mode */
        case 'D':
        case 'd':
            PWM_OutputProc(OPERATION_CENTER_ALIGNED, true);
            break;
        case 0x1B:  // Keyboard code <ESC>
            goto OUT;
        default:
            SYS_WRN("Cannot find subtest case %c!\n", c);
            break;
        }
    }
OUT:
    return PWM_TST_OK;
}
