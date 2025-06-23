/********************************************************* * 
@file		:fmc_common.c 
@brief  	:common interface header file 
@version	:v1.0 
@date 		:19/08/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/
#ifndef __FMC_COMMON_H__
#define __FMC_COMMON_H__

uint32_t FMC_BasicReadWriteEraseTest(void);
uint32_t FMC_SmallDataReadWriteTest(void);
uint32_t FMC_InfoAreaReadTest(void);
uint32_t FMC_HardwareCrcCheckTest(void);
void FMC_TestFunctionEnter(uint16_t Idx);

#endif // __FMC_COMMON_H__
