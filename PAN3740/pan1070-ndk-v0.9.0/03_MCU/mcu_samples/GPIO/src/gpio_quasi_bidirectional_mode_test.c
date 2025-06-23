/**************************************************************************//**
 * @file     gpio_quasi_bidirectional_mode_test.c
 * @version  V1.0
 * $Date:    19/09/09 19:20 $
 * @brief    GPIO test case 3, quasi-bidirectional input/output test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "gpio_common.h"


T_GPIO_TEST_RESULT GPIO_QuasiBidirectionalModeTestCase3(uint8_t TargetPin, uint8_t AuxiliaryPin)
{
    SYS_ConfigMFP(TargetPin, SYS_MFP_GPIO);
    SYS_ConfigMFP(AuxiliaryPin, SYS_MFP_GPIO);

    GPIO_SetModeByPin(TargetPin, GPIO_MODE_QUASI);  //Quasi-bidirectional mode
    GPIO_SetModeByPin(AuxiliaryPin, GPIO_MODE_INPUT);

#ifdef ENABLE_INTERNAL_PULLUP_RES
    GPIO_EnablePullupPathByPin(TargetPin);      //Enable internal pull-up resistor of target pin
    if (TargetPin == P5_6 || TargetPin == P4_6 || TargetPin == P4_7)
    {
        // These Pins' PUEN bits should be synced to 3v area
        CLK_Wait3vSyncReady();
    }
#endif

    //1. Output Test, target pin pull down
    P(TargetPin) = 0;

    SYS_delay_10nop(10);

    if (P(AuxiliaryPin) != 0)
    {
        return GPIO_TST_TGT_PULL_DOWN_FAIL;
    }

    //2. Output Test, target pin pull up
    P(TargetPin) = 1;

    SYS_delay_10nop(100);

    if (P(AuxiliaryPin) != 1)
    {
        return GPIO_TST_TGT_PULL_UP_FAIL;
    }

    //3. Input Test (Target DOUT should be set to 1)
    GPIO_SetModeByPin(AuxiliaryPin, GPIO_MODE_OUTPUT);   //Set auxiliary pin to output mode
    P(TargetPin) = 1; //Set target DOUT to 1

    //3.1 Pull down auxiliary pin
    P(AuxiliaryPin) = 0;

    SYS_delay_10nop(10);

    if (P(TargetPin) != 0)
    {
        return GPIO_TST_TGT_INPUT_GET_0_FAIL;
    }

    //3.2 Pull up auxiliary pin
    P(AuxiliaryPin) = 1;

    SYS_delay_10nop(100);

    if (P(TargetPin) != 1)
    {
        return GPIO_TST_TGT_INPUT_GET_1_FAIL;
    }

#ifdef ENABLE_INTERNAL_PULLUP_RES
    GPIO_DisablePullupPathByPin(TargetPin);   //Disable internal pull-up resistor of auxiliary pin
    if (TargetPin == P5_6 || TargetPin == P4_6 || TargetPin == P4_7)
    {
        // These Pins' PUEN bits should be synced to 3v area
        CLK_Wait3vSyncReady();
    }
#endif

    return GPIO_TST_OK;
}
