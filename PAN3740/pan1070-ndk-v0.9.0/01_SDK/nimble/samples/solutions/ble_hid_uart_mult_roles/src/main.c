/**
 *******************************************************************************
 * @file     main.c
 * @create   2023-08-01    
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "app.h"

int main(void)
{
	/* user initialization entry. */
	setup();
	
	/**
	 *  "main()" is a thread if CONFIG_OS_EN=1
 	 *  "main()" is a main entry if CONFIG_OS_EN=0
	 *  
	 *  If the user is using the main thread, you need to enable the following code 
	 *  and add the user's processing logic to the "loop()" function.
	 */
	while(1)
	{
		loop();
	}
}
