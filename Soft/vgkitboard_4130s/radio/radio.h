/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file radio.h
 * @brief
 *
 * @history - V0.7, 2024-3
*******************************************************************************/
#ifndef __RADIO_H_
#define __RADIO_H_

#include "pan3029_port.h"
#include "pan3029.h"
#define DEFAULT_PWR            22
#define DEFAULT_FREQ           (433920000)
#define DEFAULT_SF             SF_9
#define DEFAULT_BW             BW_125K
#define DEFAULT_CR             CODE_RATE_47

#define RADIO_FLAG_IDLE         0
#define RADIO_FLAG_TXDONE       1
#define RADIO_FLAG_RXDONE       2
#define RADIO_FLAG_RXTIMEOUT    3
#define RADIO_FLAG_RXERR        4
#define RADIO_FLAG_PLHDRXDONE   5
#define RADIO_FLAG_MAPM         6


#define LEVEL_INACTIVE          6
#define LEVEL_ACTIVE            7

#define	GPIO_PIN_CAD            Pin09
#define	GPIO_PORT_CAD           PortA

#define  CHECK_CAD()            0//PORT_GetBit(GPIO_PORT_CAD, GPIO_PIN_CAD)
#define  SET_TIMER_MS(time)     //timer6_open_ms(time)

#define MAC_EVT_TX_CAD_NONE     0x00
#define MAC_EVT_TX_CAD_TIMEOUT  0x01
#define MAC_EVT_TX_CAD_ACTIVE   0x02

#define TEST_MODE_BUFFER_LEN   10

#pragma pack(1)
struct RxDoneMsg
{
    uint8_t Payload[255];
    uint8_t PlhdPayload[16];
	uint8_t TestModePayload[TEST_MODE_BUFFER_LEN];
    uint16_t PlhdSize;
    uint16_t Size;
    uint16_t mpam_recv_index;
    uint8_t mpam_recv_buf[1024];  //set buf size based on actual application
    int8_t Rssi;
    float Snr;
};
#pragma pack ()

typedef enum {
    RF_PARA_TYPE_FREQ,
    RF_PARA_TYPE_CR,
    RF_PARA_TYPE_BW,
    RF_PARA_TYPE_SF,
    RF_PARA_TYPE_TXPOWER,
    RF_PARA_TYPE_LDRO,
    RF_PARA_TYPE_CRC,
} rf_para_type_t;

uint32_t rf_get_recv_flag(void);
void rf_set_recv_flag(int status);
uint32_t rf_get_transmit_flag(void);
void rf_set_transmit_flag(int status);
uint32_t rf_init(void);
uint32_t rf_deepsleep_wakeup(void);
uint32_t rf_deepsleep(void);
uint32_t rf_sleep_wakeup(void);
uint32_t rf_sleep(void);
uint32_t rf_get_tx_time(uint8_t size);
uint32_t rf_set_mode(uint8_t mode);
uint8_t rf_get_mode(void);
uint32_t rf_set_tx_mode(uint8_t mode);
uint32_t rf_set_rx_mode(uint8_t mode);
uint32_t rf_set_rx_single_timeout(uint32_t timeout);
float rf_get_snr(void);
int8_t rf_get_rssi(void);
int8_t rf_get_channel_rssi(void);
uint8_t rf_get_irq(void);
uint8_t rf_clr_irq(void);
uint32_t rf_set_refresh(void);
uint32_t rf_set_preamble(uint16_t pream);
uint32_t rf_get_preamble(void);
uint32_t rf_set_cad(uint8_t threshold, uint8_t chirps);
uint32_t rf_set_cad_off(void);
uint32_t rf_set_syncword(uint8_t sync);
uint8_t rf_get_syncword(void);
void rf_irq_handler(void);
void rf_set_plhd_rx_on(uint8_t addr,uint8_t len);
void rf_set_plhd_rx_off(void);
uint8_t rf_get_plhd_len(void);
uint32_t rf_receive(uint8_t *buf);
uint32_t rf_plhd_receive(uint8_t *buf,uint8_t len);
uint32_t rf_enter_continous_rx(void);
uint32_t rf_enter_single_timeout_rx(uint32_t timeout);
uint32_t rf_enter_single_rx(void);
uint32_t rf_single_tx_data(uint8_t *buf, uint8_t size, uint32_t *tx_time);
uint32_t rf_enter_continous_tx(void);
uint32_t rf_continous_tx_send_data(uint8_t *buf, uint8_t size);
uint32_t rf_set_agc(uint32_t state);
uint32_t rf_set_para(rf_para_type_t para_type, uint32_t para_val);
uint32_t rf_get_para(rf_para_type_t para_type, uint32_t *para_val);
void rf_set_default_para(void);
uint32_t rf_set_modem_mode(uint8_t mode);
uint32_t rf_set_dcdc_mode(uint32_t dcdc_val);
uint32_t rf_set_ldr(uint32_t mode);
uint32_t rf_get_ldr(void);
uint32_t rf_set_all_sf_preamble(uint32_t sf);
uint32_t rf_set_all_sf_search(void);
uint32_t rf_set_all_sf_search_off(void);
void rf_irq_process(void);
uint32_t get_chirp_time(uint32_t bw,uint32_t sf);
uint32_t check_cad_rx_inactive(uint32_t one_chirp_time);
uint32_t check_cad_tx_inactive(void);
void rf_set_mapm_on(void);
void rf_set_mapm_off(void);
void rf_set_mapm_para(stc_mapm_cfg_t *p_mapm_cfg);
uint32_t rf_set_dcdc_calibr_on(void);
#endif

