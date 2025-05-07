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
    uint32_t rx_cnt = 0;
    uint32_t rx_max_times = 0;
    uint32_t bw,sf,one_chirp_time;

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
    rf_set_bw(TEST_BW);
    rf_set_sf(TEST_SF);
	sf = rf_get_sf();
	bw = rf_get_bw();
    one_chirp_time = rf_get_chirp_time(bw,sf);

    rf_cad_on(CAD_DETECT_THRESHOLD_10,CAD_DETECT_NUMBER_3);
    rf_enter_continous_rx();
    DDL_Printf("please check GPIO_PORT_CAD is OK\r\n");

    while (1)
    {
        if(check_cad_rx_inactive(one_chirp_time) == LEVEL_ACTIVE)
        {
            DDL_Printf("cad\r\n");
            rx_max_times = 0;

            while(1)
            {
                rf_irq_process();
                if(rf_get_recv_flag() == RADIO_FLAG_RXDONE)
                {
                    BSP_LED_Toggle();
                    rf_set_recv_flag(RADIO_FLAG_IDLE);

                    DDL_Printf("###Rx len  %d\r\n", RxDoneParams.Size);
                    rx_cnt ++;
                    DDL_Printf("###Rx cnt %d##\r\n", rx_cnt);
                    break;
                }
                if((rf_get_recv_flag() == RADIO_FLAG_RXTIMEOUT) || (rf_get_recv_flag() == RADIO_FLAG_RXERR))
                {
                    rf_set_recv_flag(RADIO_FLAG_IDLE);
                    DDL_Printf("Rxerr\r\n");
                    break;
                }

                SysTick_Delay_10us(1);

                rx_max_times++;
                if(rx_max_times < one_chirp_time/10*2)/*2-chirp-cad-check*/
                {
                    if(CHECK_CAD() != 1)
                    {
                        DDL_Printf("break1\r\n");
                        break;
                    }
                }
                if(rx_max_times >= one_chirp_time/10*800)/*500-chirp-timeout*/
                {
                    printf("break2\r\n");
                    break;
                }
            }
        }
        else
        {
			printf("sleep\r\n");
            rf_deepsleep();
			rf_delay_ms(1000);
			rf_init();
			rf_set_default_para();
			rf_set_bw(TEST_BW);
			rf_set_sf(TEST_SF);
			rf_cad_on(CAD_DETECT_THRESHOLD_10,CAD_DETECT_NUMBER_3);
			rf_enter_continous_rx();
        }
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
