/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PanSeries.h"
#include "comm_prf.h"
#include "info.h"

#define REG_WRITE_BITS(reg, mask, value)    (reg) = (((reg) & ~(mask)) | (value))


pan_prf_config_t usr_config = {
	.work_mode			= (prf_mode_t)PRF_MODE_NORMAL,
	.chip_mode			= (prf_chip_mode_sel_t)PRF_CHIP_MODE_SEL_XN297,
	.trx_mode			= (prf_trx_mode_t)PRF_RX_MODE,
	.phy				= (prf_phy_t)PRF_PHY_1M,
	.crc				= (prf_crc_sel_t)PRF_CRC_SEL_CRC16,
	.src				= (prf_scramble_sel_t)PRF_SRC_SEL_EN,
	.mode_conf			= (prf_mode_conf_sel_t)PRF_BLE_CONF,
	.rx_timeout			= DISABLE,			//us
	.rf_channel			= 2410,
	.tx_no_ack			= DISABLE,
	.trf_type			= (prf_trf_t)PRF_TRF_NORMAL,
	.rx_length			= 5,
	.sync_length		= 4,
	.crc_include_sync	= ENABLE,
	.src_include_sync	= DISABLE,
	.sync				= { 0x91, 0xd3, 0x91, 0xd3 },
	.pid_manual_flag	= DISABLE,
	.tx_power			= 0,
	.pipe				= PRF_PIPE0,
};

panchip_prf_payload_t tx_payload = {
	.data_length = 5,
	.data = { 0x01, 0x02, 0x03, 0x04, 0x05 },
};

panchip_prf_payload_t ack_payload = {
	.data_length = 5,
	.data = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 },
};

typedef struct {
	uint16_t rx_cnt;
	uint16_t timeout_cnt;
	uint16_t crc_cnt;
	uint16_t pid_cnt;
} rf_isr_cnt_t;
static rf_isr_cnt_t rf_isr_cnt;

static volatile bool tx_done;
static volatile bool rx_done;
static volatile bool rssi_read;
static volatile bool rate_test;
static volatile bool limit_rate_test;



static volatile int16_t rssi_read_value;
static volatile uint8_t ack_length = 5;

extern void prf_delay_nop(uint32_t nop_count);

#define DEBUG_PIN_ENABLE (0)

void Sys_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

	ANA->LP_FSYN_LDO |= 0X1;
    //MCU
	CLK_XthStartupConfig();
    CLK->XTH_CTRL |= CLK_XTHCTL_XTH_EN_Msk;
	CLK_WaitClockReady(CLK_SYS_SRCSEL_XTH);
    //MCU
    CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH,CLK_DPLL_OUT_48M);
	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);
    //APB Enable
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_All,ENABLE);

    CLK_APB1PeriphClockCmd(CLK_APB1Periph_All,ENABLE);
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_All,ENABLE);

    SYS_LockReg();
}

void Uart_Init(void)
{
#ifdef IP_101x
    /* Init I/O Multi-function  */
    SYS_SET_MFP(P1, 1, UART0_TX);
    SYS_SET_MFP(P1, 2, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT2);
#else
    /* Init I/O Multi-function  */
    SYS_SET_MFP(P1, 6, UART0_TX);
    SYS_SET_MFP(P1, 7, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT7);
#endif

    UART_InitTypeDef Init_Struct;

	Init_Struct.UART_BaudRate = 921600;
	Init_Struct.UART_LineCtrl = Uart_Line_8n1;

	/* Init UART1 */
	UART_Init(UART0, &Init_Struct);
	UART_EnableFifo(UART0);
}

static int pan10xx_hw_calib_init(void)
{
	OTP_STRUCT_T otp;

    SYS_TEST("Try to load HW calibration data..");
    if (!SystemHwParamLoader(&otp)) {
        SYS_TEST("\nWARNING: Cannot find valid calib data in current chip!\n");
    } else {
        SYS_TEST(" DONE.\n");
        SYS_TEST("- Chip Info         : 0x%x\n", otp.m.chip_info);
        SYS_TEST("- Chip CP Version   : %d\n", otp.m.cp_version);
        SYS_TEST("- Chip FT Version   : %d\n", otp.m.ft_version);
        if (otp.m.ft_version >= 2) {
			SYS_TEST("- Chip MAC Address  : %02X%02X%02X%02X%02X%02X\n", otp.m_v2.mac_addr[0], otp.m_v2.mac_addr[1],
				otp.m_v2.mac_addr[2], otp.m_v2.mac_addr[3], otp.m_v2.mac_addr[4], otp.m_v2.mac_addr[5]);
		} else {
			SYS_TEST("- Chip MAC Address  : %02X%02X%02X%02X%02X%02X\n", otp.m.mac_addr[0], otp.m.mac_addr[1],
				otp.m.mac_addr[2], otp.m.mac_addr[3], otp.m.mac_addr[4], otp.m.mac_addr[5]);
		}
        SYS_TEST("- Chip UID          : %02X%02X%02X%02X%02X%02X%02X%02X%02X\n", otp.m.uid[0], otp.m.uid[1],
            otp.m.uid[2], otp.m.uid[3], otp.m.uid[4], otp.m.uid[5], otp.m.uid[6], otp.m.uid[7], otp.m.uid[8]);

#if INIT_OPTIMIZE_ACTIVE_POWER
        uint32_t tmp;
        int32_t val;

        //buck out(DCDC): ft - 2
        val = (otp.m.buck_out_trim >> 1) - 2;
        if (val < 0x0) {
            val = 0x0;
        }
        tmp = ANA->LP_BUCK_3V;
        tmp &= ~(0xFu << 2);
        tmp |= (val << 2);
        ANA->LP_BUCK_3V = tmp;

        //HPLDO(DVDD): ft - 2
        val = otp.m.hp_ldo_trim - 2;
        if (val < 0x0) {
            val = 0x0;
        }
        tmp = ANA->LP_HP_LDO;
        tmp &= ~(0xFu << 3);
        tmp |= (val << 3);
        ANA->LP_HP_LDO = tmp;
#endif
    }
    SYS_TEST("- Chip Flash UID    : ");
    for (uint32_t i = 0; i < 16; i++) {
        SYS_TEST("%02X", flash_ids.uid[i]);
    }
    SYS_TEST("\n- Chip Flash Size   : %ld KB\n", BIT(flash_ids.memory_density_id) >> 10);

	return 0;
}



#if (DEBUG_PIN_ENABLE == 1)
void panchip_debug_init(void)
{
	SYS_SET_MFP(P2, 5, GPIO);
	GPIO_SetMode(P2, BIT5, GPIO_MODE_OUTPUT);
	P25 = 0;
}

void panchip_debug_output(uint8_t value)
{
	(value) ? (P25 = 1) : (P25 = 0);
}
#endif

void data_printf(uint8_t const *data, uint32_t len)
{
	uint32_t i = 0;

	if (len == 0) {
		return;
	}

	for (; i < len; i++) {
		SYS_TEST("0x%02X ", data[i]);
	}
	SYS_TEST("\n");
}

void init_payload_data(void)
{
	uint8_t send_data[PRF_DATA_MAX_SIZE];

	for (uint8_t i = 0; i < PRF_DATA_MAX_SIZE; i++) {
		send_data[i] = i;
	}
	memcpy(ack_payload.data, send_data, PRF_DATA_MAX_SIZE);
}

__ramfunc uint16_t set_ack_payload_length(void)
{
	char length;
	uint16_t data_length = 0;

	SYS_TEST("choose data length:\n");
	SYS_TEST("0-->1byte payload\n");
	SYS_TEST("1-->32byte payload\n");
	SYS_TEST("2-->63byte payload\n");
	SYS_TEST("3-->64byte payload\n");
	SYS_TEST("4-->255byte payload\n");
	length = getchar();

	switch (length) {
	case '0':
		data_length = 1;
		break;
	case '1':
		data_length = 32;
		break;
	case '2':
		data_length = 63;
		break;
	case '3':
		data_length = 64;
		break;
	case '4':
		data_length = 255;
		break;
	default:
		data_length = 5;
		break;
	}

	return data_length;
}

__ramfunc void set_ack_payload(void)
{
	ack_payload.data_length = ack_length;
	panchip_prf_set_data(&ack_payload);
}

//__ramfunc void panchip_prf_reduce_trx_pre_post_time(void)
//{
//	extern uint32_t pre_tx_config_2M_prf[];
//	extern uint32_t post_tx_config[];
//	extern uint32_t pre_rx_config_2M_prf[];

//	/* 24us */
//	pre_tx_config_2M_prf[5] = (0x001C7012 | (0x8 << 24));
//	post_tx_config[0] = (0x000E7002 | (0x0 << 24));
//	/* 21us */
//	pre_rx_config_2M_prf[4] = (0x00060A02 | (0x4 << 24));
//	extern uint32_t PHY_SeqRamInit(void);
//	PHY_SeqRamInit();
//}

__ramfunc uint8_t panchip_prf_data_rec_fast(panchip_prf_payload_t *p_payload)
{
	uint32_t index, tmp_data;
	uint8_t i;

	p_payload->data_length = ((LLHWC_READ32_REG(LIST_RAM_OFST, data_addr_rx) >> 16) & 0xff);
	/* SYS_TEST("%d",p_payload->data_length); */
	if ((p_payload->data_length % 4) == 0) {
		index = p_payload->data_length / 4;
	} else {
		index = p_payload->data_length / 4 + 1;
	}

	for (i = 0; i < index; i++) {
		tmp_data = LLHWC_READ32_REG(LIST_RAM_OFST, (data_addr_rx + 4 + i * 4));
		p_payload->data[i * 4] = (uint8_t)tmp_data;
		p_payload->data[i * 4 + 1] = (uint8_t) (tmp_data >> 8);
		p_payload->data[i * 4 + 2] = (uint8_t) (tmp_data >> 16);
		p_payload->data[i * 4 + 3] = (uint8_t) (tmp_data >> 24);
	}
	return p_payload->data_length;
}

__ramfunc void panchip_prf_set_ack_data_fast(panchip_prf_payload_t *p_payload)
{
	uint8_t i;
	uint32_t tmp_data;

	PRI_RF_WRITE_REG_VALUE(PRI_RF, R00_CTL, TX_PAYLOAD_LEN, p_payload->data_length);
	PRI_RF_WRITE_REG_VALUE(PRI_RF, R00_CTL, RX_PAYLOAD_LEN, p_payload->data_length);

	for (i = 0; i < (p_payload->data_length + 3) / 4; i++) {
		tmp_data = (((uint32_t) p_payload->data[i * 4]) | (((uint32_t) p_payload->data[i * 4 + 1]) << 8) |
			    (((uint32_t) p_payload->data[i * 4 + 2]) << 16) | (((uint32_t) p_payload->data[i * 4 + 3]) << 24));
		LLHWC_WRITE32_REG(REG_FILE_OFST, LIST_RAM_OFST + (data_addr_tx + i) * 4, tmp_data);
	}
}

__ramfunc void panchip_prf_trx_start_fast(void)
{
	PRI_RF_SET_FUNC_ENABLE(PRI_RF, R07_SRAM_CTL, TX_READY, DISABLE);
	PRI_RF_SET_FUNC_ENABLE(PRI_RF, R07_SRAM_CTL, RX_READY, DISABLE);
	PRI_RF_SET_FUNC_ENABLE(PRI_RF, R07_SRAM_CTL, TX_READY, ENABLE);
	PRI_RF_SET_FUNC_ENABLE(PRI_RF, R07_SRAM_CTL, RX_READY, ENABLE);
}

__ramfunc void ana_prf_ldo_dis(void)
{
	if (!limit_rate_test) {
		REG_WRITE_BITS(ANA->ANA_ADC_LDO, 0x83, 0x82);
		REG_WRITE_BITS(ANA->ANA_RFFE_LDO, 0x83, 0x82);
		REG_WRITE_BITS(ANA->ANA_VCO_LDO, 0x83, 0x82);
		if (usr_config.work_mode == PRF_MODE_ENHANCE) {
			prf_delay_nop(90);
		}
	}
}

__ramfunc void event_tx_fun(void)
{
	if (!rate_test) {
		SYS_TEST("tx done\n");
	} else {
		if (limit_rate_test) {
			panchip_prf_trx_start_fast();
		} else {
			panchip_prf_trx_start();
		}
	}
	tx_done = true;
}

__ramfunc void event_rx_fun(void)
{
	panchip_prf_payload_t rx_payload;

	if (rate_test) {
		rf_isr_cnt.rx_cnt++;
		if (usr_config.work_mode != PRF_MODE_ENHANCE) {
			if (limit_rate_test) {
				panchip_prf_trx_start_fast();
			} else {
				panchip_prf_trx_start();
			}
		}
	} else {
		rx_payload.data_length = panchip_prf_data_rec(&rx_payload);
		if (usr_config.work_mode != PRF_MODE_ENHANCE) {
			SYS_TEST("rx data:");
			data_printf(rx_payload.data, rx_payload.data_length);
		}

		if (usr_config.work_mode == PRF_MODE_ENHANCE) {
			set_ack_payload();
		}
	}

	rx_done = true;
	if (rssi_read) {
		rssi_read_value = panchip_prf_read_rssi();
	}
}

__ramfunc void event_rx_timeout_fun(void)
{
	if (rate_test) {
		rf_isr_cnt.timeout_cnt++;
	}
	panchip_prf_trx_start();
}

__ramfunc void event_crc_err_fun(void)
{
	panchip_prf_payload_t rx_payload;

	if (rate_test) {
		rf_isr_cnt.crc_cnt++;
		if (limit_rate_test) {
			panchip_prf_trx_start_fast();
		} else {
			panchip_prf_trx_start();
		}
	} else {
		SYS_TEST("rx data crc err\n");
		rx_payload.data_length = panchip_prf_data_rec(&rx_payload);
		SYS_TEST("rx data:");
		data_printf(rx_payload.data, rx_payload.data_length);
	}
	rx_done = true;

}

__ramfunc void event_pid_err_fun(void)
{
	if (rate_test) {
		rf_isr_cnt.pid_cnt++;
		if (limit_rate_test) {
			panchip_prf_trx_start_fast();
		}
	} else {
		SYS_TEST("pid error\n");
	}
}

void panchip_prf_isr_init(void)
{
	isr_cb.tx_cb = event_tx_fun;
	isr_cb.rx_cb = event_rx_fun;
	isr_cb.rx_timeout_cb = event_rx_timeout_fun;
	isr_cb.rx_crc_err_cb = event_crc_err_fun;
	isr_cb.rx_pid_err_cb = event_pid_err_fun;
}

void panchip_prf_irq_enable(void)
{
	NVIC_SetPriority (LL_IRQn, 0);
	/* Enable RF interrupt */
	NVIC_EnableIRQ(LL_IRQn);
}


void prf_api_test_print(void)
{
	SYS_TEST("--------------------PAN1070 RF API TEST------------------\n");
	SYS_TEST("A: TEST RF API panchip_prf_set_chn\n");
	SYS_TEST("B: TEST RF API PHY_SetExternChannel\n");
	SYS_TEST("C: TEST RF API panchip_switch_prf\n");
	SYS_TEST("D: TEST RF API panchip_prf_set_tx_pwr\n");
	SYS_TEST("E: TEST RF API panchip_prf_set_trx_mode\n");
	SYS_TEST("F: TEST RF API panchip_prf_set_addr\n");
	SYS_TEST("G: TEST RF API panchip_prf_data_rec\n");
	SYS_TEST("H: TEST RF API panchip_prf_mode_conf_set\n");
	SYS_TEST("I: TEST RF API panchip_prf_rx_stop\n");
	SYS_TEST("J: TEST RF API panchip_prf_read_rssi\n");
	SYS_TEST("K: TEST RF API panchip_prf_read_carrier_rssi\n");
	SYS_TEST("L: TEST RF API panchip_prf_set_ack_data\n");
	SYS_TEST("M: TEST RF API event_pid_err_fun\n");
	SYS_TEST("N: TEST RF API panchip_prf_trx_trans_time\n");
	SYS_TEST("O: TEST RF packet_acceptance_rate_test_normal\n");
	SYS_TEST("P: TEST RF limit_packet_acceptance_rate_test_normal\n");
	SYS_TEST("Q: TEST RF packet_acceptance_rate_test_enhance\n");
	SYS_TEST("R: TEST RF limit_packet_acceptance_rate_test_enhance\n");
}

void prf_debug_pin_enable(void)
{
	PRI_RF_WRITE_REG_VALUE(PRI_RF, TEST_MUX02, TST_MUX_SELECT_11, 0x14); //tx on
    SYS->P1_MFP |= SYS_MFP_P10_LL_DBG_11;

	PRI_RF_WRITE_REG_VALUE(PRI_RF, TEST_MUX02, TST_MUX_SELECT_09, 0x15); //rx on
	SYS->P0_MFP |= SYS_MFP_P04_LL_DBG_9;

	PRI_RF_WRITE_REG_VALUE(PRI_RF, TEST_MUX02, TST_MUX_SELECT_10, 0x16); //addr match
    SYS->P0_MFP |= SYS_MFP_P07_LL_DBG_10;

	PRI_RF_WRITE_REG_VALUE(PRI_RF, TEST_MUX03, TST_MUX_SELECT_12, 0x05); //seq done
	SYS->P0_MFP |= SYS_MFP_P05_LL_DBG_12;
}

void prf_api_test_panchip_prf_get_version(void)
{
	pan_prf_version_t *version;

	version = panchip_prf_get_version();
	SYS_TEST("major.minor.patch.commit_id: %x,%x,%x,%x\n", version->major,
	       version->minor, version->patch, version->commit_id);
}

void prf_api_test_panchip_prf_set_chn(void)
{
	uint16_t channel = 2402;

	while (channel <= 2480) {
		SYS_TEST("channel = %d\n", channel);
		#if (DEBUG_PIN_ENABLE == 1)
		panchip_debug_output(1);
		#endif
		panchip_prf_set_chn(channel);
		#if (DEBUG_PIN_ENABLE == 1)
		panchip_debug_output(0);
		#endif
		panchip_prf_trx_start();
		while (!rx_done) {
		}
		rx_done = false;
		channel += 1;
	}
}

void prf_api_test_PHY_SetExternChannel(void)
{
	uint16_t channel = 2360;

	while (channel <= 2401) {
		SYS_TEST("channel = %d\n", channel);
		#if (DEBUG_PIN_ENABLE == 1)
		panchip_debug_output(1);
		#endif
		panchip_prf_set_chn(channel);
		#if (DEBUG_PIN_ENABLE == 1)
		panchip_debug_output(0);
		#endif
		panchip_prf_trx_start();
		while (!rx_done) {
		}
		rx_done = false;
		channel += 1;
	}

	channel = 2481;
	while (channel <= 2510) {
		SYS_TEST("channel = %d\n", channel);
		panchip_prf_set_chn(channel);
		panchip_prf_trx_start();
		while (!rx_done) {
		}
		rx_done = false;
		channel += 1;
	}
}

void prf_api_test_panchip_switch_prf(void)
{
	uint16_t channel = 2402;

	panchip_prf_set_chn(channel);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;

	SYS_TEST("change chip mode to nrf mode\n");
	usr_config.chip_mode = PRF_CHIP_MODE_SEL_NRF;
	panchip_switch_prf(&usr_config);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;
	usr_config.chip_mode = PRF_CHIP_MODE_SEL_XN297;
	panchip_switch_prf(&usr_config);
}

void prf_api_test_panchip_prf_set_tx_pwr(void)
{
	uint16_t channel = 2402;
	int8_t tx_pwr_value = -45;

	rssi_read = true;

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);

	while (tx_pwr_value <= 7) {
		panchip_prf_set_chn(channel);
		SYS_TEST("ch %d test tx pwr = %d dbm\n", channel, tx_pwr_value);
		panchip_prf_trx_start();
		while (!rx_done) {
		}
		rx_done = false;
		tx_pwr_value++;
		channel++;
		SYS_TEST("rssi = %d\n", rssi_read_value);
	}

	rssi_read = false;

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);
}

void prf_api_test_panchip_prf_set_trx_mode(void)
{
	uint16_t channel = 2402;

	SYS_TEST("set rx mode\n");
	panchip_prf_set_chn(channel);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;

	SYS_TEST("turn to tx mode\n");
	panchip_prf_set_trx_mode(PRF_TX_MODE);
	panchip_prf_set_data(&tx_payload);
	panchip_prf_trx_start();
	while (!tx_done) {
	}
	tx_done = false;
	panchip_prf_set_trx_mode(PRF_RX_MODE);
}

void prf_api_test_panchip_prf_set_addr(void)
{
	uint16_t channel = 2402;
	uint8_t addr[5] = { 0x12, 0x23, 0x34, 0x45, 0x56 };

	SYS_TEST("set rx mode\n");
	panchip_prf_set_chn(channel);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;

	SYS_TEST("set new addr\n");
	panchip_prf_set_addr(addr, 5, PRF_PIPE0, PRI_RF_MODE_SEL_RX);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;

	uint8_t addr_raw[4] = { 0x71, 0x76, 0x41, 0x76 };

	SYS_TEST("restore raw addr\n");
	panchip_prf_set_addr(addr_raw, 4, PRF_PIPE0, PRI_RF_MODE_SEL_RX);
}

void prf_api_test_panchip_prf_data_rec(void)
{
	uint16_t channel = 2402;

	panchip_prf_set_chn(channel);

	SYS_TEST("receive 1 data\n");
	PRI_RF_SetTrxPayloadLen(PRI_RF, PRI_RF_MODE_SEL_RX, 1);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;

	SYS_TEST("receive 64 data\n");
	PRI_RF_SetTrxPayloadLen(PRI_RF, PRI_RF_MODE_SEL_RX, 64);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;

	SYS_TEST("receive 255 data\n");
	PRI_RF_SetTrxPayloadLen(PRI_RF, PRI_RF_MODE_SEL_RX, 255);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;


	SYS_TEST("change chip mode to nrf mode\n");
	usr_config.chip_mode = PRF_CHIP_MODE_SEL_NRF;
	panchip_switch_prf(&usr_config);
	SYS_TEST("receive 1 data\n");
	PRI_RF_SetTrxPayloadLen(PRI_RF, PRI_RF_MODE_SEL_RX, 1);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;

	SYS_TEST("receive 64 data\n");
	PRI_RF_SetTrxPayloadLen(PRI_RF, PRI_RF_MODE_SEL_RX, 64);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;

	SYS_TEST("receive 255 data\n");
	PRI_RF_SetTrxPayloadLen(PRI_RF, PRI_RF_MODE_SEL_RX, 255);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;

	SYS_TEST("restore chip mode to xn297 mode\n");
	usr_config.chip_mode = PRF_CHIP_MODE_SEL_XN297;
	panchip_switch_prf(&usr_config);
}

void prf_api_test_panchip_prf_mode_conf_set(void)
{
	uint16_t channel = 2402;

	panchip_prf_set_chn(channel);
	usr_config.chip_mode = PRF_CHIP_MODE_SEL_XN297;
	panchip_switch_prf(&usr_config);
	panchip_prf_mode_conf_set(PRF_BLE_CONF);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;

	SYS_TEST("change chip mode to nrf mode\n");
	SYS_TEST("set dev mode to nrf mode\n");
	usr_config.chip_mode = PRF_CHIP_MODE_SEL_NRF;
	panchip_switch_prf(&usr_config);
	panchip_prf_mode_conf_set(PRF_NRF_CONF);
	panchip_prf_trx_start();
	while (!rx_done) {
	}
	rx_done = false;

	usr_config.chip_mode = PRF_CHIP_MODE_SEL_XN297;
	panchip_switch_prf(&usr_config);
	panchip_prf_mode_conf_set(PRF_BLE_CONF);
}

void prf_api_test_panchip_prf_rx_stop(void)
{
	uint16_t channel = 2402;

	SYS_TEST("enter rx mode without timeout\n");
	panchip_prf_set_chn(channel);
	panchip_prf_trx_start();

	SYS_TEST("hold on 100ms, calling the rx stop function triggers a timeout interrupt\n");
	panchip_prf_rx_stop();
}

void prf_api_test_panchip_prf_read_rssi(void)
{
	uint16_t channel = 2402;


	rssi_read = true;

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);

	while (channel <= 2480) {
		panchip_prf_set_chn(channel);
		panchip_prf_trx_start();
		while (!rx_done) {
		}
		rx_done = false;
		SYS_TEST("channel = %d, rssi = %d\n", channel, rssi_read_value);
		channel += 1;
	}

	rssi_read = false;

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);
}

void prf_api_test_panchip_prf_read_carrier_rssi(void)
{
	uint16_t channel = 2402;
	uint16_t rcv_cnt = 0;

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);

	panchip_prf_enable_carrier_rssi();
	while (rcv_cnt <= 100) {
		panchip_prf_reset();
		panchip_prf_set_chn(channel);
		panchip_prf_trx_start();
		TIMER_Delay(TIMER0, 50000);
		SYS_TEST("channel = %d, rssi = %d\n", channel, panchip_prf_read_carrier_rssi());
		rcv_cnt += 1;
	}

	panchip_prf_disable_carrier_rssi();

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);
}

void prf_api_test_panchip_prf_set_ack_data(void)
{
	uint16_t channel = 2402;
	uint16_t send_cnt = 0;

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);

	usr_config.work_mode = PRF_MODE_ENHANCE;
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);

	init_payload_data();
	ack_length = set_ack_payload_length();
	SYS_TEST("set ack payload length = %d\n", ack_length);
	panchip_prf_set_chn(channel);

	prf_debug_pin_enable();

	while (send_cnt < 10) {
		panchip_prf_trx_start();
		while (!tx_done) {
		}
		tx_done = false;
		send_cnt += 1;
	}

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);

	usr_config.work_mode = PRF_MODE_NORMAL;
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);
}

void prf_api_test_event_pid_err_fun(void)
{
	uint16_t channel = 2402;
	uint16_t send_cnt = 0;

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);

	usr_config.work_mode = PRF_MODE_ENHANCE;
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);

	ack_length = 5;
	SYS_TEST("set ack payload length = %d\n", ack_length);
	panchip_prf_set_chn(channel);

	prf_debug_pin_enable();

	while (send_cnt < 10) {
		panchip_prf_trx_start();
		while (!tx_done) {
		}
		tx_done = false;
		send_cnt += 1;
	}

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);

	usr_config.work_mode = PRF_MODE_NORMAL;
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);
}

__ramfunc void prf_api_test_panchip_prf_trx_trans_time(void)
{
	uint16_t channel = 2402;
	uint16_t send_cnt = 0;
	uint16_t trans_wait_time = 120;

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);

	usr_config.work_mode = PRF_MODE_ENHANCE;
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);

	init_payload_data();
	ack_length = set_ack_payload_length();
	SYS_TEST("set ack payload length = %d\n", ack_length);
	panchip_prf_set_chn(channel);

	prf_debug_pin_enable();

	while (send_cnt < 10) {
		SYS_TEST("set transfer wait time %d us\n", trans_wait_time);
		PRI_RF_SetTrxTransWaitTime(PRI_RF, trans_wait_time);
		panchip_prf_trx_start();
		while (!tx_done) {
		}
		tx_done = false;
		send_cnt += 1;
		trans_wait_time += 5;
	}

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);

	usr_config.work_mode = PRF_MODE_NORMAL;
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);
}

void packet_acceptance_rate_test_normal(void)
{
	uint16_t channel = 2402, receive_length = 5;
	uint8_t test_cnt = 0;

	rate_test = true;
	panchip_prf_set_chn(channel);

	prf_debug_pin_enable();

	receive_length = set_ack_payload_length();
	SYS_TEST("receive data length = %d\n", receive_length);
	PRI_RF_SetTrxPayloadLen(PRI_RF, PRI_RF_MODE_SEL_RX, receive_length);
	panchip_prf_trx_start();

	while (test_cnt < 10) {
		if (rx_done) {
			TIMER_Delay(TIMER0, 3500000);
			SYS_TEST("rx %d timeout %d crc %d pid %d\n", rf_isr_cnt.rx_cnt, rf_isr_cnt.timeout_cnt, rf_isr_cnt.crc_cnt, rf_isr_cnt.pid_cnt);
			memset(&rf_isr_cnt, 0, sizeof(rf_isr_cnt));
			rx_done = false;
			test_cnt++;
		}
	}

	TIMER_Delay(TIMER0, 1000);
	rate_test = false;
}

__ramfunc void limit_packet_acceptance_rate_test_normal(void)
{
	uint16_t channel = 2402, receive_length = 5;

	rate_test = true;
	limit_rate_test = true;
	panchip_prf_set_chn(channel);
	prf_debug_pin_enable();

	receive_length = set_ack_payload_length();
	SYS_TEST("receive data length = %d\n", receive_length);
	PRI_RF_SetTrxPayloadLen(PRI_RF, PRI_RF_MODE_SEL_RX, receive_length);
	PRI_RF_SetTrxTransWaitTime(PRI_RF, 0);
	panchip_prf_reduce_trx_pre_post_delay_time(&usr_config, false);
	panchip_prf_trx_start_fast();

	while (1) {
		if (rx_done) {
			TIMER_Delay(TIMER0, 1000000);
			SYS_TEST("rx %d timeout %d crc %d pid %d\n", rf_isr_cnt.rx_cnt, rf_isr_cnt.timeout_cnt, rf_isr_cnt.crc_cnt, rf_isr_cnt.pid_cnt);
			memset(&rf_isr_cnt, 0, sizeof(rf_isr_cnt));
			rx_done = false;
		}
	}

	TIMER_Delay(TIMER0, 1000000);
	rate_test = false;
	limit_rate_test = false;
}

void packet_acceptance_rate_test_enhance(void)
{
	uint16_t channel = 2402;
	uint16_t test_cnt = 0;

	rate_test = true;

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);

	usr_config.work_mode = PRF_MODE_ENHANCE;
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);

	ack_length = set_ack_payload_length();
	if (ack_length > 64) {
		SYS_TEST("enhance mode max payload is 64 byte\n");
	} else {
		SYS_TEST("set ack payload length = %d\n", ack_length);
	}
	panchip_prf_set_chn(channel);

	prf_debug_pin_enable();
	panchip_prf_trx_start();

	while (test_cnt < 10) {
		if (rx_done) {
			TIMER_Delay(TIMER0, 2500000);
			SYS_TEST("rx %d timeout %d crc %d pid %d\n", rf_isr_cnt.rx_cnt, rf_isr_cnt.timeout_cnt, rf_isr_cnt.crc_cnt, rf_isr_cnt.pid_cnt);
			memset(&rf_isr_cnt, 0, sizeof(rf_isr_cnt));
			rx_done = false;
			test_cnt++;
		}
	}
	rate_test = false;

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);

	usr_config.work_mode = PRF_MODE_NORMAL;
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);
}

void limit_packet_acceptance_rate_test_enhance(void)
{
	uint16_t channel = 2402;

	rate_test = true;
	limit_rate_test = true;

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);

	usr_config.work_mode = PRF_MODE_ENHANCE;
	usr_config.phy = PRF_PHY_2M;

	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);

	ack_length = set_ack_payload_length();
	if (ack_length > 64) {
		SYS_TEST("enhance mode max payload is 64 byte\n");
	} else {
		SYS_TEST("set ack payload length = %d\n", ack_length);
	}
	panchip_prf_set_chn(channel);

	prf_debug_pin_enable();
	int16_t transfer_time = panchip_prf_set_trx_trans_time(&usr_config, PRF_TRANSFER_SPEED_8K, 33);
	if (transfer_time < 0) {
		SYS_TEST("time set error, trans_wait_time = %d\n", transfer_time);
		return;
	}
	panchip_prf_trx_start_fast();

	while (1) {
		if (rx_done) {
			TIMER_Delay(TIMER0, 1000000);
			SYS_TEST("rx %d timeout %d crc %d pid %d\n", rf_isr_cnt.rx_cnt, rf_isr_cnt.timeout_cnt, rf_isr_cnt.crc_cnt, rf_isr_cnt.pid_cnt);
			memset(&rf_isr_cnt, 0, sizeof(rf_isr_cnt));
			rx_done = false;
		}
	}
	rate_test = false;
	limit_rate_test = false;

	CLK->IPRST0 |= (CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);
	CLK->IPRST0 &= ~(CLK_IPRST0_MDMSTDBYRST_Msk | CLK_IPRST0_MDMRST_Msk);

	usr_config.work_mode = PRF_MODE_NORMAL;
	panchip_prf_module_init();
	PHY_set_tx_pwr(usr_config.tx_power);
	panchip_prf_init(&usr_config);
}

int main(void)
{
	char input;

	Sys_Init();
	Uart_Init();
	#if (DEBUG_PIN_ENABLE == 1)
	panchip_debug_init();
	#endif

    pan10xx_hw_calib_init();

	SYS_TEST("check info\n");
    if(check_info_tlv_data_prf()) {
		phy_value_init_from_info_prf();
    } else {
		phy_value_init_from_code_prf();
	}

	SYS_TEST("\nCPU @ %dHz\n", SystemCoreClock);
	prf_api_test_panchip_prf_get_version();

	init_payload_data();

	panchip_prf_init(&usr_config);

	prf_debug_pin_enable();

	prf_api_test_print();

	while (1) {
		input = getchar();

		switch (input) {
		case 'A':
			prf_api_test_panchip_prf_set_chn();
			break;
		case 'B':
			prf_api_test_PHY_SetExternChannel();
			break;
		case 'C':
			prf_api_test_panchip_switch_prf();
			break;
		case 'D':
			prf_api_test_panchip_prf_set_tx_pwr();
			break;
		case 'E':
			prf_api_test_panchip_prf_set_trx_mode();
			break;
		case 'F':
			prf_api_test_panchip_prf_set_addr();
			break;
		case 'G':
			prf_api_test_panchip_prf_data_rec();
			break;
		case 'H':
			prf_api_test_panchip_prf_mode_conf_set();
			break;
		case 'I':
			prf_api_test_panchip_prf_rx_stop();
			break;
		case 'J':
			prf_api_test_panchip_prf_read_rssi();
			break;
		case 'K':
			prf_api_test_panchip_prf_read_carrier_rssi();
			break;
		case 'L':
			prf_api_test_panchip_prf_set_ack_data();
			break;
		case 'M':
			prf_api_test_event_pid_err_fun();
			break;
		case 'N':
			prf_api_test_panchip_prf_trx_trans_time();
			break;
		case 'O':
			packet_acceptance_rate_test_normal();
			break;
		case 'P':
			limit_packet_acceptance_rate_test_normal();
			break;
		case 'Q':
			packet_acceptance_rate_test_enhance();
			break;
		case 'R':
			limit_packet_acceptance_rate_test_enhance();
			break;

		default:
			break;
		}
		prf_api_test_print();
	}
	return 0;
}
