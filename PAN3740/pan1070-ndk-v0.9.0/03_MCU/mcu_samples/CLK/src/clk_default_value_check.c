/********************************************************* * 
@file		:clk_default_value_check.c 
@brief  	:clk default value test
@author 	:zhongfeng
@version	:v1.0 
@date 		:22/02/17
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "clk_common.h"

const uint32_t ClkDefaultValue[] =
    {
        0x00000040, // RSTSTS;            //0x40040000
        0x00000000, // IPRST0;            //0x40040004
        0x00000000, // IPRST1;            //0x40040008
        0x000c0040, // BODCTL;            //0x4004000C
        0x00000101, // BLDBCTL;           //0x40040010
        0x00000000, // RSV[3];
        0x00000000, //
        0x00000000, //
        0x00000000, // CLK_TOP_CTRL_3V;      0x40040020
        0x01048081, // RCL_CTRL;          0x40040018
        0x01035021, // RCH_CTRL;          0x4004001C
        0x00031020, // XTL_CTRL_3V;          0x4004002C
        0x00000020, // XTH_CTRL;          0x40040024
        0x00001480, // DPLL_CTRL;         0x40040028
        0x00010C26, // AHB_CLK_CTRL;      0x4004002C
        0x00000000, // APB1_CLK_CTRL;     0x40040030
        //    0x00400000,        // APB1_CLK_CTRL1;    0x40040040
        0x00000000, // APB2_CLK_CTRL;     0x40040034
        //    0x00F40002,        // APB2_CLK_CTRL1;    0x40040048
        //    0x00000000,        // ACT_32K_CTRL;      0x4004004C
        //    0x00000000,        // ACT_32K_CNT;       0x40040050
        0x00F40000, // MEAS_CLK_CTRL      0x40040038
};

/*
  * @brief	   :clk register default value check
  * @retval    :0/1
*/
uint32_t CLK_DefaultRegValTestCase7(void)
{
    uint8_t i = 0,different = 0;
    
    while(i<sizeof(ClkDefaultValue)/sizeof(uint32_t)){
        if(((__IO uint32_t *)(&CLK->RSTSTS))[i] != ClkDefaultValue[i]){
            SYS_TEST("Clk register addr offset:%x,expect:%08x,current:%08x \r\n",i*4,ClkDefaultValue[i],((__IO uint32_t *)(&CLK->RSTSTS))[i]);
            different++;
        }
        i++;
    }
    if(!different){
       SYS_TEST("clk default value check ok \r\n"); 
    }
	return 0; 

}

