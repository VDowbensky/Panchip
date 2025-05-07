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

#define TX_LEN  10
uint8_t tx_test_buf[TX_LEN] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

extern struct RxDoneMsg RxDoneParams;

void SysTick_IrqHandler(void)
{
    SysTick_IncTick();
}

int32_t main(void)
{
    uint32_t ret = 0;
    uint32_t cnt = 0;

    BSP_CLK_Init();
    Timer4_Init();
    SysTick_Init(1000u);
    SysTick_IncTick();

    BSP_GPIO_Init();
    RF_IRQ_Init();
    BSP_LED_Init();
    Spi_Config();
    Uart_Config();

    ret = rf_init();
    if(ret != OK)
    {
        DDL_Printf("RF Init Fail");
        while(1);
    }else{
		DDL_Printf("RF Init OK\r\n");
	}
	DDL_Printf("TX MODE\r\n");
    rf_set_default_para();
    rf_enter_continous_tx();
    if(rf_continous_tx_send_data(tx_test_buf, TX_LEN) != OK)
    {
        DDL_Printf("tx fail \r\n");
    }
    else
    {
        cnt ++;
        DDL_Printf("Tx cnt %d\r\n", cnt );
    }

    while (1)
    {
        rf_irq_process();
        if(rf_get_transmit_flag() == RADIO_FLAG_TXDONE)
        {
            BSP_LED_Toggle();
            rf_set_transmit_flag(RADIO_FLAG_IDLE);
            rf_sleep();
			rf_delay_ms(1000);
			rf_sleep_wakeup();

            if(rf_continous_tx_send_data(tx_test_buf, TX_LEN) != OK)
            {
                DDL_Printf("tx fail \r\n");
            }
            else
            {
                cnt ++;
                DDL_Printf("Tx cnt %d\r\n", cnt );
            }
        }
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
