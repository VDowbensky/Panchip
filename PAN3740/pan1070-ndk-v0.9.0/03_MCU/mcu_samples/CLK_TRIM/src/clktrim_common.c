/**************************************************************************//**
 * @file     clktrim_common.c
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

uint8_t stop_int_flag = 0,hw_trig_flag = 0;

T_TRIM_TEST_RESULT (*const TRIM_TestCase[])(void) = 
{
    TRIM_RegisterDefaultValueCheckCase0,
    TRIM_MeasureCase1,
    TRIM_TuningCase2,
    TRIM_InterruptTestCase3
};

void TRIM_TestFunctionEnter(uint16_t TcIdx)
{
    T_TRIM_TEST_RESULT r = TRIM_TST_OK;

    if (TcIdx >= sizeof(TRIM_TestCase) / sizeof(void*))
    {
        SYS_TEST("Error, cannot find Testcase %d!", TcIdx);
        return;
    }

    r = (TRIM_TestCase[TcIdx])();
    if (r != TRIM_TST_OK)
    {
        SYS_TEST("TRIM Test Fail, Fail case: %d, Error Code: %d\n", TcIdx, r);
    }
    else
    {
        SYS_TEST("TRIM Test OK, Success case: %d\n", TcIdx);
    }
}


void TRIM_TestModuleInit(void)
{
    // Enable Clock
	CLK_EnableClkTrim(ENABLE);
	CLK_SelectClkTrimSrc(CLKTRIM_CALC_CLK_SEL_32K);
	CLK_Set3vSyncAuto();
	NVIC_EnableIRQ(TRIM_IRQn);
    // P11 would be configure as UART0 Tx when use PAN101x
	SYS_SET_MFP(P1,1,CLK_32K);
}

void TRIM_IRQHandler(void)
{
	SYS_TEST("1--TRIM->TRIM_CTRL:%x\n",TRIM->TRIM_CTRL);
	if(TRIM_IsIntStatusOccured(TRIM,TRIM_INT_MEASURE_STOP_Msk))
	{
		SYS_TEST("m \n");
//		stop_int_flag = 1;
		TRIM_ClearIntStatusMsk(TRIM,TRIM_INT_MEASURE_STOP_Msk);
	}
	if(TRIM_IsIntStatusOccured(TRIM,TRIM_INT_CTUNE_STOP_Msk))
	{
		SYS_TEST("c \n");
//		stop_int_flag = 1;
		TRIM_ClearIntStatusMsk(TRIM,TRIM_INT_CTUNE_STOP_Msk);
	}	
	if(TRIM_IsIntStatusOccured(TRIM,TRIM_INT_FTUNE_STOP_Msk))
	{
		SYS_TEST("f \n");
//		stop_int_flag = 1;
		TRIM_ClearIntStatusMsk(TRIM,TRIM_INT_FTUNE_STOP_Msk);
	}		

	if(TRIM_IsIntStatusOccured(TRIM,TRIM_INT_OVERFLOW_Msk))
	{
		SYS_TEST("overflow \n");
		TRIM_ClearIntStatusMsk(TRIM,TRIM_INT_OVERFLOW_Msk);
	}
	stop_int_flag = 1;
	SYS_TEST("2--TRIM->TRIM_REAL_CNT:%x\n",TRIM->TRIM_REAL_CNT);
}

const uint32_t TrimDefaultValue[]=
{
    0x00000000,        // TRIM_EN       
    0x00078008,        // TRIM_CODE     
    0x000f0f02,        // TRIM_CTRL     
    0x00000001,        // TRIM_INT      
    0x0400001e,        // TRIM_CAL_CNT  
    0x0001869f,        // TRIM_IDEAL_CNT
    0x00000000,        // TRIM_REAL_CNT 
};

T_TRIM_TEST_RESULT TRIM_RegisterDefaultValueCheckCase0(void)
{

    uint8_t i = 0,different = 0;
    
    while(i<sizeof(TrimDefaultValue)/sizeof(uint32_t)){
        if(((__IO uint32_t *)(&TRIM->TRIM_EN))[i] != TrimDefaultValue[i]){
            SYS_TEST("addr offset:%x,expect:%08x,current:%08x \r\n",i*4,TrimDefaultValue[i],((__IO uint32_t *)(&TRIM->TRIM_EN))[i]);
            different++;
        }
        i++;
    }
    if(!different){
       SYS_TEST("clk trim default value check ok \r\n"); 
    }	
	
    return TRIM_TST_OK;
}
