/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file main.c
 * @brief
 *
 * @history - V0.8, 2024-4
*******************************************************************************/
#include "hc32_ddl.h"
#include "pan_rf.h"
#include <at.h>

extern struct RxDoneMsg RxDoneParams;

void SysTick_IrqHandler(void)
{
    SysTick_IncTick();
}

int32_t main(void)
{
	BSP_CLK_Init();
	
	SysTick_Init(1000u);
	SysTick_IncTick();
	BSP_GPIO_Init();
	RF_IRQ_Init();
	BSP_LED_Init();
	Spi_Config();
	Uart_Config();
	Timer4_Init();
	at_server_init();
	
	int ret = rf_init();
	if(ret != OK)
	{
		printf("rf init err");
	}
	else
	{
		printf("rf init ok.");
	}	
	
	while (1)
	{
		at_server_process();
	}
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
