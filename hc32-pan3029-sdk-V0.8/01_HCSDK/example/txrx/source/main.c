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
    uint32_t txcnt = 0;
    uint32_t rxcnt = 0;
    uint8_t i;
    uint32_t tx_time=0;

    BSP_CLK_Init();
    Timer4_Init();
    Timer6_Cfg();
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
	DDL_Printf("TXRX MODE\r\n");
    rf_set_default_para();
    //tx first
    if(rf_single_tx_data(tx_test_buf, TX_LEN, &tx_time) != OK)
    {
        DDL_Printf("tx fail \r\n");
    }

    while (1)
    {
        rf_irq_process();
        //rxdone flag
        if(rf_get_recv_flag() == RADIO_FLAG_RXDONE)
        {
            BSP_LED_Toggle();
            rf_set_recv_flag(RADIO_FLAG_IDLE);
            DDL_Printf("Rx : SNR: %f ,RSSI: %d \r\n", RxDoneParams.Snr, RxDoneParams.Rssi);
            for(i = 0; i < RxDoneParams.Size; i++)
            {
                DDL_Printf("0x%02x ", RxDoneParams.Payload[i]);
            }
            DDL_Printf("\r\n");
            rxcnt ++;
            DDL_Printf("###Rx cnt %d##\r\n", rxcnt);
            //rxdone
            SysTick_Delay(3000);
            //tx
            if(rf_single_tx_data(tx_test_buf, TX_LEN, &tx_time) != OK)
            {
                DDL_Printf("tx fail \r\n");
            }
        }
        //rxtimeout or rxerr flag
        if((rf_get_recv_flag() == RADIO_FLAG_RXTIMEOUT) || (rf_get_recv_flag() == RADIO_FLAG_RXERR))
        {
            rf_set_recv_flag(RADIO_FLAG_IDLE);
            DDL_Printf("Rxerr\r\n");
            //rxtimeout or rxerr
            //tx again
            SysTick_Delay(10000);
            if(rf_single_tx_data(tx_test_buf, TX_LEN, &tx_time) != OK)
            {
                DDL_Printf("tx fail \r\n");
            }
        }
        if(rf_get_transmit_flag() == RADIO_FLAG_TXDONE)
        {
            rf_set_transmit_flag(RADIO_FLAG_IDLE);
            txcnt ++;
            DDL_Printf("Tx cnt %d\r\n", txcnt );
            //txdone
            //single_timeout_rx
            rf_enter_single_timeout_rx(15000);
        }
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
