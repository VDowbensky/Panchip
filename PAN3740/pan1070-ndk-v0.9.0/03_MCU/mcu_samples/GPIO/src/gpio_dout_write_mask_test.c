/**************************************************************************//**
 * @file     gpio_dout_write_mask_test.c
 * @version  V1.0
 * $Date:    19/09/03 17:00 $
 * @brief    GPIO test case 7, DOUT write mask test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "gpio_common.h"


T_GPIO_TEST_RESULT GPIO_DoutWriteMaskTestCase7(uint8_t TargetPin, uint8_t AuxiliaryPin)
{
    SYS_ConfigMFP(TargetPin, SYS_MFP_GPIO);
    SYS_ConfigMFP(AuxiliaryPin, SYS_MFP_GPIO);

    GPIO_SetModeByPin(TargetPin, GPIO_MODE_OUTPUT); //Push-pull mode
    GPIO_SetModeByPin(AuxiliaryPin, GPIO_MODE_INPUT);

    //CLK_Wait3vSyncReady();

    /* Test1. Mask and Un-mask before pull up target pin */
    P(TargetPin) = 0; //Target pin pull down as initial state
    SYS_delay_10nop(10);

    if (P(AuxiliaryPin) != 0)
    {
        return GPIO_TST_TGT_PULL_DOWN_FAIL;
    }

    //Enable Dout mask of target pin
    GPIO_EnableDoutMaskByPin(TargetPin);

    //Pull up target pin, and is expected to be inoperative
    GPIO_SetOutValByPin(TargetPin, 1);

    if (P(AuxiliaryPin) != 0)    //Error if successfully pull up the auxiliary pin
    {
        return GPIO_TST_TGT_PULL_UP_MASK_FAIL;
    }

    //Disable Dout mask of target pin
    GPIO_DisableDoutMaskByPin(TargetPin);

    //Pull up target pin, and is expected success
    GPIO_SetOutValByPin(TargetPin, 1);

    if (P(AuxiliaryPin) != 1)    //Error if fail to pull up the auxiliary pin
    {
        return GPIO_TST_TGT_PULL_UP_FAIL;
    }

    /* Test2. Mask and Un-mask before pull down target pin */
    //Enable Dout mask of target pin
    GPIO_EnableDoutMaskByPin(TargetPin);

    //Pull down target pin, and is expected to be inoperative
    GPIO_SetOutValByPin(TargetPin, 0);

    if (P(AuxiliaryPin) != 1)    //Error if successfully pull down the auxiliary pin
    {
        return GPIO_TST_TGT_PULL_DOWN_MASK_FAIL;
    }

    //Disable Dout mask of target pin
    GPIO_DisableDoutMaskByPin(TargetPin);

    //Pull down target pin, and is expected success
    GPIO_SetOutValByPin(TargetPin, 0);

    if (P(AuxiliaryPin) != 0)    //Error if fail to pull down the auxiliary pin
    {
        return GPIO_TST_TGT_PULL_DOWN_FAIL;
    }

    return GPIO_TST_OK;
}
