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

#define  SET_TIMER_MS(time)     timer6_open_ms(time)

uint8_t tx_test_buf[256];
uint32_t fre_lib[2] = {480250000,473550000};
volatile uint32_t cad_tx_detect_flag = MAC_EVT_TX_CAD_NONE;

extern struct RxDoneMsg RxDoneParams;

void SysTick_IrqHandler(void)
{
    SysTick_IncTick();
}

uint32_t rf_get_cad_detect_flag(void)
{
    return cad_tx_detect_flag;
}

void rf_set_cad_detect_flag(int status)
{
    cad_tx_detect_flag = status;
}

uint32_t rf_cad_detect_start(void)
{
    uint32_t bw, sf;
    uint32_t one_chirp_time;

    bw = rf_get_bw();
    sf = rf_get_sf();
    one_chirp_time = rf_get_chirp_time(bw, sf); // us

    RF_ASSERT(rf_cad_on(CAD_DETECT_THRESHOLD_10, CAD_DETECT_NUMBER_3));

    cad_tx_detect_flag = MAC_EVT_TX_CAD_NONE;

    RF_ASSERT(rf_enter_continous_rx())

    SET_TIMER_MS(one_chirp_time * 7 / 1000 + 1);

    return OK;
}

int32_t main(void)
{
    uint32_t ret = 0;
    uint32_t cnt = 0;
    uint32_t i=0;
    uint32_t dat[100];
    uint8_t freq_cnt = 0;

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
    CAD_GPIO_Init();

    for(i = 0; i < 256; i++)
    {
        tx_test_buf[i] = i;
    }

    for(i=0; i<100; i++)
    {
        dat[i] = (rand()%255 + 1);//len
    }

    ret = rf_init();
    if(ret != OK)
    {
        DDL_Printf(" RF Init Fail");
        while(1);
    }
	DDL_Printf("RF Init ok");
    rf_set_default_para();
    rf_set_preamble(170);//table
    rf_enter_continous_tx();

    rf_set_transmit_flag(RADIO_FLAG_TXDONE);//start

    DDL_Printf("TX start: %d, %d\r\n",fre_lib[0],fre_lib[1]);

    while (1)
    {
        rf_irq_process();
        if(rf_get_transmit_flag() == RADIO_FLAG_TXDONE)
        {
            BSP_LED_Toggle();
            rf_set_transmit_flag(RADIO_FLAG_IDLE);

            i++;
            if(i>=100)i=0;

            cnt++;
            tx_test_buf[0] = cnt>>8;
            tx_test_buf[1] = cnt;
            tx_test_buf[2] = dat[i];//len

            rf_delay_ms(dat[i]+150);
            rf_set_mode(RF_MODE_STB3);
            freq_cnt = rand()%9;
            freq_cnt %= 2;
            if(freq_cnt)
            {
                tx_test_buf[3] = freq_cnt;
                rf_set_freq(fre_lib[freq_cnt]);
            }
            else
            {
                tx_test_buf[3] = freq_cnt;
                rf_set_freq(fre_lib[freq_cnt]);
            }

            rf_cad_detect_start();
            while(1)
            {
                if(rf_get_cad_detect_flag() == MAC_EVT_TX_CAD_TIMEOUT)
                {
                    rf_set_cad_detect_flag(MAC_EVT_TX_CAD_NONE);
                    rf_enter_continous_tx();
                    if(rf_continous_tx_send_data(tx_test_buf, dat[i]) != OK)
                    {
                        DDL_Printf("tx fail \r\n");
                    }
                    else
                    {
                        DDL_Printf("Tx cnt %d, len %d, freq %d \r\n", cnt, dat[i], fre_lib[freq_cnt]);
                    }
                    break;

                } else if(rf_get_cad_detect_flag() == MAC_EVT_TX_CAD_ACTIVE)
                {
                    rf_set_cad_detect_flag(MAC_EVT_TX_CAD_NONE);
                    rf_set_mode(RF_MODE_STB3);
                    rf_delay_ms(dat[i]+150);
                    rf_set_transmit_flag(RADIO_FLAG_TXDONE);
                    break;
                }
            }
        }
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
