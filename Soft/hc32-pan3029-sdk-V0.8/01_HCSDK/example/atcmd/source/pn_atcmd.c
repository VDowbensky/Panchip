/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file pn_atcmd.c
 * @brief
 *
 * @history - V0.8, 2024-4
*******************************************************************************/
#include "user.h"
#include "pan_port.h"
#include "pan_rf.h"
#include "at.h"
#include <stdlib.h>
#include "at_log.h"
#include "hc32_ddl.h"
#include "ev_hc32f460_lqfp100_v2.h"
#include "hc32f460_utility.h"

extern rfConfig_t gRfConfig;
uint8_t tx_carrywave_flag = 0;

int userBreakCheck(void)
{
	char ch;
	int len;

	do
	{
		len = uart_fifo_read((u8*)&ch, 1);
		if ( len > 0 )//input ctrl+c will break rx task
		{
			if ( ch == 3  /*ctrl+c*/ 
				 || ch == 'c' 
				 || ch == 'C' )
			{
				uart_rx_fifo_flush();
				return 1;
			}
		}
		else
		{
			break;
		}
	}while ( 1 );

	return 0;
}

static at_result_t at_rf_init(void)
{
	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+RF_INIT", PN_NULL, PN_NULL, PN_NULL, PN_NULL, at_rf_init);

static at_result_t rf_enter_deepsleep(void)
{
	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_deepsleep();

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+DEEPSLEEPMODE", PN_NULL, PN_NULL, PN_NULL, PN_NULL, rf_enter_deepsleep);

static at_result_t rf_enter_sleep(void)
{
	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_sleep();

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+SLEEPMODE", PN_NULL, PN_NULL, PN_NULL, PN_NULL, rf_enter_sleep);

static at_result_t rf_enter_stb1(void)
{
	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_set_mode(RF_MODE_STB3);
	rf_port.delayms(2);
	rf_set_mode(RF_MODE_STB2);
	rf_port.delayus(10);
	rf_set_mode(RF_MODE_STB1);

	rf_port.tcxo_close();
	rf_antenna_init();

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+STB1MODE", PN_NULL, PN_NULL, PN_NULL, PN_NULL, rf_enter_stb1);

static at_result_t rf_enter_stb2(void)
{
	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_set_mode(RF_MODE_STB3);
	rf_port.delayms(2);
	rf_set_mode(RF_MODE_STB2);

	rf_port.tcxo_close();
	rf_antenna_init();

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+STB2MODE", PN_NULL, PN_NULL, PN_NULL, PN_NULL, rf_enter_stb2);

static at_result_t rf_enter_stb3(void)
{
	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_set_mode(RF_MODE_STB3);
	rf_port.tcxo_close();
	rf_antenna_init();

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+STB3MODE", PN_NULL, PN_NULL, PN_NULL, PN_NULL, rf_enter_stb3);

static at_result_t rf_getPara(void)
{
	rfConfig_t *p = &gRfConfig;

	at_server_printfln("+RF_PARA=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", \
					   p->freq, p->cr,p->bw,p->sf,p->pwr,   \
					   p->crc,p->ldr,p->modemMode,p->preamble,\
					   p->dcdc);

	return AT_RESULT_OK;
}


static at_result_t rf_setPara(const char *args)
{
	int freq, cr, bw, sf, txpower, crc, lowdron, modemMode, preamble, dcdc, argc;
	const char *req_expr = "=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";

	argc = at_req_parse_args(args, req_expr, &freq, &cr, &bw, &sf,
							 &txpower, &crc, &lowdron, &modemMode,
							 &preamble, &dcdc);
	if (argc != 10)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	pcDebug("freq:%u", freq);
	pcDebug("cr:%u", cr);
	pcDebug("bw:%u", bw);
	pcDebug("sf:%u", sf);
	pcDebug("txpower:%u", txpower);
	pcDebug("crc:%u", crc);
	pcDebug("ldr:%u", lowdron);
	pcDebug("modemMode:%u", modemMode);
	pcDebug("preamble:%u", preamble);
	pcDebug("dcdc:%u", dcdc);

	gRfConfig.freq = freq;
	gRfConfig.cr = cr;
	gRfConfig.bw = bw;
	gRfConfig.sf = sf;
	gRfConfig.pwr = txpower;
	gRfConfig.crc = crc;
	gRfConfig.ldr = lowdron;
	gRfConfig.modemMode = modemMode;
	gRfConfig.preamble = preamble;
	gRfConfig.dcdc = dcdc;

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+RF_PARA", "=<freq>,<cr>,<bw>,<sf>,<txp>,<crc>,<ldr>,<modemMode>,<preamble>,<dcdc>", PN_NULL, rf_getPara, rf_setPara, PN_NULL);


static at_result_t rf_getFreq(void)
{
	rfConfig_t *p = &gRfConfig;

	at_server_printfln("+FREQ=%d", p->freq);

	return AT_RESULT_OK;
}

static at_result_t rf_setFreq(const char *args)
{
	int freq, argc;
	const char *req_expr = "=%d";

	argc = at_req_parse_args(args, req_expr, &freq);
	if (argc != 1)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	atDebug("freq:%u", freq);
	gRfConfig.freq = freq;

	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+FREQ", "=<freq[(Hz)]>", PN_NULL, rf_getFreq, rf_setFreq, PN_NULL);

static at_result_t rf_getCr(void)
{
	rfConfig_t *p = &gRfConfig;

	at_server_printfln("+CR=%d", p->cr);

	return AT_RESULT_OK;
}

static at_result_t rf_setCr(const char *args)
{
	int cr, argc;
	const char *req_expr = "=%d";

	argc = at_req_parse_args(args, req_expr, &cr);
	if (argc != 1)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	atDebug("cr:%u", cr);
	gRfConfig.cr = cr;

	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+CR", "=<CR[1-4]>", PN_NULL, rf_getCr, rf_setCr, PN_NULL);

static at_result_t rf_getBw(void)
{
	rfConfig_t *p = &gRfConfig;

	at_server_printfln("+BW=%d", p->bw);

	return AT_RESULT_OK;
}

static at_result_t rf_setBw(const char *args)
{
	int bw, argc;
	const char *req_expr = "=%d";

	argc = at_req_parse_args(args, req_expr, &bw);
	if (argc != 1)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	atDebug("bw:%u", bw);
	gRfConfig.bw = bw;

	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+BW", "=<BW[6(62.5K)-9(500K)]>", PN_NULL, rf_getBw, rf_setBw, PN_NULL);

static at_result_t rf_getSf(void)
{
	rfConfig_t *p = &gRfConfig;

	at_server_printfln("+SF=%d", p->sf);

	return AT_RESULT_OK;
}

static at_result_t rf_setSf(const char *args)
{
	int sf, argc;
	const char *req_expr = "=%d";

	argc = at_req_parse_args(args, req_expr, &sf);
	if (argc != 1)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	atDebug("sf:%u", sf);
	gRfConfig.sf = sf;

	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+SF", "=<SF[5-12]>", PN_NULL, rf_getSf, rf_setSf, PN_NULL);

static at_result_t rf_getPwr(void)
{
	rfConfig_t *p = &gRfConfig;

	at_server_printfln("+PWR=%d", p->pwr);

	return AT_RESULT_OK;
}

static at_result_t rf_setPwr(const char *args)
{
	int pwr, argc;
	const char *req_expr = "=%d";

	argc = at_req_parse_args(args, req_expr, &pwr);
	if (argc != 1)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	atDebug("pwr:%u", pwr);
	gRfConfig.pwr = pwr;

	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+PWR", "=<txpower[1-23]>", PN_NULL, rf_getPwr, rf_setPwr, PN_NULL);

static at_result_t rf_getCrc(void)
{
	rfConfig_t *p = &gRfConfig;

	at_server_printfln("+CRC=%d", p->crc);

	return AT_RESULT_OK;
}

static at_result_t rf_setCrc(const char *args)
{
	int crc, argc;
	const char *req_expr = "=%d";

	argc = at_req_parse_args(args, req_expr, &crc);
	if (argc != 1)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	atDebug("crc:%u", crc);
	gRfConfig.crc = crc>0?1:0;

	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+CRC", "=<crc[0/1]>", PN_NULL, rf_getCrc, rf_setCrc, PN_NULL);

static at_result_t rf_getLdr(void)
{
	rfConfig_t *p = &gRfConfig;

	at_server_printfln("+LDR=%d", p->ldr);

	return AT_RESULT_OK;
}

static at_result_t rf_setLdr(const char *args)
{
	int ldr, argc;
	const char *req_expr = "=%d";

	argc = at_req_parse_args(args, req_expr, &ldr);
	if (argc != 1)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	atDebug("ldr:%u", ldr);
	gRfConfig.ldr = ldr>0?1:0;

	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+LDR", "=<low data rate[0/1]>", PN_NULL, rf_getLdr, rf_setLdr, PN_NULL);

static at_result_t rf_getModemMode(void)
{
	rfConfig_t *p = &gRfConfig;

	at_server_printfln("+MODEMMODE=%d", p->modemMode);

	return AT_RESULT_OK;
}

static at_result_t rf_setModemMode(const char *args)
{
	int modemMode, argc;
	const char *req_expr = "=%d";

	argc = at_req_parse_args(args, req_expr, &modemMode);
	if (argc != 1)
	{
	return AT_RESULT_PARSE_FAILE;
	}

	if ( modemMode != MODEM_MODE_NORMAL
	 && modemMode != MODEM_MODE_MULTI_SECTOR)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	atDebug("modemMode:%u", modemMode);
	gRfConfig.modemMode = modemMode;

	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+MODEMMODE", "=<MODEMMODE[1/2]>", PN_NULL, rf_getModemMode, rf_setModemMode, PN_NULL);

static at_result_t rf_getPreamLen(void)
{
	rfConfig_t *p = &gRfConfig;

	at_server_printfln("+PREAMLEN=%d", p->preamble);

	return AT_RESULT_OK;
}

static at_result_t rf_setPreamLen(const char *args)
{
	int preamble, argc;
	const char *req_expr = "=%d";

	argc = at_req_parse_args(args, req_expr, &preamble);
	if (argc != 1)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	atDebug("preamble:%u", preamble);

	gRfConfig.preamble = preamble;

	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+PREAMLEN", "=<preamble len[8-65535]>", PN_NULL, rf_getPreamLen, rf_setPreamLen, PN_NULL);

static at_result_t rf_getDCDC(void)
{
	rfConfig_t *p = &gRfConfig;

	at_server_printfln("+DCDC=%d", p->dcdc);

	return AT_RESULT_OK;
}

static at_result_t rf_setDCDC(const char *args)
{
	int dcdc, argc;
	const char *req_expr = "=%d";

	argc = at_req_parse_args(args, req_expr, &dcdc);
	if (argc != 1)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	atDebug("dcdc:%u", dcdc);
	gRfConfig.dcdc = dcdc>0?1:0;

	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+DCDC", "=<dcdc[0/1]>", PN_NULL, rf_getDCDC, rf_setDCDC, PN_NULL);

static at_result_t rf_setCarryWave(const char *args)
{
	int on, argc;
	const char *req_expr = "=%d";

	argc = at_req_parse_args(args, req_expr, &on);
	if (argc != 1)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	atDebug("set carry wave:%u", on);

	if ( on )
	{
		int ret = rf_init();
		if(ret != OK)
		{
			atDebug("rf init err");
			return AT_RESULT_FAILE;
		}
		else
		{
			atDebug("rf init ok.");
		}			

		ret = rf_set_carrier_wave_on();
		if(ret != OK)
		{
			atDebug("rf_set_carrier_wave_test_mode err");
			return AT_RESULT_FAILE;
		}

		rf_set_freq(gRfConfig.freq);
		rf_set_tx_power(gRfConfig.pwr);
		rf_set_carrier_wave_freq(gRfConfig.freq);
		tx_carrywave_flag = 1;
	}else
	{
		rf_set_carrier_wave_off();
		rf_write_reg(REG_OP_MODE,RF_MODE_STB3);
		rf_antenna_close();
		tx_carrywave_flag = 0;
	}

	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+RF_CARRYWAVE", "=<mode[0/1]>", PN_NULL, PN_NULL, rf_setCarryWave, PN_NULL);

static at_result_t rf_setRxMode(const char *args)
{
	uint64_t tick;
	int recvCount = 0;
	int timeout_ms, argc;
	const char *req_expr = "=%d";

	atDebug("rf enter rx mode");
	atDebug("recv \'C\' or \'c\' exit rx mode");

	argc = at_req_parse_args(args, req_expr, &timeout_ms);
	if (argc != 1)
	{
		return AT_RESULT_PARSE_FAILE;
	}

	if ( timeout_ms < 1000 )
	{
		timeout_ms = 1000;
	}
	if ( timeout_ms > 1000000 )
	{
		timeout_ms = 1000000;
	}
	
	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init err");
		return AT_RESULT_FAILE;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);

	ret = rf_enter_continous_rx();
	if(ret != OK)
	{
		atDebug("rf_enter_continous_rx err");
		return AT_RESULT_FAILE;
	}

	tick = SysTick_usTick();

	while (1)
	{
		rf_irq_process();
		if (rf_get_recv_flag() == RADIO_FLAG_RXDONE)
		{
			extern struct RxDoneMsg RxDoneParams;

			rf_set_recv_flag(RADIO_FLAG_IDLE);
			atDebug("+count=%d\r\n", ++recvCount);
			atDebug("+RSSI=%d\r\n", RxDoneParams.Rssi);

			tick = SysTick_usTick();
		}

		if(rf_get_recv_flag() == RADIO_FLAG_RXERR)
		{
			rf_set_recv_flag(RADIO_FLAG_IDLE);
			atDebug("rf recv err");
			tick = SysTick_usTick();
		}

		if(rf_get_recv_flag() == RADIO_FLAG_RXTIMEOUT)
		{
			rf_set_recv_flag(RADIO_FLAG_IDLE);
			atDebug("rf recv timeout.");
		}

		/* timeout_ms, exit rx */
		if ( (SysTick_usTick()-tick) >= timeout_ms*100 )
		{
			tick = SysTick_usTick();
			atDebug("recv data timeout:%ds", timeout_ms/1000);
			rf_antenna_close();
			rf_set_mode(RF_MODE_STB3);
			rf_refresh();
			break;
		}

		if ( userBreakCheck() )
		{
			atDebug("user break");
			goto exit;
		}

		rf_delay_ms(5);
	}

exit:
	atDebug("rf exit rx mode");
	rf_antenna_close();
	rf_set_mode(RF_MODE_STB3);
	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+RF_RXMODE", "=<timeout[1000(ms)-1000000(ms)]>", PN_NULL, PN_NULL, rf_setRxMode, PN_NULL);

static at_result_t rf_setTxMode(const char *args)
{
	uint64_t tick;
	int txTimes = 0;
	int payloadLen, count, interval, argc;
	const char *req_expr = "=%d,%d,%d";
	u8 txData[300] ;

	/*payloadLen, tx len.
	cout, tx count.
	interval, tx interval time(ms).*/
	atDebug("rf enter tx mode");

	argc = at_req_parse_args(args, req_expr, &payloadLen, &count, &interval);
	if (argc != 3)
	{
		return AT_RESULT_PARSE_FAILE;
	}
	if(payloadLen > 255)
	{
		return AT_RESULT_PARSE_FAILE;
	}
	
	int ret = rf_init();
	if(ret != OK)
	{
		atDebug("rf init er");
		goto exit;
	}
	else
	{
		atDebug("rf init ok");
	}

	rf_SetUserPara((rfConfig_t*)&gRfConfig);
	rf_set_tx_power(gRfConfig.pwr);

	for (int i = 0; i < payloadLen; i++)
	{
		txData[i] = i;
	}
	
	rf_clr_irq(REG_IRQ_TX_DONE);
	tx_carrywave_flag = 0;
	while (1)
	{
		rf_set_transmit_flag(RADIO_FLAG_IDLE);
		ret = rf_enter_continous_tx();
		if(ret != OK)
		{
			atDebug("rf_enter_continous_tx err");
			goto exit;
		}

		ret = rf_continous_tx_send_data(txData, payloadLen);
		if ( ret != Ok )
		{
			atDebug("RF put tx data error");
			goto exit;
		}
		else
		{
			tick = SysTick_usTick();
		}

		while(rf_get_transmit_flag() != RADIO_FLAG_TXDONE)
		{
			rf_irq_process();
			/* timeout, exit rx*/
			if ( (SysTick_usTick()-tick) >= 20000*100 )
			{
				tick = SysTick_usTick();
				atDebug("tx datas timeout:5s");
				goto exit;
			}

			if ( userBreakCheck() )
			{
				atDebug("user break");
				goto exit;
			}
			rf_delay_ms(5);
		}

		rf_enter_continous_rx();

		if(rf_get_transmit_flag() == RADIO_FLAG_TXDONE)
		{
			rf_set_transmit_flag(RADIO_FLAG_IDLE);

			txTimes++;

			atDebug("RF tx datas done[%d]", txTimes);

			if ( txTimes >= count )
			{
				txTimes = 0;
				atDebug("tx %d pkgs success", count);
				goto exit;
			}
			else
			{
				rf_delay_ms(interval);
			}
		}
	}

exit:
	rf_set_mode(RF_MODE_STB3);
	rf_antenna_close();
	rf_refresh();

	atDebug("rf exit tx mode");

	memset(txData,0x00,300);

	if ( ret != OK )
	{
		return AT_RESULT_FAILE;
	}
	else
	{
		return AT_RESULT_OK;
	}
}
AT_CMD_EXPORT("AT+RF_TXMODE", "=<payloadlen>,<count>,<interval>", PN_NULL, PN_NULL, rf_setTxMode, PN_NULL);

//新增一个写寄存器的AT命令，page,reg,val
static at_result_t pan_WriteReg(const char *args)
{
    int argc, page, reg, val;
    const char *req_expr = "=%x,%x,%x";

    argc = sscanf(args, req_expr, &page, &reg, &val);
    if (argc != 3)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("page:%02x", page);
    atDebug("reg:%02x", reg);
    atDebug("val:%02x", val);

    if (page > 1 || page < 0)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    if (reg > 0x7F || reg < 0x00)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    if (val > 0xFF || val < 0x00)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    RF_Err_t ret = rf_write_spec_page_reg((enum PAGE_SEL)page, reg, val);
    if(ret != RF_OK)
    {
        return AT_RESULT_FAILE;
    }
    else
    {
        return AT_RESULT_OK;
    }
}
AT_CMD_EXPORT("AT+WREG", "=<PAGE>,<REG>,<VAL>", PN_NULL, PN_NULL, pan_WriteReg, PN_NULL);

//新增一个读寄存器的AT命令，page,reg
static at_result_t pan_ReadReg(const char *args)
{
    int argc, page, reg;
    const char *req_expr = "=%x,%x";

    argc = sscanf(args, req_expr, &page, &reg);
    if (argc != 2)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("page:0x%02x", page);
    atDebug("reg:0x%02x", reg);

    if (page > 1 || page < 0)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    if (reg > 0x7F || reg < 0x00)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    u8 val = rf_read_spec_page_reg((enum PAGE_SEL)page, reg);
    
    atDebug("val:%d", val);
    printf("+RREG=%02X,%02X,%02X\r\n", page, reg, val);
    
    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+RREG", "=<PAGE>,<REG>", PN_NULL, PN_NULL, pan_ReadReg, PN_NULL);

void printHex1(int align, u8 *buf, int len)
{
    for (int i = 0; i < len; i++)
    {
        printf("%02x ", buf[i]);
        if((i+1)%align == 0)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");
}

static at_result_t pan211_DumpInitVals(void)
{
    u8 i;
    RF_Err_t ret;
    u8 buf[128];

    // dump page0, page1, page2, page3所有寄存器值
    for(i = 0; i <= PAGE3_SEL; i++)
    {
        printf("dump page%d: \r\n", i);
        ret = rf_read_spec_page_regs((enum PAGE_SEL)i, 0, buf, 128);
        if(ret != RF_OK)
        {
            printf("RF_ReadRegs page%d failed.\r\n", i);
        }
        else
        {
            printf("RF_ReadRegs page%d success.\r\n", i);
        }

        printHex1(16, buf, 128);
    }
  
	return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+DUMP", PN_NULL, PN_NULL, PN_NULL, PN_NULL, pan211_DumpInitVals);
