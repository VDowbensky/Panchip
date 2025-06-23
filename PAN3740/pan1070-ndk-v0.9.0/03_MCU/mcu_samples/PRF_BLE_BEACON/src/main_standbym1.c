/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "PanSeries.h"
#include "comm_prf.h"
#include "info.h"
#include "pan_lp.h"

#define ADV_CHANNEL_1				2402		//37
#define ADV_CHANNEL_2				2426		//38
#define ADV_CHANNEL_3				2480		//39
#define ADV_CHANNEL_NUM				3

/**@defgroup LP_WK_SOURCE_FLAG Lowpower wakeup source select
 * @brief       Lowpower wakeup source select definitions
 * @{ */
#define LP_WKUP_MODE_SEL_EXT_GPIO                       (0)
#define LP_WKUP_MODE_SEL_32K                            (1)
#define LP_WKUP_MODE_SEL_GPIO                       	(2)
#define LP_WKUP_MODE_SEL_WDT                            (3)
#define LP_WKUP_MODE_SEL_TIMER                      	(4)
#define LP_WKUP_MODE_SEL_BOD                      		(5)
#define LP_WKUP_MODE_SEL_LVR	                      	(6)

#define SYS_TEST(...)

volatile uint8_t adv_channel_index = 1;
volatile uint8_t sleep_flag = 0;
uint32_t vec_remap_adr = 0x20007000;


pan_prf_config_t tx_config = {
		.work_mode			= PRF_MODE_NORMAL,			//PRF_MODE_NORMAL,PRF_MODE_ENHANCE
		.chip_mode			= PRF_CHIP_MODE_SEL_NRF,	//PRF_CHIP_MODE_SEL_NRF,//PRF_CHIP_MODE_SEL_XN297,
		.trx_mode			= PRF_TX_MODE,
		.phy				= PRF_PHY_1M,				//PRF_PHY_250K,//PRF_PHY_1M,
		.crc				= PRF_CRC_SEL_CRC24,
		.src				= PRF_SRC_SEL_EN,
		.mode_conf			= PRF_BLE_CONF,
		.rx_timeout			= 50000,		//us
		.rf_channel			= ADV_CHANNEL_1,
		.tx_no_ack			= DISABLE,
		.trf_type			= PRF_TRF_NRF52,
		.rx_length			= 0,
		.sync_length		= 4,
		.crc_include_sync	= DISABLE,
		.src_include_sync	= DISABLE,
		.sync				= { 0xd6, 0xbe, 0x89, 0x8e },
		.pid_manual_flag	= DISABLE,
		.tx_power			= -1,
		.pipe				= PRF_PIPE0,
	};

panchip_prf_payload_t tx_payload = {
	.data_length		= 36,
	.data				= { 0xff, 0x22, 0x33, 0x00, 0x05, 0x06,
							0x02, 0x01, 0x06, 0x1A, 0x0ff,
							0xd1, 0x07, /* Panchip */
							0x02, 0x15, /* iBeacon */
							0x18, 0xee, 0x15, 0x16, /* UUID[15..12] */
							0x01, 0x6b, /* UUID[11..10] */
							0x4b, 0xec, /* UUID[9..8] */
							0xad, 0x96, /* UUID[7..6] */
							0xbc, 0xb9, 0x6d, 0x16, 0x6e, 0x97, /* UUID[5..0] */
							0x00, 0x00, /* Major */
							0x00, 0x00, /* Minor */
							0xc8},		/* RSSI */			//mac addr(06 05 00 33 22 ff) + adv data
};

__ramfunc void set_ble_frame_format(void)
{
	panchip_prf_set_endian(PRF_LITTLE_ENDIAN);
	PRI_RF->R04_RX_CTL |= R04_RX_CTL_HDR_LEN_EXIST_Msk;
	PRI_RF->R04_RX_CTL = (PRI_RF->R04_RX_CTL & ~R04_RX_CTL_HDR_LEN_NUMB_Msk) | (2 << R04_RX_CTL_HDR_LEN_NUMB_Pos);
	PRI_RF->R06_TX_CTL = (PRI_RF->R06_TX_CTL & ~R06_TX_CTL_RX_HEADER_Msk) | (2 << R06_TX_CTL_RX_HEADER_Pos);		//0x2,ADV_NONCONN_IND;0x0,ADV_IND;0x4,SCAN_RSP;
}

__ramfunc void set_ble_channel(uint16_t channel)
{
	uint8_t scr_value = 0;
	uint8_t idx = 0;
	uint32_t init_channel[] = {
		0x00007D12,
	};

	if(channel == 2402) {
		idx = 0;
		scr_value = 0x53;
	}else if(channel == 2426) {
		idx = 12;
		scr_value = 0x33;
	} else if(channel == 2480) {
		idx = 39;
		scr_value = 0x73;
	}

	init_channel[0] |= (idx << 16);

	PHY_WriteCfgFifoBatch(init_channel, (sizeof init_channel >> 2), NULL);

	panchip_white_init_value(scr_value);
}

void Sys_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    ANA->LP_FSYN_LDO |= 0X1;

	CLK_XthStartupConfig();
    CLK->XTH_CTRL |= CLK_XTHCTL_XTH_EN_Msk;
	CLK_WaitClockReady(CLK_SYS_SRCSEL_XTH);

	CLK_HCLKConfig(1);
	CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH, CLK_DPLL_OUT_64M);

	CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);

	CLK->RCH_CTRL &= ~BIT(0);

	#if 0	// XTL enalbe
	CLK->XTL_CTRL_3V = (CLK->XTL_CTRL_3V & ~CLK_XTLCTL_CORE_BIAS_Msk_3v) | (0x3<<CLK_XTLCTL_CORE_BIAS_Pos_3v);
	CLK->XTL_CTRL_3V |= CLK_XTLCTL_XTL_CAP_EN_Msk_3v;
	CLK->XTL_CTRL_3V |= CLK_XTLCTL_XTL_EN_Msk_3v;
	while(!(CLK->XTL_CTRL_3V & CLK_XTLCTL_STABLE_Msk));
	CLK->CLK_TOP_CTRL_3V |= CLK_TOPCTL_32K_CLK_SEL_Msk_3v;
	SYS_delay_10nop(1000);
	CLK->RCL_CTRL_3V &= ~CLK_RCLCTL_RC32K_EN_Msk_3v;
	#endif

    CLK_PCLK1Config(1);
    CLK_PCLK2Config(1);
    /*
	 * Note that all clocks on APB are disabled by default after SoC power up, and
	 * the default AHB clocks enabled after SoC power up are:
	 * ACC/eFuse/ROM/RCC_AHB/Systick/GPIO.
	 * Here we disable ACC, eFuse and ROM clock as these modules are not commonly
	 * used. We should re-enabled them once we use later.
	 * Here We also enable APB1 and APB2 clock path to make sure modules on APB can
	 * be easily enabled by each peripheral driver by enabling their clock enable
	 * bits on APB bus.
	 */
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_ROM, DISABLE);
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_APB1 | CLK_AHBPeriph_APB2, DISABLE);

    /*Basic clock for BLE*/
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_BLE_32M | CLK_AHBPeriph_BLE_32K, ENABLE);

	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_USB_AHB|CLK_AHBPeriph_USB_48M, DISABLE);
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

	CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);

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

__ramfunc void event_tx_fun(void)
{
	if(adv_channel_index  < ADV_CHANNEL_NUM)
	{
		adv_channel_index++;
		if(adv_channel_index == 2)
		{
			set_ble_channel(ADV_CHANNEL_2);
		}
		else if(adv_channel_index == 3)
		{
			set_ble_channel(ADV_CHANNEL_3);
		}
		panchip_prf_trx_start();
	}
	else
	{
		adv_channel_index = 1;
		set_ble_channel(ADV_CHANNEL_1);
		sleep_flag = 1;
	}
	SYS_TEST("tx done\n");
}

__ramfunc void event_rx_fun(void)
{
	panchip_prf_payload_t rx_payload;

	rx_payload.data_length = panchip_prf_data_rec(&rx_payload);
	SYS_TEST("rx data:");
	data_printf(rx_payload.data, rx_payload.data_length);
}

__ramfunc void event_rx_timeout_fun(void)
{
	SYS_TEST("rx timeout\n");
}

__ramfunc void event_crc_err_fun(void)
{
	SYS_TEST("rx data crc err\n");
}

__ramfunc void event_pid_err_fun(void)
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

void Sleep_Int_Enable(void)
{
	NVIC_EnableIRQ(LP_IRQn);
}

void temp_autooptimizeparams(void)
{
	#define TEMP_DETECT_INTERVAL	901

	float temp;
	static uint16_t wk_count = 0;

	if(wk_count < TEMP_DETECT_INTERVAL) {
		wk_count++;

		return;
	}
	wk_count = 0;
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_APB1, ENABLE);
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_ADC, ENABLE);
	ADC_Init(ADC, true);
	ADC_SelInputRange(ADC,ADC_INPUTRANGE_LOW);
	temp = ADC_MeasureSocTemperature(ADC);
	ADC->BV_CTL = 0x100;
	PW_AutoOptimizeParams((int)temp);
	CLK_APB1PeriphClockCmd(CLK_APB1Periph_ADC, DISABLE);
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_APB1, DISABLE);
}

void SLPTMR_IRQHandler(void)
{
	ANA->LP_INT_CTRL = ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_LP_INT_EN_Msk | ANAC_INT_SLEEP_TMR_INT_EN_Msk | ANAC_INT_SLEEP_TMR_WK_EN_Msk;

}
void LP_IRQHandler(void)
{
	temp_autooptimizeparams();
}

__ramfunc void LP_RestoreAfterStandbyWakeup(void)
{
	extern const uint32_t PanFlashLineMode;
	extern const bool PanFlashEnhanceEnable;

    // Speed up flash clock from default 4MHz to 32MHz
    CLK_SetFlashClkDiv(CLK_FLASH_CLKDIV_1);
    // Switch flash to 4-Line mode
    FMC_SetFlashMode(FLCTL, PanFlashLineMode, PanFlashEnhanceEnable);
    // Init I-Cache
    InitIcache(FLCTL, PanFlashLineMode);

	LP_ClearWakeFlag(ANA,ANAC_INT_SLEEP_TMR0_Msk|ANAC_INT_STANDBY_M1_FLAG_Msk);
	wake_from_lp_mode = true;
	NVIC_EnableIRQ(LP_IRQn);
	SYS_TEST("restored\n");
}

__ramfunc void LP_SetStandbyMode1Config(ANA_T *ana,
							uint32_t wkMode,
							uint8_t PowerCtrl,
							uint32_t slpTimer,
							uint8_t spaceIdx,
							uint8_t stdy_mode)
{
	LP_SetSleepMode(ana, LP_MODE_SEL_STANDBY_M1_MODE);
	LP_EnableInt(ana,ENABLE);
	if (stdy_mode == LP_STANDBY_M1_MODE1){
		ana->LP_LP_LDO_3V |= ANAC_LPLDO_H_EN_Msk_3v;
		ana->LP_LP_LDO_3V |= ANAC_LPLDO_L_EN_Msk;
		ana->LP_FL_CTRL_3V |=  ANAC_FL_LDO_ISOLATE_EN_Msk;
		ana->LP_FL_CTRL_3V &= ~(ANAC_LDOL_POWER_CTL_Msk | ANAC_LDO_POWER_CTL_Msk);
	} else if (stdy_mode == LP_STANDBY_M1_MODE2){
		ana->LP_LP_LDO_3V |= ANAC_LPLDO_H_EN_Msk_3v;
		ana->LP_LP_LDO_3V &= ~ANAC_LPLDO_L_EN_Msk;
		ana->LP_FL_CTRL_3V |= ANAC_LDOL_POWER_CTL_Msk | ANAC_FL_LDO_ISOLATE_EN_Msk;
		ana->LP_FL_CTRL_3V &= ~(ANAC_LDO_POWER_CTL_Msk);
	} else if (stdy_mode == LP_STANDBY_M1_MODE3){
		ana->LP_LP_LDO_3V &= ~ANAC_LPLDO_H_EN_Msk_3v;
		ana->LP_LP_LDO_3V |= ANAC_LPLDO_L_EN_Msk;
		ana->LP_FL_CTRL_3V |= ANAC_LDOL_POWER_CTL_Msk | ANAC_FL_LDO_ISOLATE_EN_Msk;
		ana->LP_FL_CTRL_3V &= ~(ANAC_LDO_POWER_CTL_Msk);
	}

	ana->LP_FL_CTRL_3V = ((PowerCtrl & 0x1f) << 24u) | (ana->LP_FL_CTRL_3V & 0xe0FFFFFF);
	if((LP_WKUP_MODE_SEL_EXT_GPIO != wkMode)&&(LP_WKUP_MODE_SEL_GPIO != wkMode)){
		if (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_32K_CLK_SEL_Msk_3v) {
			ana->LP_FL_CTRL_3V |= ANAC_FL_XTAL32K_EN_Msk_3v;
			ana->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
		} else {
			ana->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
			ana->LP_FL_CTRL_3V |= ANAC_FL_RC32K_EN_Msk_3v;
		}
		ana->LP_INT_CTRL |= ANAC_INT_SLEEP_TMR_WK_EN_Msk;
	} else {
		ana->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
		ana->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
	}

	// Wait 3v sync ready manually
	ANA->LP_REG_SYNC |= ANAC_LP_REG_SYNC_3V_Msk | ANAC_LP_REG_SYNC_3V_TRG_Msk;
	while(ANA->LP_REG_SYNC & (ANAC_LP_REG_SYNC_3V_TRG_Msk)) {}

	if((LP_WKUP_MODE_SEL_EXT_GPIO != wkMode)&&(LP_WKUP_MODE_SEL_GPIO != wkMode)){
		LP_SetSleepTime(ana,slpTimer,spaceIdx);
	}
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	__WFI();
}

__ramfunc uint32_t LP_StandbyCpuRetentionRemap(void)
{
	uint32_t tmp_reg, sw = 0x1f;
    uint32_t wkupMode = LP_WKUP_MODE_SEL_GPIO;

	tmp_reg = ANA->LP_LP_LDO_3V;
	tmp_reg |= ANAC_LPLDO_H_MODE_SEL_Msk_3v;
	ANA->LP_LP_LDO_3V = tmp_reg;


	P0->MODE = 0;
	P1->MODE = 0;
	P2->MODE = 0;
	P3->MODE = 0;
	GPIO_DisableDigitalPath(P0,0xff);
	GPIO_DisablePullupPath(P0,0xff);
	GPIO_DisableDigitalPath(P1,0xff);
	GPIO_DisablePullupPath(P1,0xff);
	GPIO_DisableDigitalPath(P2,0xff);
	GPIO_DisablePullupPath(P2,0xff);
	GPIO_DisableDigitalPath(P3,0xff);
	GPIO_DisablePullupPath(P3,0xff);

	FMC_DisableAutoDp(FLCTL);

	sw = 0x0d;
	wkupMode = LP_WKUP_MODE_SEL_32K;
	ANA->LP_INT_CTRL |= ANAC_INT_SLEEP_TMR_INT_EN_Msk;
	GPIO_DisableDigitalPath(P1,BIT7);
	LP_SetWakeDelay(ANA, 0X7);
	LP_SetStandbyMode1Config(ANA, wkupMode, sw, 10656, 0, LP_STANDBY_M1_MODE1);
	LP_RestoreAfterStandbyWakeup();

	return 0;
}

__ramfunc uint32_t LP_StandbyCpuRetentionRemapTest(void)
{
	uint32_t tmp_reg = ANA->CPU_ADDR_REMAP_CTRL;

	tmp_reg &= ~ANAC_CPU_REMAP_ADR_Msk;
	tmp_reg |= ((vec_remap_adr >> 8) & ANAC_CPU_REMAP_ADR_Msk);
	tmp_reg |= ANAC_CPU_ADR_REMAP_EN_Msk;
	ANA->CPU_ADDR_REMAP_CTRL = tmp_reg;
	ANA->LP_FL_CTRL_3V |= ANAC_FL_CPU_RETENTION_EN_Msk;

	LP_StandbyCpuRetentionRemap();
	return 0;
}

int main(void)
{
	uint8_t null_value = 0;
	static bool first_dp_config = false;

	memcpy((void*)vec_remap_adr, (void*)(null_value), 256);

	Sys_Init();

	Sleep_Int_Enable();

	ANA->LP_HP_LDO &= ~ANAC_HPLDO_FLASHLDO_BP_Msk_3v;
	ANA->LP_SW &= ~BIT3;		//sram1
//	ANA->LP_SW &= ~BIT2;		//sram0 close

//	Uart_Init();

    pan10xx_hw_calib_init();

    SYS_TEST("check info\n");
    if(check_info_tlv_data_prf()) {
		phy_value_init_from_info_prf();
    } else {
		phy_value_init_from_code_prf();
	}

	panchip_prf_init(&tx_config);

	set_ble_channel(ADV_CHANNEL_1);

	set_ble_frame_format();

    panchip_prf_set_data(&tx_payload);

	/*adr match bit */
	PRI_RF_SetAddrMatchBit(PRI_RF, 0);

	SYS_TEST("prf tx sample init\n");

	panchip_prf_trx_start();

	while (1)
	{
		if(sleep_flag == 1) {
			sleep_flag = 0;

			if (first_dp_config == false) {
				first_dp_config = true;
				LP_StandbyCpuRetentionRemapTest();
			} else {
				ANA->LP_INT_CTRL |= ANAC_INT_SLEEP_TMR0_Msk;
				LP_SetSleepMode(ANA, LP_MODE_SEL_STANDBY_M1_MODE);
				LP_SetSleepTime(ANA,10656, 0);
				SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
				__WFI();
				LP_RestoreAfterStandbyWakeup();
			}
			panchip_prf_init(&tx_config);
			set_ble_channel(ADV_CHANNEL_1);
			set_ble_frame_format();
			panchip_prf_set_data(&tx_payload);
			panchip_prf_trx_start();
			LP_SetSleepMode(ANA, LP_MODE_SEL_SLEEP_MODE);
			__WFI();
		} else {
			LP_SetSleepMode(ANA, LP_MODE_SEL_SLEEP_MODE);
			__WFI();
		}
	}
}
