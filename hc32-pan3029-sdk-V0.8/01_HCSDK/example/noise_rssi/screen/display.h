/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file radio.c
 * @brief
 *
 * @history - V0.8, 2024-4
*******************************************************************************/
#ifndef __DISPLAY_H_
#define __DISPLAY_H_
#include "stdint.h"
#include "oled.h"

#define FREQ_LIST_NUM  3

void show_menu(void);
void dis_err(char *str);
void dis_set_sf(uint32_t val);
void dis_set_bw(uint32_t val);
void dis_set_pl(uint32_t val);
void dis_set_mode(uint32_t val);
void dis_set_ldr(uint32_t val);
void dis_set_cnt(uint32_t val);
void dis_set_fq(uint32_t val);
void dis_set_cr(uint32_t val);
void dis_set_txpower(uint32_t val);
void dis_set_carrier_power(uint32_t val);
void refresh_frame(void);
#endif

