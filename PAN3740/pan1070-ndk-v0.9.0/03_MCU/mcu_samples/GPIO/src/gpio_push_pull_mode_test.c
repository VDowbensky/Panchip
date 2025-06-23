/**************************************************************************//**
 * @file     gpio_push_pull_mode_test.c
 * @version  V1.0
 * $Date:    19/09/03 17:00 $
 * @brief    GPIO test case 1, push-pull output test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "gpio_common.h"


T_GPIO_TEST_RESULT GPIO_PushPullModeTestCase1(uint8_t TargetPin, uint8_t AuxiliaryPin)
{
    SYS_ConfigMFP(TargetPin, SYS_MFP_GPIO);
    SYS_ConfigMFP(AuxiliaryPin, SYS_MFP_GPIO);

    GPIO_SetModeByPin(TargetPin, GPIO_MODE_OUTPUT); //Push-pull mode
    GPIO_SetModeByPin(AuxiliaryPin, GPIO_MODE_INPUT);

    //CLK_Wait3vSyncReady();

    //1. Target pin pull down
    P(TargetPin) = 0;

    SYS_delay_10nop(20);

    if (P(AuxiliaryPin) != 0)
    {
        return GPIO_TST_TGT_PULL_DOWN_FAIL;
    }

    //2. Target pin pull up
    P(TargetPin) = 1;

    SYS_delay_10nop(20);

    if (P(AuxiliaryPin) != 1)
    {
        return GPIO_TST_TGT_PULL_UP_FAIL;
    }

    return GPIO_TST_OK;
}
