/**************************************************************************//**
 * @file     pwm_common.c
 * @version  V1.0
 * $Date:    19/11/06 17:10 $
 * @brief    Common source file for PWM test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <ctype.h>
#include "PanSeries.h"
#include "pwm_common.h"


T_PWM_TEST_RESULT (*const PWM_TestCase[])(void) = 
{
    PWM_RegisterDefaultValueCheckCase0,
    PWM_CountingModesTestCase1,
    PWM_CountingTypesAndInteruptsTestCase2,
    PWM_OperationModesTestCase3,
    PWM_PolarityControlTestCase4,
    PWM_AdcTriggerTestCase5,
};

void PWM_TestFunctionEnter(uint16_t TcIdx)
{
    T_PWM_TEST_RESULT r = PWM_TST_OK;

    if (TcIdx >= sizeof(PWM_TestCase) / sizeof(void*))
    {
        SYS_TEST("Error, cannot find Testcase %d!", TcIdx);
        return;
    }

    r = (PWM_TestCase[TcIdx])();
    if (r != PWM_TST_OK)
    {
        SYS_TEST("PWM Test Fail, Fail case: %d, Error Code: %d\n", TcIdx, r);
    }
    else
    {
        SYS_TEST("PWM Test OK, Success case: %d\n", TcIdx);
    }
}

void PWM_IrqProcess(PWM_T *pwm)
{
    // Zero Point Interrupt
    if(PWM_GetZeroIntFlag(pwm, TGT_PWM_CH))
    {
        FLAG_ZIF = 1;
        PWM_ClearZeroIntFlag(pwm, TGT_PWM_CH);
        SYS_delay_10nop(1);
        FLAG_ZIF = 0;
    }
    // Compare Up Interrupt
    if(PWM_GetCMPUIntFlag(pwm, TGT_PWM_CH))
    {
        FLAG_CMPUIF = 1;
        PWM_ClearCMPUIntFlag(pwm, TGT_PWM_CH);
        SYS_delay_10nop(1);
        FLAG_CMPUIF = 0;
    }
    // Period Interrupt
    if(PWM_GetPeriodIntFlag(pwm, TGT_PWM_CH))
    {
        FLAG_PIF = 1;
        PWM_ClearPeriodIntFlag(pwm, TGT_PWM_CH);
        SYS_delay_10nop(1);
        FLAG_PIF = 0;
    }
    // Compare Down Interrupt
    if(PWM_GetCMPDIntFlag(pwm, TGT_PWM_CH))
    {
        FLAG_CMPDIF = 1;
        PWM_ClearCMPDIntFlag(pwm, TGT_PWM_CH);
        SYS_delay_10nop(1);
        FLAG_CMPDIF = 0;
    }
}

void PWM_IRQHandler(void)
{
    PWM_IrqProcess(PWM);
}



void PWM_TestModuleInit(void)
{
    // Enable Clock
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_PWM0_CH01, ENABLE);
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_PWM0_CH23, ENABLE);
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_PWM0_CH45, ENABLE);
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_PWM0_CH67, ENABLE);

    // Config Pinmux
    SYS_SET_MFP(P2, 2, PWM_CH0);
    SYS_SET_MFP(P2, 3, PWM_CH1);
    SYS_SET_MFP(P2, 4, PWM_CH2);
    SYS_SET_MFP(P2, 5, PWM_CH3);
    SYS_SET_MFP(P2, 6, PWM_CH4);
    SYS_SET_MFP(P2, 7, PWM_CH5);
    SYS_SET_MFP(P3, 0, PWM_CH6);
    SYS_SET_MFP(P3, 1, PWM_CH7);

    GPIO_SetMode(P1, BIT0, GPIO_MODE_OUTPUT);   //To indicate PERIOD/CMPDAT value changed
    GPIO_SetMode(P1, BIT2, GPIO_MODE_OUTPUT);   //To indicate ZIF signal
    GPIO_SetMode(P1, BIT3, GPIO_MODE_OUTPUT);   //To indicate CMPUIF signal
    GPIO_SetMode(P1, BIT4, GPIO_MODE_OUTPUT);   //To indicate PIF signal
    GPIO_SetMode(P1, BIT5, GPIO_MODE_OUTPUT);   //To indicate CMPDIF signal
}

T_PWM_TEST_RESULT PWM_RegisterDefaultValueCheckCase0(void)
{
    uint8_t j,different = 0;
    while(j<24){
        if(((__IO uint32_t *)(&TGT_PWM->CLKPSC))[j] != 0){
            SYS_TEST("addr offset:%x,respect:0,current:%08x \r\n",j*4,((__IO uint32_t *)(&TGT_PWM->CLKPSC))[j]);
            different++;
        }
        j++;
    }
    if(TGT_PWM->DTCTL    != 0) different++;
    if(TGT_PWM->ADCTCTL0 != 0) different++;  
    if(TGT_PWM->ADCTCTL1 != 0) different++;
    if(TGT_PWM->ADCTSTS0 != 0) different++;
    if(TGT_PWM->ADCTSTS1 != 0) different++;
    if(TGT_PWM->PCACTL   != 0) different++;

    if(!different){
       SYS_TEST("pwm default value check ok \r\n"); 
    }

    return PWM_TST_OK;
}

