/**************************************************************************//**
 * @file     gpio_wakeup_test.c
 * @version  V1.0
 * $Date:    19/09/03 17:00 $
 * @brief    GPIO test case 6, wakeup test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "gpio_common.h"


T_GPIO_TEST_RESULT GPIO_WakeupTestCase6(uint8_t TargetPin, uint8_t AuxiliaryPin)
{
    SYS_TEST("\nNOTE:\nPleae refer to related test case in LowPower Sample!\n\n");

    return GPIO_TST_OK;
}
