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
#include "hc32f460.h"
#include "ev_hc32f460_lqfp100_v2.h"


#define  TEST_BW    BW_500K
#define  TEST_SF    SF_7
#define  TEST_CR    CODE_RATE_45
#define  TEST_FREQ  473000000

#define  ADDR_MATCH 

uint8_t recv_flag = 0;  //data receiving flag  0:Not in data receiving, 1:data receiving
uint8_t mapm_sleep_flag= 0;
uint32_t app_tx_mode = 0;
static uint32_t rx_timeout_val;  //
static uint32_t cad_check_time;
static uint32_t sleep_timer_val;  //Sleep timing
static uint32_t one_chirp_time;


stc_mapm_cfg_t cur_mapm_cfg;
extern uint32_t tx_len;  //tx send payload length
extern volatile uint32_t cad_flag;
extern uint8_t timera_wake_flag;
extern struct RxDoneMsg RxDoneParams;
static uint16_t m_au16Adc1Value[ADC1_CH_COUNT];

/* Default parameters are mapm parameters for SF7~SF12, BW500K */
const stc_mapm_cfg_t mapm_cfg[] =
{
    {
        .mapm_addr= {0x73, 0x62, 0x54, 0x81},    //SF7 BW_500K
        .fn = 64,
        .fnm = 0,
        .gfs = FIELD_COUNTER,
        .gn = 2,
        .pg1 = 10,
        .pgn = 120,
        .pn = 8
    },
    {
        .mapm_addr= {0x73, 0x62, 0x54, 0x81},    //SF8 BW_500K
        .fn = 32,
        .fnm = 0,
        .gfs = FIELD_COUNTER,
        .gn = 2,
        .pg1 = 10,
        .pgn = 120,
        .pn = 8
    },
    {
        .mapm_addr= {0x73, 0x62, 0x54, 0x81},    //SF9 BW_500K
        .fn = 16,
        .fnm = 0,
        .gfs = FIELD_COUNTER,
        .gn = 2,
        .pg1 = 10,
        .pgn = 120,
        .pn = 8
    },
    {
        .mapm_addr= {0x73, 0x62, 0x54, 0x81},    //SF10 BW_500K,only for PAN3029
        .fn = 8,
        .fnm = 0,
        .gfs = FIELD_COUNTER,
        .gn = 2,
        .pg1 = 10,
        .pgn = 120,
        .pn = 8
    },
    {
        .mapm_addr= {0x73, 0x62, 0x54, 0x81},    //SF11 BW_500K,only for PAN3029
        .fn = 6,
        .fnm = 0,
        .gfs = FIELD_COUNTER,
        .gn = 2,
        .pg1 = 10,
        .pgn = 120,
        .pn = 8
    },
    {
        .mapm_addr= {0x73, 0x62, 0x54, 0x81},    //SF12 BW_500K,only for PAN3029
        .fn = 6,
        .fnm = 0,
        .gfs = FIELD_COUNTER,
        .gn = 2,
        .pg1 = 10,
        .pgn = 120,
        .pn = 8
    },
};

void SysTick_IrqHandler(void)
{
    SysTick_IncTick();
}

uint32_t calculate_payload_time(uint32_t payload_len)
{
    uint8_t sf = rf_get_sf();
    uint8_t cr = rf_get_code_rate();
    uint8_t bw = rf_get_bw();
    uint32_t ldr = rf_get_ldr();

    const float bw_table[10] = {0, 0, 0, 0, 0, 0, 62.5, 125, 250, 500};

    float symbol_len = (float)(1 << sf) / bw_table[bw]; // symbol length
    float payload_time = 0;                             // payload time:ms

    if (sf < 7)
    {
        payload_time = ceil((float)(payload_len * 8 - sf * 4 + 36) / ((sf - ldr * 2) * 4));
    }
    else
    {
        payload_time = ceil((float)(payload_len * 8 - sf * 4 + 44) / ((sf - ldr * 2) * 4));
    }
    payload_time = payload_time * (cr + 4);
    payload_time = payload_time + 8;
    payload_time = payload_time * symbol_len;

    return payload_time;
}

#define CODE_RUNTIME 10 // Code runtime, set by the user based on actual situation
void process_rf_events()
{
    uint8_t addr_num = 0;
    static uint8_t mapm_target = 0;
    uint16_t one_field_chirp;
    static uint32_t mapm_sleep_begtime;
    static uint32_t mapm_sleeptime;
    uint32_t mapm_preamble_time;
    uint32_t flag = 0;

    flag = rf_get_transmit_flag();
    if (flag == RADIO_FLAG_TXDONE)
    {
        DDL_Printf("REG_IRQ_TX_DONE \r\n");
        BSP_LED_Toggle();
        rf_tx_done_routine();
        rf_set_transmit_flag(RADIO_FLAG_IDLE);
    }

    flag = rf_get_recv_flag();
    if (flag == RADIO_FLAG_MAPM)
    {
        // DDL_Printf("REG_IRQ_MAPM_DONE \r\n");
        rf_set_recv_flag(RADIO_FLAG_IDLE);

        /*Received a complete field,one field data number is mapm_cfg.gn + 1*/
        if (RxDoneParams.mpam_recv_index == cur_mapm_cfg.gn + 1)
        {
            uint8_t i;

            if (cur_mapm_cfg.gfs == FIELD_COUNTER)
            {
                addr_num = cur_mapm_cfg.gn;
            }
            else
            {
                addr_num = cur_mapm_cfg.gn + 1;
            }
            for (i = 0; i < addr_num; i++)
            {
                if (cur_mapm_cfg.mapm_addr[i] != RxDoneParams.mpam_recv_buf[i]) // not the data receiving target
                {
                    mapm_target = 0;
                    break;
                }
            }
            if (i == addr_num)
            {
                mapm_target = 1; // Is the data receiving target
            }
            if (cur_mapm_cfg.gfs == FIELD_COUNTER)
            {
                cur_mapm_cfg.fn = RxDoneParams.mpam_recv_buf[cur_mapm_cfg.gn];
                mapm_preamble_time = rf_calculate_mapm_preambletime(&cur_mapm_cfg, one_chirp_time);
                one_field_chirp = cur_mapm_cfg.pg1 + 2 + (cur_mapm_cfg.pgn + 2) * cur_mapm_cfg.gn;
                if (mapm_target)
                {
                    /*mapm can sleep time, currently, a field time has passed,and wake up one field time in advance*/
                    mapm_sleeptime = mapm_preamble_time - one_field_chirp * one_chirp_time / 1000;
                    if (mapm_sleeptime > CODE_RUNTIME)
                    {
                        rf_sleep();
                        mapm_sleep_flag = 1;
                        mapm_sleep_begtime = SysTick_GetTick();
                    }
                    else
                    {
                        DDL_Printf("sleep time too short\r\n");
                    }
                }
                else
                {
                    memset(RxDoneParams.mpam_recv_buf, 0, RxDoneParams.mpam_recv_index);
                    RxDoneParams.mpam_recv_index = 0;
                    mapm_sleeptime = mapm_preamble_time + calculate_payload_time(tx_len);
                    rf_sleep();
                    mapm_sleep_flag = 1;
                    mapm_sleep_begtime = SysTick_GetTick();
                }
            }
        }
    }

    if (flag == RADIO_FLAG_RXDONE)
    {
        uint8_t i;
        recv_flag = 0;
        BSP_LED_Toggle();
        rf_rx_done_routine();
        rf_set_recv_flag(RADIO_FLAG_IDLE);
        if (RxDoneParams.mpam_recv_index)
        {
            uint16_t i;
            for (i = 0; i < RxDoneParams.mpam_recv_index; i++)
            {
                DDL_Printf("mapm addr[%d] =%02x\r\n", i, RxDoneParams.mpam_recv_buf[i]);
            }
            memset(RxDoneParams.mpam_recv_buf, 0, RxDoneParams.mpam_recv_index);
            DDL_Printf("mapm addr num =%d\r\n", RxDoneParams.mpam_recv_index);
            RxDoneParams.mpam_recv_index = 0;
        }

        DDL_Printf("Rx : SNR: %f ,RSSI: %d \r\n", RxDoneParams.Snr, RxDoneParams.Rssi);
        for (i = 0; i < RxDoneParams.Size; i++)
        {
            DDL_Printf("0x%02x ", RxDoneParams.Payload[i]);
        }
        DDL_Printf("\r\n");
        DDL_Printf("%x ", rf_read_pkt_cnt());
    }
    if (flag == RADIO_FLAG_RXERR)
    {
        DDL_Printf("recv erro\r\n");
        rf_set_recv_flag(RADIO_FLAG_IDLE);
        recv_flag = 0;
    }
    if (flag == RADIO_FLAG_RXTIMEOUT)
    {
        DDL_Printf("recv timeout\r\n");
        rf_set_recv_flag(RADIO_FLAG_IDLE);
        recv_flag = 0;
    }

    if (mapm_sleep_flag == 1)
    {
        if ((SysTick_GetTick() - mapm_sleep_begtime) > (mapm_sleeptime - CODE_RUNTIME))
        {
            rf_sleep_wakeup();
            mapm_sleep_flag = 0;
            if (mapm_target)
            {
                mapm_target = 0;
            }
            else
            {
                recv_flag = 0;
            }
            rf_enter_single_timeout_rx(rx_timeout_val);
        }
    }
}

void syncword_update(uint32_t sf)
{
  
    rf_set_syncword(0x75);
   
}

extern bool sf_chg_flag;
extern bool bw_chg_flag;
extern uint32_t now_sf,now_bw;
void sf_bw_change_update(void)
{
    uint16_t multiple = 1;
    if(sf_chg_flag || bw_chg_flag)
    {
        // Get the current SF, BW500K configuration
        memcpy((uint8_t*)&cur_mapm_cfg, (uint8_t*)&mapm_cfg[now_sf-SF_7], sizeof(stc_mapm_cfg_t));

        /* if the current BW is not BW_500K, reduce the number of preambles by the corresponding multiple 
           to maintain the same preamble length under the same SF
         */
        if(now_bw != BW_500K)
        {
            multiple = 1 << (BW_500K - now_bw);

            // Get the number of chirps in a field in the current mapm configuration
            uint16_t field_chirps = cur_mapm_cfg.pg1 + cur_mapm_cfg.gn * cur_mapm_cfg.pgn + (cur_mapm_cfg.gn + 1) * 2;
            // Change the number of preambles according to the multiple
            cur_mapm_cfg.pgn = (field_chirps/multiple - (cur_mapm_cfg.gn + 1) * 2 - cur_mapm_cfg.pg1) / cur_mapm_cfg.gn;
            rf_set_mapm_group_preamble_num(cur_mapm_cfg.pgn);
        }

        if(sf_chg_flag)
        {
            syncword_update(now_sf);
            rf_set_mapm_cfg(&cur_mapm_cfg);
        }

        one_chirp_time = rf_get_chirp_time(now_bw, now_sf);//us
        uint32_t preamble_time = rf_calculate_mapm_preambletime(&cur_mapm_cfg, one_chirp_time);
        rx_timeout_val =preamble_time + calculate_payload_time(tx_len) + 200;
        sleep_timer_val = preamble_time/2;

        if(one_chirp_time<100)
        {
            cad_check_time = 1 + 1;
        }
        else
        {
            cad_check_time = 10*one_chirp_time/1000 + 1;
        }

        sf_chg_flag = bw_chg_flag = 0;
    }
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int32_t main(void)
{
    uint8_t sleep_flag = 0; // 0:sleep, 1:wakeup
    uint8_t cad_judge_flag = 0;
    uint32_t cad_judge_begtime;
    uint32_t ret = 0;
    double bat_value = 0;

    BSP_CLK_Init();
    SysTick_Init(1000u);
    SysTick_IncTick();
    Timer4_Init();

    Timera_Init();
    BSP_GPIO_Init();
    OLED_Init();
    RF_IRQ_Init();
    CAD_PA09_Init();
    BSP_LED_Init();
    BSP_KEY_Init();
    Spi_Config();
    Uart_Config();
    AdcConfig();

    ADC_PollingSa(M4_ADC1, m_au16Adc1Value, ADC1_CH_COUNT, TIMEOUT_VAL);
    bat_value = ((float)m_au16Adc1Value[0u] * ADC_VREF * (1.500f)) / (float)ADC1_ACCURACY;

    if (bat_value > 3.5)
    {
        DDL_Printf("ADC12_IN10 voltage is %.4fV.\r\n", bat_value);
    }
    else
    {
        BAT_LED_ON();
    }

    if (TX_RX_SELECT == TX_SELECT)
    {
        app_tx_mode = 1;
    }

    ret = rf_init();
    if (ret != OK)
    {
        dis_err(" RF Init Fail");
        while (1)
            ;
    }

    rf_write_spec_page_reg(PAGE3_SEL, 0x1d, 0x50);
    rf_set_default_para();
    now_bw = TEST_BW;
    now_sf = TEST_SF;
    rf_set_bw(now_bw);
    rf_set_sf(now_sf);
    rf_set_code_rate(TEST_CR);
    rf_set_freq(TEST_FREQ);

    /*Users set syncword  based on actual situations*/
    syncword_update(now_sf);

    memcpy((uint8_t *)&cur_mapm_cfg, (uint8_t *)&mapm_cfg[now_sf - SF_7], sizeof(stc_mapm_cfg_t)); // get cuurrent mapm config ,first mapm_cfg is SF5 BW500K mapm config
    // enable mapm mode
    rf_set_mapm_on();
    // configure relevant parameters used in mapm mode
    rf_set_mapm_cfg(&cur_mapm_cfg);
    // configure mapm addr
    for (uint8_t i = 0; i < cur_mapm_cfg.gn + 1; i++)
    {
        rf_set_mapm_addr(i, cur_mapm_cfg.mapm_addr[i]);
    }

    one_chirp_time = rf_get_chirp_time(now_bw, now_sf); // us
    // Calculate the duration of the package, add 200ms as a receive timeout judgment time
    uint32_t preamble_time = rf_calculate_mapm_preambletime(&cur_mapm_cfg, one_chirp_time);
    rx_timeout_val = preamble_time + calculate_payload_time(tx_len) + 200;
    sleep_timer_val = preamble_time / 2;
    show_menu();
    dis_init();
    OLED_Refresh_Gram();

    if (app_tx_mode)
    {
        DDL_Printf("tx\r\n");
    }
    else
    {
        DDL_Printf("rx\r\n");
#ifndef ADDR_MATCH
        if (now_sf == SF_5)
        {
            cur_mapm_cfg.mapm_addr[1] = 0x21;
            rf_set_mapm_addr(1, cur_mapm_cfg.mapm_addr[1]);
        }
        else
        {
            cur_mapm_cfg.mapm_addr[1] = 0x61;
            rf_set_mapm_addr(1, cur_mapm_cfg.mapm_addr[1]);
        }

#endif
        rf_cad_on(CAD_DETECT_THRESHOLD_10, CAD_DETECT_NUMBER_3);
        rf_set_dcdc_mode(DCDC_ON);
        rf_enter_single_timeout_rx(rx_timeout_val);
    }

    if (one_chirp_time < 100)
    {
        cad_check_time = 1 + 1;
    }
    else
    {
        cad_check_time = 10 * one_chirp_time / 1000 + 1;
    }

    while (1)
    {
        /*if BW SF changes update chirp_time*/
        sf_bw_change_update();

        if (TX_RX_SELECT == RX_SELECT)
        {
            /*Hibernation timer has arrived*/
            if (timera_wake_flag == 1)
            {
                timera_wake_flag = 0;
                sleep_flag = 0;
                rf_sleep_wakeup();
                rf_set_dcdc_mode(DCDC_ON);
                rf_enter_single_timeout_rx(rx_timeout_val);
            }

            if (sleep_flag == 0)
            {
                if (!cad_flag)
                {
                    if (!recv_flag) // now not in data receiving ,check CAD
                    {
                        if (cad_judge_flag == 0) // CAD detection has not started yet
                        {
                            cad_judge_begtime = SysTick_GetTick();
                            cad_judge_flag = 1;
                        }
                        else
                        {
                            // if CAD is not detected for 10 chirp times,sleep
                            if ((SysTick_GetTick() - cad_judge_begtime) > cad_check_time)
                            {
                                cad_judge_flag = 0;
                                sleep_flag = 1;
                                rf_sleep();
                                rf_set_dcdc_mode(DCDC_OFF);
                                timera_open_ms(sleep_timer_val);
                            }
                        }
                    }
                }
                else
                {
                    recv_flag = 1;
                    cad_flag = 0;
                    cad_judge_flag = 0;
                }
            }
        }

        rf_irq_process();
        key_scan();
        key_event_process();
        process_rf_events();
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
