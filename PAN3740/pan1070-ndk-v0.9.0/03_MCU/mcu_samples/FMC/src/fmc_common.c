/********************************************************* * 
@file       :fmc_common.c 
@brief      :common interface source file,store some common interface functions
@version    :v1.0 
@date       :19/10/31
Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
***********************************************************/
#include "PanSeries.h"
#include "fmc_common.h"

uint32_t (*FMC_TestEntry[])(void) = 
{
    FMC_BasicReadWriteEraseTest,
    FMC_SmallDataReadWriteTest,
    FMC_HardwareCrcCheckTest,
    FMC_InfoAreaReadTest
};

void FMC_TestFunctionEnter(uint16_t Idx)
{
    (FMC_TestEntry[Idx])();
}
