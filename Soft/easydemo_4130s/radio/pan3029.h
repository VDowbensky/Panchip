/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file pan3029.h
 * @brief
 *
 * @history - V0.7, 2024-3
*******************************************************************************/
#ifndef __PAN_3029_H
#define __PAN_3029_H

#include "pan3029_port.h"

/* result */
#define OK                              0
#define FAIL                            1
/* 3029 mode define*/
#define PAN3029_MODE_DEEP_SLEEP         0
#define PAN3029_MODE_SLEEP              1
#define PAN3029_MODE_STB1               2
#define PAN3029_MODE_STB2               3
#define PAN3029_MODE_STB3               4
#define PAN3029_MODE_TX                 5
#define PAN3029_MODE_RX                 6
/* 3029 Tx mode */
#define PAN3029_TX_SINGLE               0
#define PAN3029_TX_CONTINOUS            1
/* 3029 Rx mode */
#define PAN3029_RX_SINGLE               0
#define PAN3029_RX_SINGLE_TIMEOUT       1
#define PAN3029_RX_CONTINOUS            2
/* 3029 power maximum ramp */
#define PAN3029_MAX_RAMP                22
#define PAN3029_MIN_RAMP                1
/* System control register */
#define REG_SYS_CTL                     0x00
#define REG_FIFO_ACC_ADDR               0x01
/* 3V Logical area register */
#define REG_OP_MODE                     0x02
/* dcdc calibration select */
#define CALIBR_REF_CMP  0x01
#define CALIBR_ZERO_CMP 0x02
#define CALIBR_IMAX_CMP 0x03

#define MODEM_MODE_NORMAL               0x01
#define MODEM_MODE_MULTI_SECTOR         0x02

#define MODEM_MPA                       0x01
#define MODEM_MPB                       0x02

#define freq_360000000                  (360000000)
#define freq_370000000                  (370000000)
#define freq_385000000                  (385000000)
#define freq_405000000                  (405000000)
#define freq_415000000                  (415000000)
#define freq_430000000                  (430000000)
#define freq_445000000                  (445000000)
#define freq_465000000                  (465000000)
#define freq_485000000                  (485000000)
#define freq_495000000                  (495000000)
#define freq_505000000                  (505000000)
#define freq_530000000                  (530000000)
#define freq_565000000                  (565000000)
#define freq_600000000                  (600000000)

#define freq_720000000                  (720000000)
#define freq_740000000                  (740000000)
#define freq_770000000                  (770000000)
#define freq_810000000                  (810000000)
#define freq_830000000                  (830000000)
#define freq_860000000                  (860000000)
#define freq_890000000                  (890000000)
#define freq_930000000                  (930000000)
#define freq_970000000                  (970000000)
#define freq_1010000000                 (1010000000)
#define freq_1060000000                 (1060000000)
#define freq_1080000000                 (1080000000)

#define CODE_RATE_45                     0x01
#define CODE_RATE_46                     0x02
#define CODE_RATE_47                     0x03
#define CODE_RATE_48                     0x04

#define SF_5                            5
#define SF_6                            6
#define SF_7                            7
#define SF_8                            8
#define SF_9                            9
#define SF_10                           10
#define SF_11                           11
#define SF_12                           12

#define BW_62_5K                        6
#define BW_125K                         7
#define BW_250K                         8
#define BW_500K                         9

#define CRC_OFF                         0
#define CRC_ON                          1

#define PLHD_IRQ_OFF                    0
#define PLHD_IRQ_ON                     1

#define PLHD_OFF                        0
#define PLHD_ON                         1

#define PLHD_LEN8                       0
#define PLHD_LEN16                      1

#define AGC_ON                          1
#define AGC_OFF                         0

#define LO_400M                         0
#define LO_800M                         1

#define DCDC_OFF                        0
#define DCDC_ON                         1

#define LDR_OFF                         0
#define LDR_ON                          1

#define MAPM_OFF                        1
#define MAPM_ON                         0

#define CAD_DETECT_THRESHOLD_0A         0x0A
#define CAD_DETECT_THRESHOLD_10         0x10
#define CAD_DETECT_THRESHOLD_15         0x15
#define CAD_DETECT_THRESHOLD_20         0x20

#define CAD_DETECT_NUMBER_1             0x01
#define CAD_DETECT_NUMBER_2             0x02
#define CAD_DETECT_NUMBER_3             0x03

#define REG_PAYLOAD_LEN                 0x0C

/*IRQ BIT MASK*/
#define REG_IRQ_MAPM_DONE               0x40
#define REG_IRQ_RX_PLHD_DONE            0x10
#define REG_IRQ_RX_DONE                 0x8
#define REG_IRQ_CRC_ERR                 0x4
#define REG_IRQ_RX_TIMEOUT              0x2
#define REG_IRQ_TX_DONE                 0x1

enum REF_CLK_SEL {REF_CLK_32M,REF_CLK_16M};
enum PAGE_SEL {PAGE0_SEL,PAGE1_SEL,PAGE2_SEL, PAGE3_SEL};
enum MAPM_LASTADDR_FUNC {ORDINARY_ADDR,FIELD_COUNTER};

typedef struct
{
    uint8_t mapm_addr[4];
    uint8_t fn;
    uint8_t fnm;
    uint8_t gfs;
    uint8_t gn;
    uint8_t pg1;
    uint8_t pgn;
    uint16_t pn;
} stc_mapm_cfg_t;

#pragma pack(1)
typedef struct
{
    uint8_t  ramp;
    uint8_t  pa_trim;
    uint8_t  pa_ldo;
    uint8_t  pa_duty;
} power_ramp_t;
#pragma pack ()

#pragma pack(1)
typedef struct
{
    uint8_t  ramp;
    uint8_t  pa_ldo;
    uint8_t  pa_duty;
} power_ramp_cfg_t;
#pragma pack ()

uint8_t PAN3029_read_reg(uint8_t addr);
uint8_t PAN3029_write_reg(uint8_t addr,uint8_t value);
void PAN3029_write_fifo(uint8_t addr,uint8_t *buffer,int size);
void PAN3029_read_fifo(uint8_t addr,uint8_t *buffer,int size);
uint32_t PAN3029_switch_page(enum PAGE_SEL page);
uint32_t PAN3029_write_spec_page_reg(enum PAGE_SEL page,uint8_t addr,uint8_t value);
uint8_t PAN3029_read_spec_page_reg(enum PAGE_SEL page,uint8_t addr);
uint32_t PAN3029_write_read_continue_regs(enum PAGE_SEL page,uint8_t addr,uint8_t *buffer,uint8_t len);
uint8_t PAN3029_clr_irq(void);
uint8_t PAN3029_get_irq(void);
uint32_t PAN3029_refresh(void);
uint16_t PAN3029_read_pkt_cnt(void);
void PAN3029_clr_pkt_cnt(void);
uint32_t PAN3029_agc_enable(uint32_t state);
uint32_t PAN3029_agc_config(void);
uint32_t PAN3029_ft_calibr(void);
uint32_t PAN3029_init(void);
uint32_t PAN3029_deepsleep_wakeup(void);
uint32_t PAN3029_sleep_wakeup(void);
uint32_t PAN3029_deepsleep(void);
uint32_t PAN3029_sleep(void);
uint32_t PAN3029_set_lo_freq(uint32_t lo);
uint32_t PAN3029_set_freq(uint32_t freq);
uint32_t PAN3029_read_freq(void);
uint32_t PAN3029_calculate_tx_time(uint8_t size);
uint32_t PAN3029_set_bw(uint32_t bw_val);
uint8_t PAN3029_get_bw(void);
uint32_t PAN3029_set_sf(uint32_t sf_val);
uint8_t PAN3029_get_sf(void);
uint32_t PAN3029_set_crc(uint32_t crc_val);
uint8_t PAN3029_get_crc(void);
uint32_t PAN3029_set_code_rate(uint8_t code_rate);
uint8_t PAN3029_get_code_rate(void);
uint32_t PAN3029_set_mode(uint8_t mode);
uint8_t PAN3029_get_mode(void);
uint32_t PAN3029_set_tx_mode(uint8_t mode);
uint32_t PAN3029_set_rx_mode(uint8_t mode);
uint32_t PAN3029_set_modem_mode(uint8_t mode);
uint32_t PAN3029_set_timeout(uint32_t timeout);
float PAN3029_get_snr(void);
int8_t PAN3029_get_rssi(void);
int8_t PAN3029_get_channel_rssi(void);
uint32_t PAN3029_set_tx_power(uint8_t tx_power);
uint32_t PAN3029_get_tx_power(void);
uint32_t PAN3029_set_preamble(uint16_t reg);
uint32_t PAN3029_get_preamble(void);
uint32_t PAN3029_set_gpio_input(uint8_t gpio_pin);
uint32_t PAN3029_set_gpio_output(uint8_t gpio_pin);
uint32_t PAN3029_set_gpio_state(uint8_t gpio_pin, uint8_t state);
uint8_t PAN3029_get_gpio_state(uint8_t gpio_pin);
uint32_t PAN3029_cad_en(uint8_t threshold, uint8_t chirps);
uint32_t PAN3029_cad_off(void);
uint32_t PAN3029_set_syncword(uint32_t sync);
uint8_t PAN3029_get_syncword(void);
uint32_t PAN3029_send_packet(uint8_t *buff, uint32_t len);
uint8_t PAN3029_recv_packet(uint8_t *buff);
uint32_t PAN3029_set_early_irq(uint32_t earlyirq_val);
uint8_t PAN3029_get_early_irq(void);
uint32_t PAN3029_set_plhd(uint8_t addr,uint8_t len);
uint8_t PAN3029_get_plhd_len(void);
uint32_t PAN3029_set_plhd_mask(uint32_t plhd_val);
uint8_t PAN3029_get_plhd_mask(void);
uint8_t PAN3029_recv_plhd8(uint8_t *buff);
uint8_t PAN3029_recv_plhd16(uint8_t *buff);
uint32_t PAN3029_plhd_receive(uint8_t *buf,uint8_t len);
uint32_t PAN3029_set_dcdc_mode(uint32_t dcdc_val);
uint32_t PAN3029_set_ldr(uint32_t mode);
uint32_t PAN3029_get_ldr(void);
uint32_t PAN3029_set_all_sf_preamble(uint32_t sf);
uint32_t PAN3029_set_all_sf_search(void);
uint32_t PAN3029_set_all_sf_search_off(void);
void PAN3029_irq_handler(void);
uint32_t PAN3029_set_carrier_wave_on(void);
uint32_t PAN3029_set_carrier_wave_freq(uint32_t freq);
uint32_t PAN3029_set_carrier_wave_off(void);
uint32_t carrier_wave_test_mode(void);
uint32_t PAN3029_mapm_en(void);
uint32_t PAN3029_mapm_dis(void);
uint32_t PAN3029_set_mapm_mask(uint32_t mapm_val);
uint32_t PAN3029_set_mapm_para(uint8_t field_num_mux, uint8_t group_fun_sel, uint8_t gn, uint8_t pgn);
uint32_t PAN3029_set_mapm_addr(uint8_t addr_no, uint8_t addr);
uint32_t PAN3029_calculate_mapm_preambletime(stc_mapm_cfg_t *mapm_cfg, uint32_t one_chirp_time);
uint32_t PAN3029_get_mapm_field_num(void);
uint32_t PAN3029_set_mapm_field_num(uint8_t);
uint32_t PAN3029_set_mapm_field_num_mux(uint8_t fnm);
uint32_t PAN3029_set_mapm_group_fun_sel(uint8_t gfs);
uint32_t PAN3029_set_mapm_group_num(uint8_t gn);
uint32_t PAN3029_set_mapm_firgroup_preamble_num(uint8_t pgl);
uint32_t PAN3029_set_mapm_group_preamble_num(uint8_t pgn);
uint32_t PAN3029_set_mapm_neces_preamble_num(uint16_t pn);
uint32_t PAN3029_efuse_on(void);
uint32_t PAN3029_efuse_off(void);
uint8_t PAN3029_efuse_read_encry_byte(uint8_t reg_addr, uint16_t pattern, uint8_t efuse_addr);
uint8_t PAN3029_efuse_read_byte(uint8_t reg_addr, uint8_t efuse_addr);
void PAN3029_efuse_write_encry_byte(uint8_t reg_addr, uint16_t pattern, uint8_t efuse_addr, uint8_t value);
void PAN3029_efuse_write_byte(uint8_t reg_addr, uint8_t efuse_addr, uint8_t value);
uint32_t PAN3029_set_dcdc_calibr_on(uint8_t calibr_type);
uint32_t PAN3029_set_dcdc_calibr_off(void);
uint32_t PAN3029_set_ldo_pa_on(void);
uint32_t PAN3029_set_ldo_pa_off(void);
#endif
