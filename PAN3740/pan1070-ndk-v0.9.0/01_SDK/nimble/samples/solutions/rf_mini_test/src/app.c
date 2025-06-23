/**
 *******************************************************************************
 * @file     app.c
 * @create   2023-08-01
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "app.h"
#include "app_log.h"
#include "rf_test.h"

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void setup(void)
{
	APP_LOG_INFO("app started\n");	

    /* PHY initialziation. */
	RF_PhyResetEx();	
	
	RF_PhyInit();
    
    /* Set tx power level. */
    RF_SetPowerLevel(CONFIG_RF_TX_POWER_LVL);
 
	/* Single carrier test initiated. */
#if CONSTANT_TONE_TEST_ENABLE
	rf_test_start_constant_tone(0);
	__WFI(); //enter sleep mode 
#endif
}

/**
 *******************************************************************************
 * @brief this is main thread entry if CONFIG_OS_EN=1;
 *        this is main loop entry if CONFIG_OS_EN=0
 *******************************************************************************
 */
void loop(void)
{

}
