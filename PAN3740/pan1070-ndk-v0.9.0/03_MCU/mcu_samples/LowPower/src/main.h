/*******************************************************************************
 * @note      Copyright (C) 2024 Shanghai Panchip Microelectronics Co., Ltd.
 *            All rights reserved.
 * @file      main.h
 * @brief     Main header file of PAN10xx Low Power Mode Demo
*******************************************************************************/
#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdint.h>

extern void platform_init(void);

extern void LP_TestSleepMode(void);
extern void LP_TestDeepSleepMode(void);
extern void LP_TestStandbyMode1(void);
extern void LP_TestStandbyMode0(void);

#endif  //__MAIN_H__
