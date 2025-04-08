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
#include "oled.h"
#include "display.h"

extern struct RxDoneMsg RxDoneParams;
extern uint8_t tx_carrywave_flag;
uint32_t app_tx_mode = 0;

void SysTick_IrqHandler(void)
{
    SysTick_IncTick();
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int32_t main(void)
{
    uint32_t ret = 0;

    BSP_CLK_Init();
    Timer4_Init(); //delay
	Timer6_Cfg();
    SysTick_Init(1000u);
    SysTick_IncTick();
    BSP_GPIO_Init();
    OLED_Init();
    RF_IRQ_Init();
    BSP_LED_Init();
    BSP_KEY_Init();
    Spi_Config();
    Uart_Config();

    if(TX_RX_SELECT == TX_SELECT)
    {
        app_tx_mode = 1;
    }
	
    if(!app_tx_mode)
    {
		Timer0_Init();
    }
	
    ret = rf_init();
    if(ret != OK)
    {
        dis_err(" RF Init Fail");
        while(1);
    }
	DDL_Printf("RF Init OK\r\n");
    rf_set_default_para();
    show_menu();
    dis_init();
    OLED_Refresh_Gram();
    if(app_tx_mode)
    {
        DDL_Printf("tx\r\n");
    }
    else
    {
        DDL_Printf("rx\r\n");
		OLED_ShowString(0,16,"E:",16);
		OLED_Refresh_Gram();
		timer6_open_ms(20);
		rf_cad_on(CAD_DETECT_THRESHOLD_0A,CAD_DETECT_NUMBER_1);
        rf_enter_continous_rx();
    }
	
    while(1) {
		
		shou_rssi_noise();
        if(!tx_carrywave_flag)
        {
          rf_irq_process();
        }
        key_scan();
        key_event_process();
        process_rf_events();
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
