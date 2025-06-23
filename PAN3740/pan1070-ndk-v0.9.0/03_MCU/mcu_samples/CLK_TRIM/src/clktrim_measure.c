/**************************************************************************//**
 * @file     clktrim_measure.c
 * @version  V1.0
 * $Date:    20/10/14 16:42 $
 * @brief    Common source file for clktrim test.
 *
 * @note
 * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <ctype.h>
#include "PanSeries.h"
#include "clktrim_common.h"


T_TRIM_TEST_RESULT TRIM_MeasureCase1(void)
{
	uint32_t wait_cnt,ca1c_cnt;
	char s[32];
	SYS_TEST("input the wait count u want\n");
	gets(s);
	SYS_TEST(" input wait cnt value:%s \n",s);
	wait_cnt = atoi(s);
	TRIM_SetCalWaitCnt(TRIM,wait_cnt);
	
	SYS_TEST("input the clac count u want\n");
	gets(s);
	SYS_TEST(" input clac cnt value:%s \n",s);
	ca1c_cnt = atoi(s);	
	TRIM_SetCalCnt(TRIM,ca1c_cnt);
	
	TRIM_EnableInt(TRIM,DISABLE);
	TRIM_StartTuning(TRIM,TRIM_MEASURE_TUNING_EN_Msk);
	while(!TRIM_IsIntStatusOccured(TRIM,TRIM_FLAG_MEASURE_STOP_Msk)){}
	TRIM_ClearIntStatusMsk(TRIM,TRIM_FLAG_MEASURE_STOP_Msk);

	SYS_TEST("measure clk is 32M clk, calculate cnt is %d, real cnt :%d \n",
			ca1c_cnt, TRIM_GetRealCnt(TRIM));
    return TRIM_TST_OK;
}
