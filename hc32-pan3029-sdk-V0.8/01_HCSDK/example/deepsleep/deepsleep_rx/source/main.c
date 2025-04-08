/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file radio.c
 * @brief
 *
 * @history - V0.8, 2024-4
*******************************************************************************/
#include "hc32_ddl.h"
#include "pan_rf.h"
#include "ev_hc32f460_lqfp100_v2.h"

#define TEST_SF             SF_10
#define TEST_BW             BW_125K

extern struct RxDoneMsg RxDoneParams;

void SysTick_IrqHandler(void)
{
    SysTick_IncTick();
}

int32_t main(void)
{
    uint32_t ret = 0;
    uint32_t cnt = 0;
    uint8_t i;

    BSP_CLK_Init();
    Timer4_Init();
    SysTick_Init(1000u);
    SysTick_IncTick();
    BSP_GPIO_Init();
    RF_IRQ_Init();
    BSP_LED_Init();
    Spi_Config();
    Uart_Config();
    CAD_GPIO_Init();
    BSP_A15B03_Init();
    ret = rf_init();
    if(ret != OK)
    {
        DDL_Printf(" RF Init Fail");
        while(1);
    }

    rf_set_default_para();
    rf_enter_continous_rx();
    DDL_Printf("rx deepsleep\r\n");

    while (1)
    {
        rf_irq_process();
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
            cnt ++;
            DDL_Printf("###Rx cnt %d##\r\n", cnt);
        }
        if((rf_get_recv_flag() == RADIO_FLAG_RXTIMEOUT) || (rf_get_recv_flag() == RADIO_FLAG_RXERR))
        {
            rf_set_recv_flag(RADIO_FLAG_IDLE);
            DDL_Printf("Rxerr\r\n");
        }
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
