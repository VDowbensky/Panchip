/**************************************************************************//**
 * @file     gpio_debounce_test.c
 * @version  V1.0
 * $Date:    19/09/03 17:00 $
 * @brief    GPIO test case 4, debounce test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "gpio_common.h"

void SimulateBounceWave(uint8_t Pin)
{
    P(Pin) = 0;
    P(Pin) = 1;
    __nop();
    P(Pin) = 0;
    P(Pin) = 1;
    __nop();__nop();
    P(Pin) = 0;
    P(Pin) = 1;
    __nop();__nop();__nop();
    P(Pin) = 0;
    P(Pin) = 1;
    __nop();__nop();__nop();__nop();
    P(Pin) = 0;
    P(Pin) = 1;
    __nop();__nop();__nop();__nop();__nop();
    P(Pin) = 0;
    P(Pin) = 1;
    __nop();__nop();__nop();__nop();__nop();__nop();
    P(Pin) = 0;
    P(Pin) = 1;
    __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
    __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
    P(Pin) = 0;
    P(Pin) = 1;
    __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
    __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
    __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
    __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
    P(Pin) = 0;
}

T_GPIO_TEST_RESULT GPIO_DebounceTestCase4(uint8_t TargetPin, uint8_t AuxiliaryPin)
{
    SYS_ConfigMFP(TargetPin, SYS_MFP_GPIO);
    SYS_ConfigMFP(AuxiliaryPin, SYS_MFP_GPIO);

    GPIO_SetModeByPin(TargetPin, GPIO_MODE_INPUT);
    GPIO_SetModeByPin(AuxiliaryPin, GPIO_MODE_OUTPUT);

    //CLK_Wait3vSyncReady();

    //Enable debounce of target pin
    SYS_TEST("Debounce Period - 8 Cycles HCLK\n");
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_HCLK, GPIO_DBCTL_DBCLKSEL_8);    //set the debounce period to 8 cycles of HCLK
    GPIO_EnableDebounceByPin(TargetPin);

    //Enable interrupt of target pin
//    GPIO_EnableIntByPin(TargetPin, GPIO_INT_RISING);
    GPIO_EnableIntByPin(TargetPin, GPIO_INT_FALLING);
//    GPIO_EnableIntByPin(TargetPin, GPIO_INT_BOTH_EDGE);
    NVIC_EnableIRQ(GPIO_GetIRQn(TargetPin));

    SYS_delay_10nop(10);

    //Simulate noise (bounce) by auxiliary pin
    SimulateBounceWave(AuxiliaryPin);
    SYS_delay_10nop(10);

    //Change debounce period and test again
    SYS_TEST("Debounce Period - 32 Cycles HCLK\n");
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_HCLK, GPIO_DBCTL_DBCLKSEL_32);    //set the debounce period to 32 cycles of HCLK

    //Simulate noise (bounce) by auxiliary pin again
    SimulateBounceWave(AuxiliaryPin);
    SYS_delay_10nop(10);

    //Test End, disable debounce and interrupt of target pin
    GPIO_DisableDebounceByPin(TargetPin);
    GPIO_DisableIntByPin(TargetPin);
    NVIC_DisableIRQ(GPIO_GetIRQn(TargetPin));

    return GPIO_TST_OK;
}
