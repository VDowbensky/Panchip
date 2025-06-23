/**************************************************************************//**
 * @file     gpio_interrupt_test.c
 * @version  V1.0
 * $Date:    19/09/03 17:00 $
 * @brief    GPIO test case 5, interrupt test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "gpio_common.h"


T_GPIO_TEST_RESULT GPIO_InterruptTestCase5(uint8_t TargetPin, uint8_t AuxiliaryPin)
{
    SYS_ConfigMFP(TargetPin, SYS_MFP_GPIO);
    SYS_ConfigMFP(AuxiliaryPin, SYS_MFP_GPIO);

    GPIO_SetModeByPin(TargetPin, GPIO_MODE_INPUT);
    GPIO_SetModeByPin(AuxiliaryPin, GPIO_MODE_OUTPUT);

    //CLK_Wait3vSyncReady();

    NVIC_EnableIRQ(GPIO_GetIRQn(TargetPin));

    /****** 1.1 Set INT type to GPIO_INT_RISING ******/
    GPIO_EnableIntByPin(TargetPin, GPIO_INT_RISING);

    SYS_delay_10nop(10);

    //Triger interrupt by auxiliary pin
    P(AuxiliaryPin) = 0;
    P(AuxiliaryPin) = 1;

    SYS_delay_10nop(10);

    /****** 1.2 Set INT type to GPIO_INT_FALLING ******/
    GPIO_DisableIntByPin(TargetPin);    //Disable INT before change type
    GPIO_EnableIntByPin(TargetPin, GPIO_INT_FALLING);

    SYS_delay_10nop(10);

    //Triger interrupt by auxiliary pin
    P(AuxiliaryPin) = 1;
    P(AuxiliaryPin) = 0;

    SYS_delay_10nop(10);

    /****** 1.3 Set INT type to GPIO_INT_BOTH_EDGE ******/
    GPIO_DisableIntByPin(TargetPin);    //Disable INT before change type
    GPIO_EnableIntByPin(TargetPin, GPIO_INT_BOTH_EDGE);

    SYS_delay_10nop(10);

    //Triger interrupt by auxiliary pin
    P(AuxiliaryPin) = 0;
    SYS_delay_10nop(1);
    P(AuxiliaryPin) = 1;
    SYS_delay_10nop(1);
    P(AuxiliaryPin) = 0;

    SYS_delay_10nop(10);

    /****** 1.4 Set INT type to GPIO_INT_HIGH ******/
    P(AuxiliaryPin) = 0; //Pull low the auxiliary pin first
    GPIO_DisableIntByPin(TargetPin);    //Disable INT of target pin before change type
    GPIO_EnableIntByPin(TargetPin, GPIO_INT_HIGH);

    SYS_delay_10nop(10);

    //Triger interrupt by auxiliary pin
    P(AuxiliaryPin) = 1; //Try to trigger interrupt
    SYS_delay_10nop(1);

    P(AuxiliaryPin) = 0;
    SYS_delay_10nop(10);

    /****** 1.5 Set INT type to GPIO_INT_LOW ******/
    P(AuxiliaryPin) = 1; //Pull high the auxiliary pin first
    GPIO_DisableIntByPin(TargetPin);    //Disable INT of target pin before change type
    GPIO_EnableIntByPin(TargetPin, GPIO_INT_LOW);

    SYS_delay_10nop(10);

    //Triger interrupt by auxiliary pin
    P(AuxiliaryPin) = 0; //Try to trigger interrupt
    SYS_delay_10nop(1);

    P(AuxiliaryPin) = 1;
    SYS_delay_10nop(10);

    /* Test End, disable interrupt of target pin and NVIC */
    GPIO_DisableIntByPin(TargetPin);
    NVIC_DisableIRQ(GPIO_GetIRQn(TargetPin));

    return GPIO_TST_OK;
}
