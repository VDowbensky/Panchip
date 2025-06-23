/**************************************************************************//**
 * @file     clktrim_interrupt.c
 * @version  V1.0
 * $Date:    20/12/14 17:17 $
 * @brief    Common source file for clktrim test.
 *
 * @note
 * Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <ctype.h>
#include "PanSeries.h"
#include "clktrim_common.h"

static void TRIM_PrintInfoCase3(void)
{
    SYS_TEST("\n");
    SYS_TEST("+--------------------------------------------------------------------------------------+ \n");
    SYS_TEST("|    Press key to start specific testcase:                                             | \n");
    SYS_TEST("|    Input 'a'    coarse tuning test                                                   | \n");
    SYS_TEST("|    Input 'b'    fine tuning test                                                     | \n");
    SYS_TEST("|    Input 'c'    cp all tuning test                                                   | \n");
    SYS_TEST("|    Press ESC key to back to the top level case list.                                 | \n");
    SYS_TEST("+--------------------------------------------------------------------------------------+ \n");
}
void TRIM_IntTuningProcess(uint32_t func,uint8_t flag,uint8_t code,uint32_t cnt)
{
	uint32_t hclk;
	char s[32];
	uint32_t bit_width,code_value,calc_cnt;
	uint32_t fun_en,check_flag;

	/*set bit width value*/
	bit_width = code;
	TRIM_SetBitWidth(TRIM,bit_width);

	/*set code value*/
	code_value = (code_value & ~((0X1 << code) - 1)) | (0X1 << code);
	if(FUNC_SELECT_FINE_TUNING == func){
		TRIM_SetFineCode(TRIM,code_value);
		fun_en = TRIM_FINE_TUNING_EN_Msk;
		check_flag = TRIM_FLAG_FTUNE_STOP_Msk;
	}
	else if(FUNC_SELECT_COARSE_TUNING == func){
		TRIM_SetCoarseCode(TRIM,code_value);
		fun_en = TRIM_COARSE_TUNING_EN_Msk;
		check_flag = TRIM_FLAG_CTUNE_STOP_Msk;
	}else{
		return;
	}

	TRIM_SetRelationIncrease(TRIM,ENABLE);
	TRIM_EnableInt(TRIM,ENABLE);
	TRIM_SetCalWaitCnt(TRIM,4);

	/*set calc cnt value*/
	calc_cnt = cnt;
	TRIM_SetCalCnt(TRIM,calc_cnt);

	/*set ideal cnt value,value = calc_cnt * (hclk / 32k) - 1*/
	hclk = FREQ_32MHZ;
	TRIM_SetIdealCnt(TRIM,hclk/32000*calc_cnt-1);

	/*early termination enable or not*/
	if(EARLY_TERMINATION_OPEN == flag)
		TRIM_SetEarlyTerminatinEn(TRIM,ENABLE);
	else
		TRIM_SetEarlyTerminatinEn(TRIM,DISABLE);

	/*set error range*/
	TRIM_SetErrRange(TRIM,16);
	SYS_TEST("err range:%d \n",(TRIM->TRIM_CTRL>>TRIM_CLRL_ERR_RANGE_Pos));
	/*start tuning*/
	TRIM_StartTuning(TRIM,fun_en);

	/*wait stop & clear flag*/
	while(!stop_int_flag){}

	uint32_t real_cnt = TRIM_GetRealCnt(TRIM);
	float cur_freq = FREQ_32MHZ;
	cur_freq /= real_cnt;
	cur_freq *=	calc_cnt;
	SYS_TEST("real cnt :%d,c/f code:%x,%x,cur_freq:%f\n",TRIM_GetRealCnt(TRIM),TRIM_GetCoarseCode(TRIM),
		TRIM_GetFineCode(TRIM),cur_freq);
	stop_int_flag = 0;
}

void TRIM_IntCPTuningProcess(uint32_t func,uint8_t flag,uint32_t bit_width,uint32_t code_value,uint32_t calc_cnt)
{
	uint32_t hclk;
	uint32_t fun_en,check_flag;

	/*set bit width value*/
	TRIM_SetBitWidth(TRIM,bit_width);

	/*set code value*/
	if(FUNC_SELECT_FINE_TUNING == func){
		TRIM_SetFineCode(TRIM,code_value);
		fun_en = TRIM_FINE_TUNING_EN_Msk;
		check_flag = TRIM_FLAG_FTUNE_STOP_Msk;
	}
	else if(FUNC_SELECT_COARSE_TUNING == func){
		TRIM_SetCoarseCode(TRIM,code_value);
		TRIM_SetFineCode(TRIM,0x80);
		fun_en = TRIM_COARSE_TUNING_EN_Msk;
		check_flag = TRIM_FLAG_CTUNE_STOP_Msk;
	}else{
		return;
	}

	TRIM_SetRelationIncrease(TRIM,ENABLE);
	TRIM_EnableInt(TRIM,ENABLE);
	TRIM_SetCalWaitCnt(TRIM,4);

	/*set calc cnt value*/
	TRIM_SetCalCnt(TRIM,calc_cnt);

	/*set ideal cnt value,value = calc_cnt * (hclk / 32k) - 1*/
	hclk = FREQ_32MHZ;
	TRIM_SetIdealCnt(TRIM,hclk/32000*calc_cnt-1);

	/*early termination enable or not*/
	if(EARLY_TERMINATION_OPEN == flag)
		TRIM_SetEarlyTerminatinEn(TRIM,ENABLE);
	else
		TRIM_SetEarlyTerminatinEn(TRIM,DISABLE);

	/*set error range*/
	TRIM_SetErrRange(TRIM,16);
	/*start tuning*/
	TRIM_StartTuning(TRIM,fun_en);

	/*wait stop & clear flag*/
	while(!stop_int_flag){}

	uint32_t real_cnt = TRIM_GetRealCnt(TRIM);
	float cur_freq = FREQ_32MHZ;
	cur_freq /= real_cnt;
	cur_freq *=	calc_cnt;
	SYS_TEST("real cnt :%d,c/f code:%x,%x,cur_freq:%f\n",TRIM_GetRealCnt(TRIM),TRIM_GetCoarseCode(TRIM),
		TRIM_GetFineCode(TRIM),cur_freq);
	stop_int_flag = 0;
}


T_TRIM_TEST_RESULT TRIM_InterruptTestCase3(void)
{
	char c;

	while(true){
		TRIM_PrintInfoCase3();
		c = getchar();
		switch(c){
			case 'a':
			case 'A':
				SYS_TEST("coarse tuning test select\n");
				TRIM_IntTuningProcess(FUNC_SELECT_COARSE_TUNING,EARLY_TERMINATION_OPEN,3,2);
				break;
			case 'b':
			case 'B':
				SYS_TEST("fine tuning test select\n");
				TRIM_IntTuningProcess(FUNC_SELECT_FINE_TUNING,EARLY_TERMINATION_OPEN,7,2);
				break;
			case 'c':
			case 'C':
				SYS_TEST("all tuning test select\n");
				TRIM_IntCPTuningProcess(FUNC_SELECT_COARSE_TUNING,EARLY_TERMINATION_CLOSE,3,0X8,2);
				TRIM_IntCPTuningProcess(FUNC_SELECT_FINE_TUNING,EARLY_TERMINATION_CLOSE,7,0X80,2);
				break;
			case 0x1b:
				goto OUT;
				break;

			default:
				SYS_WRN("Cannot find subtest case %c!\n", c);
				break;
		}
	}
OUT:
    return TRIM_TST_OK;
}
