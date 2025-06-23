/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PanSeries.h"
#include "comm_prf.h"
#include "info.h"

volatile uint32_t usr_pid = 0;

pan_prf_config_t tx_config = {
		.work_mode			= (prf_mode_t)PRF_WORK_MODE,			//PRF_MODE_NORMAL,PRF_MODE_ENHANCE
		.chip_mode			= (prf_chip_mode_sel_t)PRF_CHIP_MODE,	//PRF_CHIP_MODE_SEL_NRF,//PRF_CHIP_MODE_SEL_XN297,
		.trx_mode			= (prf_trx_mode_t)PRF_TRX_SELECT,
		.phy				= (prf_phy_t)PRF_PHY_SELECT,			//PRF_PHY_250K,//PRF_PHY_1M,
		.crc				= (prf_crc_sel_t)PRF_CRC,
		.src				= (prf_scramble_sel_t)PRF_SCR,
		.mode_conf			= (prf_mode_conf_sel_t)PRF_MODE_CONFIG,
		.rx_timeout			= 50000,		//us
		.rf_channel			= 2410,
		.tx_no_ack			= DISABLE,
		.trf_type			= (prf_trf_t)PRF_TRANS_TYPE,
		.rx_length			= 0,
		.sync_length		= 4,
		.crc_include_sync	= DISABLE,
		.src_include_sync	= DISABLE,
		.sync				= { 0x91, 0xd3, 0x91, 0xd3 },
		.pid_manual_flag	= DISABLE,
		.tx_power			= 0,
		.pipe				= PRF_PIPE0,
	};

	panchip_prf_payload_t tx_payload = {
		.data_length		= 5,
		.data				= { 0x01, 0x02, 0x03, 0x04, 0x05 },
	};

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

void event_tx_fun(void)
{
	if (usr_pid >= 3){
		usr_pid = 0;
	}
	PRI_RF_SetPidManual(PRI_RF, PRF_TX_MODE, usr_pid);
	usr_pid++;
	SYS_TEST("tx done\n");
}

void event_rx_fun(void)
{
	panchip_prf_payload_t rx_payload;

	rx_payload.data_length = panchip_prf_data_rec(&rx_payload);
	SYS_TEST("rx data:");
	data_printf(rx_payload.data, rx_payload.data_length);
}

void event_rx_timeout_fun(void)
{
	SYS_TEST("rx timeout\n");
}

void event_crc_err_fun(void)
{
	SYS_TEST("rx data crc err\n");
}

void event_pid_err_fun(void)
{

}

void SysTick_Handler(void)
{
	panchip_prf_trx_start();

	SYS_TEST("tx restart\n");
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

int main(void)
{
	Sys_Init();

	Uart_Init();

    pan10xx_hw_calib_init();

	SYS_TEST("check info\n");
    if(check_info_tlv_data_prf()) {
		phy_value_init_from_info_prf();
    } else {
		phy_value_init_from_code_prf();
	}

	panchip_prf_init(&tx_config);
	if(tx_config.trf_type == PRF_TRF_B250K)
	{
		panchip_prf_b250k_set_channel(3);
	}
	else
	{
		panchip_prf_set_chn(tx_config.rf_channel);
	}

	/*adr match bit */
	PRI_RF_SetAddrMatchBit(PRI_RF, 0);
	panchip_prf_set_data(&tx_payload);

	SYS_TEST("prf tx sample init\n");

	SysTick_Config(16000000);				//333ms

	while (1)
	{

	}
}
