/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file radio.c
 * @brief
 *
 * @history - V0.5, 2023-12-22
*******************************************************************************/
#include "oled.h"
#include "pan_rf.h"
#include "display.h"

/* change oled display mode : 0 for complex ; 1 for Simple */
#define EXTERNAL  0

/* user can change frequency list, FREQ_LIST_NUM defined in display.h*/
uint32_t freq_list[FREQ_LIST_NUM] = {9150,4330,8680};

extern uint32_t app_tx_mode;
static char mode[3] = {'A','B','C'};
static char gain[2] = {'-','+'};
static char* bw[9] = {"0","0","0","0","0","62.5","125 ","250 ","500 "};
char* cr[5] = {"0","4/5","4/6","4/7","4/8"};

void show_menu(void)
{
    OLED_ShowString(0,0,"PW:",16);
    OLED_ShowString(0,16,"CR:",16);
    OLED_ShowString(0,32,"BW:",16);
    OLED_ShowString(0,48,"PL:",16);
    OLED_ShowString(64,0,"MODE:",16);
    OLED_ShowString(64,16,"FQ:",16);
    OLED_ShowString(64,32,"SF:",16);

    if(app_tx_mode)
    {
        OLED_ShowString(64,48,"TX:",16);
    }
    else
    {
        OLED_ShowString(64,48,"RX:",16);
    }
    OLED_Refresh_Gram();
}

void dis_err(char *str)
{
    OLED_Clear();
    OLED_ShowString(0, 22, str,16);
    OLED_Refresh_Gram();
}

void dis_set_sf(uint32_t val)
{
    OLED_ShowNum(85,32,val,4,16);
}

void dis_set_bw(uint32_t val)
{
    OLED_ShowString(30,32,bw[val - 1],16);
}

void dis_set_pl(uint32_t val)
{
    OLED_ShowNum(20,48,val,4,16);
}

void dis_set_mode(uint32_t val)
{
    OLED_ShowChar(103,0,mode[val],16,1);
}

void dis_set_gain(uint32_t val)
{
    OLED_ShowChar(110,0,gain[val],16,1);
}

void dis_set_cr(uint32_t val)
{
    OLED_ShowString(30,16,cr[val],16);
}

void dis_set_fq(uint32_t val)
{
    OLED_ShowNum(85,16,val,4,16);
}

void dis_set_cnt(uint32_t val)
{
    OLED_ShowNum(85,48,val,4,16);
}

void dis_set_txpower(uint32_t val)
{
    OLED_ShowNum(22,0,val,4,16);
}

void dis_set_carrier_power(uint32_t val)
{
    OLED_ShowString(32,48,"POWER:",16);
    OLED_ShowNum(85,48,val,4,16);
}

void refresh_frame(void)
{
    OLED_Refresh_Gram();
}





