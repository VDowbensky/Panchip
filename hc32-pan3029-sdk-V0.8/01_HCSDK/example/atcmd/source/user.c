/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file user.c
 * @brief
 *
 * @history - V0.8, 2024-4
*******************************************************************************/
#include "pan_rf.h"
#include "at.h"
#include "user.h"

rfConfig_t gRfConfig = {DEFAULT_FREQ,DEFAULT_CR,DEFAULT_BW,DEFAULT_SF,\
                        RF_MAX_RAMP,CRC_ON,LDR_OFF,MODEM_MODE_NORMAL,8,DCDC_OFF};

void rf_SetUserPara(rfConfig_t *p)
{
	rf_set_mode(RF_MODE_STB3);

	rf_set_freq(p->freq);
	rf_set_code_rate(p->cr);
	rf_set_bw(p->bw);
	rf_set_sf(p->sf);
	rf_set_tx_power(p->pwr);
	rf_set_crc(p->crc);
	rf_set_ldr(p->ldr);
	rf_set_preamble(p->preamble);
	rf_set_modem_mode(p->modemMode);
	rf_set_dcdc_mode(p->dcdc);
}

