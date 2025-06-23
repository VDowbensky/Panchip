/*
 * Copyright (c) 2021 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "PANSeries.h"
#include "comm_prf.h"
#include "info.h"
#include "lcd_ui.h"
#include "sample_config.h"

#define CONFIG_ACK_TX    0

#define OLED_UI_ENABLE      1

uint32_t rx_refresh_freq = 1000;

volatile uint32_t period_test_cnt = 0;
volatile uint32_t rx_cnt = 0;
volatile uint32_t rx_timeout_cnt = 0;
volatile uint32_t rx_crc_err_cnt = 0;
volatile uint32_t rx_pid_err_cnt = 0;
volatile uint32_t rx_len_err_cnt = 0;
volatile uint32_t rx_acc_err_cnt = 0;
volatile int16_t last_rssi = 0;

pan_prf_config_t rx_config =
{
	.work_mode			= (prf_mode_t)PRF_WORK_MODE,			//PRF_MODE_NORMAL,PRF_MODE_ENHANCE
	.chip_mode			= (prf_chip_mode_sel_t)PRF_CHIP_MODE,	//PRF_CHIP_MODE_SEL_NRF,//PRF_CHIP_MODE_SEL_XN297,
	.trx_mode			= (prf_trx_mode_t)PRF_TRX_SELECT,
	.phy				= (prf_phy_t)PRF_PHY_SELECT,			//PRF_PHY_250K,//PRF_PHY_1M,
	.crc				= (prf_crc_sel_t)PRF_CRC,
	.src				= (prf_scramble_sel_t)PRF_SCR,
	.mode_conf			= (prf_mode_conf_sel_t)PRF_MODE_CONFIG,
	.rx_timeout			= DISABLE,			//us
	.rf_channel			= 2410,
	.tx_no_ack			= ENABLE,
	.trf_type			= (prf_trf_t)PRF_TRANS_TYPE,
	.rx_length			= 37,
	.sync_length		= 5,
	.crc_include_sync	= ENABLE,
	.src_include_sync	= DISABLE,
	.sync				= { 0x01, 0x02, 0x03, 0x04, 0x05 },
	.pid_manual_flag	= DISABLE,
	.tx_power			= 0,
	.pipe				= PRF_PIPE0,
};

pan_prf_config_t rx_297_config =
{
	.work_mode			= (prf_mode_t)PRF_MODE_NORMAL,			//PRF_MODE_NORMAL,PRF_MODE_ENHANCE
	.chip_mode			= (prf_chip_mode_sel_t)PRF_CHIP_MODE_SEL_XN297,	//PRF_CHIP_MODE_SEL_NRF,//PRF_CHIP_MODE_SEL_XN297,
	.trx_mode			= (prf_trx_mode_t)PRF_TRX_SELECT,
	.phy				= (prf_phy_t)PRF_PHY_1M,			//PRF_PHY_250K,//PRF_PHY_1M,
	.crc				= (prf_crc_sel_t)PRF_CRC_SEL_CRC16,
	.src				= (prf_scramble_sel_t)PRF_SRC_SEL_EN,
	.mode_conf			= (prf_mode_conf_sel_t)PRF_BLE_CONF,
	.rx_timeout			= DISABLE,			//us
	.rf_channel			= 2478,
	.tx_no_ack			= DISABLE,
	.trf_type			= (prf_trf_t)PRF_TRF_NORMAL,
	.rx_length			= 37,
	.sync_length		= 5,
	.crc_include_sync	= ENABLE,
	.src_include_sync	= ENABLE,
	.sync				= { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC },
	.pid_manual_flag	= DISABLE,
	.tx_power			= 0,
	.pipe				= PRF_PIPE0,
};

pan_prf_config_t rx_b250k_config =
{
	.work_mode			= (prf_mode_t)PRF_MODE_NORMAL,			//PRF_MODE_NORMAL,PRF_MODE_ENHANCE
	.chip_mode			= (prf_chip_mode_sel_t)PRF_CHIP_MODE_SEL_NRF,	//PRF_CHIP_MODE_SEL_NRF,//PRF_CHIP_MODE_SEL_XN297,
	.trx_mode			= (prf_trx_mode_t)PRF_RX_MODE,
	.phy				= (prf_phy_t)PRF_PHY_250K,			//PRF_PHY_250K,//PRF_PHY_1M,
	.crc				= (prf_crc_sel_t)PRF_CRC_SEL_CRC16,
	.src				= (prf_scramble_sel_t)PRF_SRC_SEL_NOSRC,
	.mode_conf			= (prf_mode_conf_sel_t)PRF_B_250K_CONF,
	.rx_timeout			= DISABLE,			//us
	.rf_channel			= 2410,
	.tx_no_ack			= DISABLE,
	.trf_type			= (prf_trf_t)PRF_TRF_B250K,
	.rx_length			= 37,
	.sync_length		= 4,
	.crc_include_sync	= DISABLE,
	.src_include_sync	= DISABLE,
	.sync				= { 0x91, 0xd3, 0x91, 0xd3 },
	.pid_manual_flag	= DISABLE,
	.tx_power			= 0,
	.pipe				= PRF_PIPE0,
};

pan_prf_config_t silicon_rx_config =
{
	.work_mode			= PRF_MODE_NORMAL,			//PRF_MODE_NORMAL,PRF_MODE_ENHANCE
	.chip_mode			= PRF_CHIP_MODE_SEL_NRF,	//PRF_CHIP_MODE_SEL_NRF,//PRF_CHIP_MODE_SEL_XN297,
	.trx_mode			= PRF_RX_MODE,
	.phy				= PRF_PHY_250K,			//PRF_PHY_250K,//PRF_PHY_1M,
	.crc				= PRF_CRC_SEL_CRC16,
	.src				= PRF_SRC_SEL_NOSRC,
	.mode_conf			= PRF_B_250K_CONF,
	.rx_timeout			= DISABLE,			//us
	.rf_channel			= 2410,
	.tx_no_ack			= DISABLE,
	.trf_type			= PRF_TRF_B250K,			//PRF_TRF_B250K
	.rx_length			= 0,
	.sync_length		= 4,
	.crc_include_sync	= DISABLE,
	.src_include_sync	= DISABLE,
	.sync				= {0x2c, 0x6e, 0x2c, 0x6e},//{ 0x91, 0xd3, 0x91, 0xd3 },
	.pid_manual_flag	= DISABLE,
	.tx_power			= 0,
	.pipe				= PRF_PIPE0,
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

	if (len == 0)
	{
		return;
	}

	for (; i < len; i++)
	{
		SYS_TEST("0x%02X ", data[i]);
	}
	SYS_TEST("\n");
}

void event_tx_fun(void)
{
	panchip_prf_payload_t rx_payload;

	rx_payload.data_length = panchip_prf_data_rec(&rx_payload);
	panchip_prf_trx_start();
}

static panchip_prf_payload_t tx_payload =
{
    .data_length = 10,
    .data = { 0x10, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa },
};


uint16_t test_cnt = 0;
void event_rx_fun(void)
{
    if (is_silicon_lab_test_mode())
    {
        rx_cnt++;
        panchip_prf_trx_start();
    }else if(is_b250k_test_mode())
    {
        panchip_prf_trx_start();
        rx_cnt++;
    }else if(is_297_test_mode())
    {
        panchip_prf_trx_start();
        rx_cnt++;
    } else{
        if (rx_config.work_mode == PRF_MODE_ENHANCE)
        {
            #if(CONFIG_ACK_TX)
            tx_payload.data[0]++;
            panchip_prf_set_ack_data(&tx_payload);

            panchip_prf_payload_t rx_payload;
            rx_payload.data_length = panchip_prf_data_rec(&rx_payload);
            #endif
            rx_cnt++;
        }
        else
        {
            panchip_prf_payload_t rx_payload;
            rx_payload.data_length = panchip_prf_data_rec(&rx_payload);
            panchip_prf_trx_start();
            rx_cnt++;
        }
    }

    last_rssi = prf_rssi;
}

void event_rx_timeout_fun(void)
{
    rx_timeout_cnt++;
    panchip_prf_trx_start();
}

void event_crc_err_fun(void)
{
	panchip_prf_payload_t rx_payload;

	rx_payload.data_length = panchip_prf_data_rec(&rx_payload);
	panchip_prf_trx_start();
    rx_crc_err_cnt++;
}

void event_pid_err_fun(void)
{
	rx_pid_err_cnt++;
}

void event_len_err_fun(void)
{
    rx_len_err_cnt++;
}

void event_acc_adr_err_fun(void)
{
    panchip_prf_payload_t rx_payload;
    rx_payload.data_length = panchip_prf_data_rec(&rx_payload);
    panchip_prf_trx_start();
    rx_acc_err_cnt++;
}

void SysTick_Handler(void)
{
    period_test_cnt++;

    if(period_test_cnt == 1001)
    {
        SYS_TEST("rx_cnt:%d\n", rx_cnt);
        SYS_TEST("rx_crc_err_cnt:%d\n", rx_crc_err_cnt);
        SYS_TEST("rx_pid_err_cnt:%d\n", rx_pid_err_cnt);
        SYS_TEST("rx_len_err_cnt:%d\n", rx_len_err_cnt);
        SYS_TEST("rx_acc_err_cnt:%d\n", rx_acc_err_cnt);
        SYS_TEST("rx_timeout_cnt:%d\n", rx_timeout_cnt);
        SYS_TEST("last_rssi:%d\n", last_rssi);

        #if OLED_UI_ENABLE
        update_test_result_ui();
        #endif

        period_test_cnt = 0;
        rx_cnt = 0;
        rx_timeout_cnt = 0;
        rx_crc_err_cnt = 0;
        rx_pid_err_cnt = 0;
        rx_len_err_cnt = 0;
        rx_acc_err_cnt = 0;
        last_rssi = 0;
    }
}

void panchip_prf_isr_init(void)
{
	isr_cb.tx_cb = event_tx_fun;
	isr_cb.rx_cb = event_rx_fun;
	isr_cb.rx_timeout_cb = event_rx_timeout_fun;
	isr_cb.rx_crc_err_cb = event_crc_err_fun;
	isr_cb.rx_pid_err_cb = event_pid_err_fun;
    isr_cb.rx_acc_adr_err_cb = event_acc_adr_err_fun;
}

void panchip_prf_irq_enable(void)
{
	NVIC_SetPriority (LL_IRQn, 0);
	/* Enable RF interrupt */
	NVIC_EnableIRQ(LL_IRQn);
}

void print_dcoc_signed_value(uint8_t signed_flag, uint8_t value)
{
    if(signed_flag)
    {
        SYS_TEST("-");
        SYS_TEST("%d\n",(0xff - value + 1));
    }else{
        SYS_TEST("%d\n",value);
    }
}

void read_clibrate_value(void)
{
    uint32_t flag_temp;
    uint32_t temp_value;

	uint32_t value;
	uint32_t response;

	value = 0x0001FF12;				/* select Page 1 */
	PHY_WriteCfgFifoBatch(&value, 1, &response);

    SYS_TEST("--------------------------------\n");
	value = 0x00006113;
	PHY_WriteCfgFifoBatch(&value, 1, &flag_temp);
	SYS_TEST("gain2-I:\n61[%x]\n",flag_temp);
	value = 0x00006213;
	PHY_WriteCfgFifoBatch(&value, 1, &temp_value);
	SYS_TEST("62[%x]\n",temp_value);
    SYS_TEST("gain2-I Int:");
    print_dcoc_signed_value(flag_temp, temp_value);

	value = 0x00006313;
	PHY_WriteCfgFifoBatch(&value, 1, &flag_temp);
	SYS_TEST("gain3-I:\n63[%x]\n",flag_temp);
	value = 0x00006413;
	PHY_WriteCfgFifoBatch(&value, 1, &temp_value);
	SYS_TEST("64[%x]\n",temp_value);
    SYS_TEST("gain3-I Int:");
    print_dcoc_signed_value(flag_temp, temp_value);

	value = 0x00007113;
	PHY_WriteCfgFifoBatch(&value, 1, &flag_temp);
	SYS_TEST("gain2-Q:\n71[%x]\n",flag_temp);
	value = 0x00007213;
	PHY_WriteCfgFifoBatch(&value, 1, &temp_value);
	SYS_TEST("72[%x]\n",temp_value);
    SYS_TEST("gain2-Q Int:");
    print_dcoc_signed_value(flag_temp, temp_value);

	value = 0x00007313;
	PHY_WriteCfgFifoBatch(&value, 1, &flag_temp);
	SYS_TEST("gain3-Q:\n73[%x]\n",flag_temp);
	value = 0x00007413;
	PHY_WriteCfgFifoBatch(&value, 1, &temp_value);
	SYS_TEST("74[%x]\n",temp_value);
    SYS_TEST("gain3-Q Int:");
    print_dcoc_signed_value(flag_temp, temp_value);

	value = 0x0000FF12;				/* select Page 0 */
	PHY_WriteCfgFifoBatch(&value, 1, &response);
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

    SYS_TEST("App Start Up...");

    #if OLED_UI_ENABLE
    init_lcd();
    #endif

    pan10xx_hw_calib_init();
	#if CONFIG_SIMULATE_WRITE_INFO
    start_overwrite_info();
    start_read_1k_info();
    start_read_info();
    #endif

    #if CONFIG_UART_OUTPUT_INFO
    total_tlv_uart_data_output();
    #else

    if(check_info_tlv_data_prf()) {
		phy_value_init_from_info_prf();
        SYS_TEST("load from info\n");
    } else {
		phy_value_init_from_code_prf();
        SYS_TEST("load from code\n");
	}
    #endif


    #if OLED_UI_ENABLE
    /*blocked here*/
    while(!ui_get_ready_test())
    {
    }
    #endif

    if (is_silicon_lab_test_mode()) /*silicon lab test mode*/
    {
        rx_refresh_freq = 250;

        panchip_prf_init(&silicon_rx_config);

        if(silicon_rx_config.trf_type == PRF_TRF_B250K)
        {
            panchip_prf_b250k_set_channel(1);
        }

        PRI_RF->R11_CFG &= ~R11_CFG_RX_DATA_REV_EN_Msk;
        PRI_RF->R11_CFG &= ~R11_CFG_B250K_INVERT_EN_Msk;
    } else if(is_297_test_mode())
    {
        rx_refresh_freq = 250;
        panchip_prf_init(&rx_297_config);
        panchip_prf_set_chn(rx_297_config.rf_channel);

    } else if(is_b250k_test_mode())
    {
        panchip_prf_init(&rx_b250k_config);

        if(rx_b250k_config.phy == PRF_PHY_250K)
        {
            rx_refresh_freq = 172; /* 100 packet 5.8S */
        }

        if(rx_b250k_config.trf_type == PRF_TRF_B250K)
        {
            panchip_prf_b250k_set_channel(10);
        }

    } else {
        #if OLED_UI_ENABLE
        prf_config_updated_from_ui(&rx_config);
        prf_packet_updated_from_ui(&tx_payload);
        #endif

        if((rx_config.phy == PRF_PHY_CODED) || (rx_config.phy == PRF_PHY_CODED_S8))
        {
            rx_config.sync_length = 4;
        }

        if(rx_config.mode_conf == PRF_B_250K_CONF)
        {
            rx_config.sync_length = 4;
            rx_config.sync[0] = 0x91; rx_config.sync[1] = 0xd3; rx_config.sync[2] = 0x91; rx_config.sync[3] = 0xd3;
            rx_config.work_mode = PRF_MODE_NORMAL;
            rx_config.crc_include_sync = DISABLE;
            rx_config.trf_type = PRF_TRF_B250K;
        }

        panchip_prf_init(&rx_config);

        if(rx_config.phy == PRF_PHY_250K)
        {
            rx_refresh_freq = 250;
        }

        if(rx_config.phy == PRF_PHY_CODED)
        {
            rx_refresh_freq = 500;
        }

        if(rx_config.phy == PRF_PHY_CODED_S8)
        {
            rx_refresh_freq = 125;
        }

        if(rx_config.mode_conf == PRF_B_250K_CONF)
        {
            rx_refresh_freq = 250;
        }

        panchip_prf_set_chn(rx_config.rf_channel);

    }

    read_clibrate_value();

	/*adr match bit */
	PRI_RF_SetAddrMatchBit(PRI_RF, 0);
	panchip_prf_trx_start();

	SYS_TEST("prf rx sample init\n");


    SysTick_Config(48000000/rx_refresh_freq);

	while (1)
	{

	}
}
