/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file ev_hc32f460_lqfp100_v2.c
 * @brief
 *
 * @history - V0.8, 2024-4
*******************************************************************************/
#include "ev_hc32f460_lqfp100_v2.h"
#include "hc32f460_usart.h"
#include "hc32_ddl.h"
#include "pan_rf.h"
#include "oled.h"
#include "display.h"
#include "hc32f460_gpio.h"

#define USART_CH                        (M4_USART1)
#define USART_BAUDRATE                  (115200ul)
#define USART_RX_PORT                   (PortA)
#define USART_RX_PIN                    (Pin11)
#define USART_RX_FUNC                   (Func_Usart1_Rx)
#define USART_TX_PORT                   (PortA)
#define USART_TX_PIN                    (Pin12)
#define USART_TX_FUNC                   (Func_Usart1_Tx)

#if (BSP_EV_HC32F460_LQFP100_V2 == BSP_EV_HC32F460)

#define SPI_SCK_PORT                    (PortA)
#define SPI_SCK_PIN                     (Pin03)
#define SPI_SCK_FUNC                    (Func_Spi1_Sck)
#define SPI_NSS_PORT                    (PortA)
#define SPI_NSS_PIN                     (Pin04)
#define SPI_NSS_FUNC                    (Func_Spi1_Nss0)
#define SPI_MOSI_PORT                   (PortA)
#define SPI_MOSI_PIN                    (Pin02)
#define SPI_MOSI_FUNC                   (Func_Spi1_Mosi)
#define SPI_MISO_PORT                   (PortA)
#define SPI_MISO_PIN                    (Pin01)
#define SPI_MISO_FUNC                   (Func_Spi1_Miso)
#define SPI_UNIT                        (M4_SPI1)
#define SPI_UNIT_CLOCK                  (PWC_FCG1_PERIPH_SPI1)

#define TIMER4_UNIT                     (M4_TMR41)
/* Timer4 PWM */
#define TIMER4_PWM_CH                   (Timer4PwmU)    /* Timer4PwmU  Timer4PwmV  Timer4PwmW */
#define TIMER4_PWM_RT_VAL               (250u)
#define WAVE_IO_PORT                    (PortB)
#define WAVE_IO_PIN                     (Pin10)
/* Timer4 RT interrupt number */
#define TIMER4_RT_INT_NUM               (INT_TMR41_RLOU)

#define TMR_UNIT            (M4_TMR02)
#define TMR_INI_GCMA        (INT_TMR02_GCMA)
#define TMR_INI_GCMB        (INT_TMR02_GCMB)

#define ENABLE_TMR0()       (PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM02, Enable))

#define SPI_IDLE                            0
#define SPI_BUSY                            1

uint32_t Timer6_tmpdelay = 0;
uint32_t Timer6_tmp = 0;
extern uint32_t cad_tx_timeout_flag;
extern uint64_t us_u64TickCount;

uint32_t tx_mode = 0;
uint32_t tx_power = DEFAULT_PWR;
uint32_t freq_mode = 0;
uint32_t tx_pkt_max_num = 1;
uint32_t tx_pkt_cnt = 0;
uint32_t rx_pkt_cnt = 0;
uint8_t tx_buf[256];
uint32_t tx_len = 10;
uint32_t tx_time;
uint32_t tx_ing = 0;
uint32_t keys_pressed = 0;
uint32_t key_second_pressed = 0;
uint32_t sys_tick = 0;
uint8_t tx_carrywave_flag = false;
static int spi_busy_flag = SPI_IDLE;

static void AdcClockConfig(void);
static void AdcInitConfig(void);
static void AdcChannelConfig(void);
static void AdcSetChannelPinMode(const M4_ADC_TypeDef *ADCx,
                                 uint32_t u32Channel,
                                 en_pin_mode_t enMode);
static void AdcSetPinMode(uint8_t u8AdcPin, en_pin_mode_t enMode);
extern uint32_t app_tx_mode;
extern uint32_t freq_list[FREQ_LIST_NUM];
extern struct RxDoneMsg RxDoneParams;

typedef struct
{
    en_port_t    port;
    en_pin_t     pin;
} BSP_KeyIn_Config;

static const BSP_KeyIn_Config BSP_KEYIN_PORT_PIN[BSP_KEY_ROW_NUM] = {
    {BSP_KEYIN0_PORT, BSP_KEYIN0_PIN},
    {BSP_KEYIN1_PORT, BSP_KEYIN1_PIN},
    {BSP_KEYIN2_PORT, BSP_KEYIN2_PIN},
    {BSP_KEYIN3_PORT, BSP_KEYIN3_PIN},
    {BSP_KEYIN4_PORT, BSP_KEYIN4_PIN},
    {BSP_KEYIN5_PORT, BSP_KEYIN5_PIN}
};

typedef struct
{
    en_port_t port;
    en_pin_t pin;
    uint32_t first_state;
    uint32_t second_state;
    uint32_t last_tick;
    void (*func)(void);
    void (*func_second)(void);
} key_info_t;

typedef enum
{
    RESET = 0,
    SET = !RESET
} FlagStatus, ITStatus;

key_info_t key_list[6] =
{
    [0] = {
        .port = BSP_KEYIN0_PORT,
        .pin = BSP_KEYIN0_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = key_set_sf_routine,
        .func_second = key_set_pl_routine,
    },
    [1] = {
        .port = BSP_KEYIN1_PORT,
        .pin = BSP_KEYIN1_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = key_set_bw_routine,
        .func_second = key_set_power_routine,
    },
    [2] = {
        .port = BSP_KEYIN2_PORT,
        .pin = BSP_KEYIN2_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = key_set_mode_routine,
        .func_second = key_set_cr_routine,
    },
    [3] = {
        .port = BSP_KEYIN3_PORT,
        .pin = BSP_KEYIN3_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = key_set_start_tx_routine,
        .func_second = key_set_fq_routine,
    },
    [4] = {
        .port = BSP_KEYIN4_PORT,
        .pin = BSP_KEYIN4_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = key_clear_cnt,
        .func_second = NULL,
    },
    [5] = {
        .port = BSP_KEYIN5_PORT,
        .pin = BSP_KEYIN5_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = NULL,
    },
};

uint32_t get_spi_state(void)
{
    return spi_busy_flag;
}

void set_spi_state(int status)
{
    spi_busy_flag = status;
}

static void AdcClockConfig(void)
{
    stc_clk_xtal_cfg_t stcXtalCfg;
    stc_clk_upll_cfg_t stcUpllCfg;

    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcUpllCfg);

    stcXtalCfg.enFastStartup = Enable;
    stcXtalCfg.enMode = ClkXtalModeOsc;
    stcXtalCfg.enDrv = ClkXtalLowDrv;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    stcUpllCfg.pllmDiv = 2u;
    stcUpllCfg.plln    = 60u;
    stcUpllCfg.PllpDiv = 16u;
    stcUpllCfg.PllqDiv = 16u;
    stcUpllCfg.PllrDiv = 16u;
    CLK_SetPllSource(ClkPllSrcXTAL);
    CLK_UpllConfig(&stcUpllCfg);
    CLK_UpllCmd(Enable);
    CLK_SetPeriClkSource(ClkPeriSrcUpllr);
}

static void AdcInitConfig(void)
{
    stc_adc_init_t stcAdcInit;

    MEM_ZERO_STRUCT(stcAdcInit);
    stcAdcInit.enResolution = AdcResolution_12Bit;
    stcAdcInit.enDataAlign  = AdcDataAlign_Right;
    stcAdcInit.enAutoClear  = AdcClren_Disable;
    stcAdcInit.enScanMode   = ADC1_SCAN_MODE;

    PWC_Fcg3PeriphClockCmd(PWC_FCG3_PERIPH_ADC1, Enable);
    ADC_Init(M4_ADC1, &stcAdcInit);
}

static void AdcChannelConfig(void)
{
    stc_adc_ch_cfg_t stcChCfg;
    uint8_t au8Adc1SaSampTime[ADC1_SA_CHANNEL_COUNT] = ADC1_SA_CHANNEL_SAMPLE_TIME;

    MEM_ZERO_STRUCT(stcChCfg);
    stcChCfg.u32Channel  = ADC1_SA_CHANNEL;
    stcChCfg.u8Sequence  = ADC_SEQ_A;
    stcChCfg.pu8SampTime = au8Adc1SaSampTime;
    /* 1. Set the ADC pin to analog mode. */
    AdcSetChannelPinMode(M4_ADC1, ADC1_CHANNEL, Pin_Mode_Ana);
    /* 2. Add ADC channel. */
    ADC_AddAdcChannel(M4_ADC1, &stcChCfg);

    /* 3. Configure the average channel if you need. */
    ADC_ConfigAvg(M4_ADC1, AdcAvcnt_32);
    /* 4. Add average channel if you need. */
    ADC_AddAvgChannel(M4_ADC1, ADC1_AVG_CHANNEL);
}

static void AdcSetChannelPinMode(const M4_ADC_TypeDef *ADCx,
                                 uint32_t u32Channel,
                                 en_pin_mode_t enMode)
{
    uint8_t u8ChIndex;
    uint8_t u8ChOffset = 0u;

    u32Channel &= ADC1_PIN_MASK_ALL;

    u8ChIndex = 0u;
    while (0u != u32Channel)
    {
        if (u32Channel & 0x1ul)
        {
            AdcSetPinMode((u8ChIndex+u8ChOffset), enMode);
        }

        u32Channel >>= 1u;
        u8ChIndex++;
    }
}

static void AdcSetPinMode(uint8_t u8AdcPin, en_pin_mode_t enMode)
{
    en_port_t enPort = PortA;
    en_pin_t enPin   = Pin00;
    bool bFlag       = true;
    stc_port_init_t stcPortInit;

    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = enMode;
    stcPortInit.enPullUp  = Disable;

    switch (u8AdcPin)
    {
    case ADC1_IN0:
        enPort = PortA;
        enPin  = Pin00;
        break;
    case ADC1_IN1:
        enPort = PortA;
        enPin  = Pin01;
        break;
    case ADC1_IN2:
        enPort = PortA;
        enPin  = Pin02;
        break;
    case ADC1_IN3:
        enPort = PortA;
        enPin  = Pin03;
        break;
    case ADC12_IN4:
        enPort = PortA;
        enPin  = Pin04;
        break;
    case ADC12_IN5:
        enPort = PortA;
        enPin  = Pin05;
        break;
    case ADC12_IN6:
        enPort = PortA;
        enPin  = Pin06;
        break;
    case ADC12_IN7:
        enPort = PortA;
        enPin  = Pin07;
        break;
    case ADC12_IN8:
        enPort = PortB;
        enPin  = Pin00;
        break;
    case ADC12_IN9:
        enPort = PortB;
        enPin  = Pin01;
        break;
    case ADC12_IN10:
        enPort = PortC;
        enPin  = Pin00;
        break;
    case ADC12_IN11:
        enPort = PortC;
        enPin  = Pin01;
        break;
    case ADC1_IN12:
        enPort = PortC;
        enPin  = Pin02;
        break;
    case ADC1_IN13:
        enPort = PortC;
        enPin  = Pin03;
        break;
    case ADC1_IN14:
        enPort = PortC;
        enPin  = Pin04;
        break;
    case ADC1_IN15:
        enPort = PortC;
        enPin  = Pin05;
        break;
    default:
        bFlag = false;
        break;
    }
    if (true == bFlag)
    {
        PORT_Init(enPort, enPin, &stcPortInit);
    }
}

void BSP_CLK_Init(void)
{
    stc_clk_sysclk_cfg_t    stcSysClkCfg;
    stc_clk_xtal_cfg_t      stcXtalCfg;
    stc_clk_mpll_cfg_t      stcMpllCfg;
    stc_sram_config_t       stcSramConfig;

    MEM_ZERO_STRUCT(stcSysClkCfg);
    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcMpllCfg);
    MEM_ZERO_STRUCT(stcSramConfig);

    /* Set bus clk div. */
    stcSysClkCfg.enHclkDiv  = ClkSysclkDiv1;
    stcSysClkCfg.enExclkDiv = ClkSysclkDiv2;
    stcSysClkCfg.enPclk0Div = ClkSysclkDiv1;
    stcSysClkCfg.enPclk1Div = ClkSysclkDiv2;
    stcSysClkCfg.enPclk2Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk3Div = ClkSysclkDiv4;
    stcSysClkCfg.enPclk4Div = ClkSysclkDiv2;
    CLK_SysClkConfig(&stcSysClkCfg);

    /* Config Xtal and Enable Xtal */
    stcXtalCfg.enMode = ClkXtalModeOsc;
    stcXtalCfg.enDrv = ClkXtalLowDrv;
    stcXtalCfg.enFastStartup = Enable;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    /* sram init include read/write wait cycle setting */
    stcSramConfig.u8SramIdx = Sram12Idx | Sram3Idx | SramHsIdx | SramRetIdx;
    stcSramConfig.enSramRC = SramCycle2;
    stcSramConfig.enSramWC = SramCycle2;
    SRAM_Init(&stcSramConfig);

    /* flash read wait cycle setting */
    EFM_Unlock();
    EFM_SetLatency(EFM_LATENCY_5);
    EFM_Lock();

    /* MPLL config (XTAL / pllmDiv * plln / PllpDiv = 200M). */
    stcMpllCfg.pllmDiv = 1ul;
    stcMpllCfg.plln    = 50ul;
    stcMpllCfg.PllpDiv = 2ul;
    stcMpllCfg.PllqDiv = 2ul;
    stcMpllCfg.PllrDiv = 2ul;
    CLK_SetPllSource(ClkPllSrcXTAL);
    CLK_MpllConfig(&stcMpllCfg);

    /* Enable MPLL. */
    CLK_MpllCmd(Enable);
    /* Wait MPLL ready. */
    while(Set != CLK_GetFlagStatus(ClkFlagMPLLRdy))
    {
        ;
    }
    /* Switch driver ability */
    PWC_HS2HP();
    /* Switch system clock source to MPLL. */
    CLK_SetSysClkSource(CLKSysSrcMPLL);
}

void BSP_A15B03_Init(void)
{
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;

    PORT_DebugPortSetting(TDI,Disable);
    PORT_SetFunc(PortA, Pin15, Func_Gpio, Disable);
    PORT_Init(PortA, Pin15, &stcPortInit);

    PORT_DebugPortSetting(TDO_SWO,Disable);
    PORT_SetFunc(PortB, Pin03, Func_Gpio, Disable);
    PORT_Init(PortB, Pin03, &stcPortInit);
}

void BSP_LED_Init(void)
{
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;

    PORT_Init(PortC, Pin13, &stcPortInit);
    PORT_SetBits(PortC, Pin13);

    PORT_Init(PortH, Pin02, &stcPortInit);
    PORT_SetBits(PortH, Pin02);
}

void BSP_GPIO_Init(void)
{
    stc_port_init_t stcPortInit;

    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    PORT_Init(SPI_NSS_PORT, SPI_NSS_PIN, &stcPortInit);
    PORT_SetBits(SPI_NSS_PORT, SPI_NSS_PIN);

    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enPullUp = Disable;
    PORT_Init(PortB, Pin02, &stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enPullUp = Enable;
    PORT_Init(PortB, Pin10, &stcPortInit);
}

void BSP_LED_Toggle(void)
{
    PORT_Toggle(PortC, Pin13);
}

void BAT_LED_Toggle(void)
{
    PORT_Toggle(PortH, Pin02);
}

void AdcConfig(void)
{
    AdcClockConfig();
    AdcInitConfig();
    AdcChannelConfig();
}

void UsartRxErrProcess(void)
{
    if (Set == USART_GetStatus(USART_CH, UsartFrameErr))
    {
        USART_ClearStatus(USART_CH, UsartFrameErr);
    }

    if (Set == USART_GetStatus(USART_CH, UsartParityErr))
    {
        USART_ClearStatus(USART_CH, UsartParityErr);
    }

    if (Set == USART_GetStatus(USART_CH, UsartOverrunErr))
    {
        USART_ClearStatus(USART_CH, UsartOverrunErr);
    }
}

void BSP_PRINTF_PortInit(void)
{
    PORT_SetFunc(BSP_PRINTF_PORT, BSP_PRINTF_PIN, BSP_PRINTF_PORT_FUNC, Disable);
}

void Uart_Config(void)
{
    en_result_t enRet = Ok;
    uint32_t u32Fcg1Periph = PWC_FCG1_PERIPH_USART1 ;
    stc_usart_uart_init_t stcInitCfg = {
        UsartIntClkCkNoOutput,
        UsartClkDiv_1,
        UsartDataBits8,
        UsartDataLsbFirst,
        UsartOneStopBit,
        UsartParityNone,
        UsartSampleBit8,
        UsartStartBitFallEdge,
        UsartRtsEnable,
    };

    BSP_CLK_Init();
    PWC_Fcg1PeriphClockCmd(u32Fcg1Periph, Enable);
    PORT_SetFunc(USART_RX_PORT, USART_RX_PIN, USART_RX_FUNC, Disable);
    PORT_SetFunc(USART_TX_PORT, USART_TX_PIN, USART_TX_FUNC, Disable);
    enRet = USART_UART_Init(USART_CH, &stcInitCfg);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }

    enRet = USART_SetBaudrate(USART_CH, USART_BAUDRATE);
    if (enRet != Ok)
    {
        while (1)
        {
        }
    }

    USART_FuncCmd(USART_CH, UsartRx, Enable);
    USART_FuncCmd(USART_CH, UsartTx, Enable);
    DDL_PrintfInit(BSP_PRINTF_DEVICE, BSP_PRINTF_BAUDRATE, BSP_PRINTF_PortInit);
}

void Spi_Config(void)
{
    stc_spi_init_t stcSpiInit;

    MEM_ZERO_STRUCT(stcSpiInit);
    PWC_Fcg1PeriphClockCmd(SPI_UNIT_CLOCK, Enable);
    PORT_SetFunc(SPI_SCK_PORT, SPI_SCK_PIN, SPI_SCK_FUNC, Disable);
    PORT_SetFunc(SPI_MOSI_PORT, SPI_MOSI_PIN, SPI_MOSI_FUNC, Disable);
    PORT_SetFunc(SPI_MISO_PORT, SPI_MISO_PIN, SPI_MISO_FUNC, Disable);

    stcSpiInit.enClkDiv = SpiClkDiv64;
    stcSpiInit.enFrameNumber = SpiFrameNumber1;
    stcSpiInit.enDataLength = SpiDataLengthBit8;
    stcSpiInit.enFirstBitPosition = SpiFirstBitPositionMSB;
    stcSpiInit.enSckPolarity = SpiSckIdleLevelLow;
    stcSpiInit.enSckPhase = SpiSckOddSampleEvenChange;
    stcSpiInit.enReadBufferObject = SpiReadReceiverBuffer;
    stcSpiInit.enWorkMode = SpiWorkMode4Line;
    stcSpiInit.enTransMode = SpiTransFullDuplex;
    stcSpiInit.enCommAutoSuspendEn = Disable;
    stcSpiInit.enModeFaultErrorDetectEn = Disable;
    stcSpiInit.enParitySelfDetectEn = Disable;
    stcSpiInit.enParityEn = Disable;
    stcSpiInit.enParity = SpiParityEven;

    stcSpiInit.enMasterSlaveMode = SpiModeMaster;
    stcSpiInit.stcDelayConfig.enSsSetupDelayOption = SpiSsSetupDelayCustomValue;
    stcSpiInit.stcDelayConfig.enSsSetupDelayTime = SpiSsSetupDelaySck1;
    stcSpiInit.stcDelayConfig.enSsHoldDelayOption = SpiSsHoldDelayCustomValue;
    stcSpiInit.stcDelayConfig.enSsHoldDelayTime = SpiSsHoldDelaySck1;
    stcSpiInit.stcDelayConfig.enSsIntervalTimeOption = SpiSsIntervalCustomValue;
    stcSpiInit.stcDelayConfig.enSsIntervalTime = SpiSsIntervalSck6PlusPck2;

    SPI_Init(SPI_UNIT, &stcSpiInit);
    SPI_Cmd(SPI_UNIT, Enable);
}

void RF_IRQ_Init(void)
{
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);

	stcPortInit.enPinMode = Pin_Mode_In;
	stcPortInit.enPullUp = Disable;
	PORT_Init(PortB, Pin13, &stcPortInit);
}

void key_set_sf_routine(void)
{
    uint32_t para;

	para = rf_get_sf();
    para ++;
    if(para > 12)
    {
        para = 5;
    }
    dis_set_sf(para);
    OLED_Refresh_Gram();
	rf_set_sf(para);
    if(app_tx_mode)
    {
    }
    else
    {
        rf_port.set_rx();
        rf_set_mode(RF_MODE_RX);
    }
}

void key_set_bw_routine(void)
{
    uint32_t para;

    para = rf_get_bw();
    para ++;
    if(para > 9)
    {
        para = 6;
    }
    dis_set_bw(para);
    OLED_Refresh_Gram();
	rf_set_bw(para);
    if(app_tx_mode)
    {
    }
    else
    {
        rf_port.set_rx();
        rf_set_mode(RF_MODE_RX);
    }
}

void key_set_pl_routine(void)
{
    tx_len += 10;
    if(tx_len > 240)
    {
        tx_len = 10;
    }
    dis_set_pl(tx_len);
    OLED_Refresh_Gram();
}

void key_set_mode_routine(void)
{
    tx_mode ++;
    if(tx_mode > 3)
    {
        tx_mode =0;
    }
    dis_set_mode(tx_mode);
    OLED_Refresh_Gram();
	
	if(tx_carrywave_flag == true)
	{
		rf_set_carrier_wave_off();
		rf_clr_irq(REG_IRQ_TX_DONE);
		tx_carrywave_flag = false;
	}
	
    if(tx_mode == 0)
    {
        tx_pkt_max_num = 1;
    }
    else if(tx_mode == 1)
    {
        tx_pkt_max_num = 100;
    }
    else if(tx_mode == 2)
    {
        tx_pkt_max_num = 9999;
    }else{
		if(app_tx_mode)
		{
			rf_set_carrier_wave_on();
			rf_set_freq(freq_list[freq_mode] * 100000);
			rf_set_tx_power(tx_power);
			rf_set_carrier_wave_freq(freq_list[freq_mode] * 100000);
			tx_carrywave_flag = true;
		}
	}
}

void key_set_power_routine(void)
{
    uint32_t para;

    para = rf_get_tx_power();

    if(para >= 23)
    {
        para = 1;
    }
    else
    {
        para ++;
    }
	tx_power = para;
    rf_set_tx_power(para);
    SysTick_Delay(1);
    para = rf_get_tx_power();

    dis_set_txpower(para);
    OLED_Refresh_Gram();
    if(app_tx_mode)
    {
		if(tx_carrywave_flag)
		{
			rf_set_carrier_wave_off();
			rf_set_carrier_wave_on();
			rf_set_freq(freq_list[freq_mode] * 100000);
			rf_set_tx_power(tx_power);
			rf_set_carrier_wave_freq(freq_list[freq_mode] * 100000);
		}
    }
    else
    {
        rf_port.set_rx();
        rf_set_mode(RF_MODE_RX);
    }
}

void key_set_cr_routine(void)
{
    uint32_t para;

    para = rf_get_code_rate();
    para ++;
    if(para > 4)
    {
        para = 1;
    }
    dis_set_cr(para);
    OLED_Refresh_Gram();
    rf_set_code_rate(para);
    if(app_tx_mode)
    {
    }
    else
    {
        rf_port.set_rx();
        rf_set_mode(RF_MODE_RX);
    }
}

void key_set_start_tx_routine(void)
{
    static uint32_t tx_time;
    if(app_tx_mode)
    {
        tx_ing ++;
        tx_ing %= 2;

        if(tx_ing || (tx_pkt_max_num == 1))
        {
            if(tx_pkt_max_num != 1)
            {
                tx_pkt_cnt = 0;
            }
            rf_single_tx_data(tx_buf, tx_len,&tx_time);
        }
    }
}

void key_set_fq_routine(void)
{
    uint32_t para;

    freq_mode ++;
    if(freq_mode > FREQ_LIST_NUM - 1)
    {
        freq_mode = 0;
    }

    if(freq_mode == 0)
    {
        para = freq_list[freq_mode] * 100000;
    }
    else if(freq_mode == 1)
    {
        para = freq_list[freq_mode] * 100000;
    }
    else
    {
        para = freq_list[freq_mode] * 100000;
    }

    rf_set_freq(para);

    para = rf_read_freq();

    dis_set_fq((para/100000));

    OLED_Refresh_Gram();

    if(app_tx_mode)
    {
		if(tx_carrywave_flag)
		{
			rf_set_carrier_wave_off();
			rf_set_carrier_wave_on();
			rf_set_freq(freq_list[freq_mode] * 100000);
			rf_set_tx_power(tx_power);
			rf_set_carrier_wave_freq(freq_list[freq_mode] * 100000);
		}
    }
    else
    {
        rf_port.set_rx();
        rf_set_mode(RF_MODE_RX);
    }
}

void key_clear_cnt(void)
{
    tx_pkt_cnt = 0;
    rx_pkt_cnt = 0;
    dis_set_cnt( 0 );
    OLED_Refresh_Gram();
}



void key_scan(void)
{
    uint8_t i;
    uint8_t pin_state;
    uint8_t pin_state_second;
    sys_tick = SysTick_GetTick();

    if(sys_tick % 50)
    {
        return ;
    }

    for(i = 0; i < 5; i++)
    {
        pin_state = PORT_GetBit(key_list[i].port, key_list[i].pin);

        if((pin_state == RESET) && (key_list[i].first_state == SET) && (key_list[i].second_state == RESET))
        {
            keys_pressed |= (1 << i);
        }

        key_list[i].first_state = key_list[i].second_state;
        key_list[i].second_state = pin_state;
    }
    pin_state_second = PORT_GetBit(key_list[5].port, key_list[5].pin);
    if(pin_state_second == RESET)
    {
        key_second_pressed = 1;
    }
    else
    {
        key_second_pressed = 0;
    }
}

void key_event_process(void)
{
    uint32_t i = 0;

    for(i = 0; i < 5; i++)
    {
        if(keys_pressed & (1 << i))
        {
            if(key_second_pressed == 0)
            {
                if(key_list[i].func != NULL)
                {
					set_spi_state(SPI_BUSY);
                    key_list[i].func();
					set_spi_state(SPI_IDLE);
                }
            }
            else
            {
                if(key_list[i].func_second != NULL)
                {
					set_spi_state(SPI_BUSY);
                    key_list[i].func_second();
					set_spi_state(SPI_IDLE);
                }
            }

            keys_pressed = 0;
            break;
        }
    }
}

void BSP_KEY_Init(void)
{
    uint8_t i;

    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;
    PORT_DebugPortSetting(TRST,Disable);

    for (i = 0U; i < BSP_KEY_ROW_NUM; i++)
    {
        PORT_Init(BSP_KEYIN_PORT_PIN[i].port, BSP_KEYIN_PORT_PIN[i].pin, &stcPortInit);
    }
}

void dis_init(void)
{
    uint32_t para;
    uint32_t i;
    for(i = 0; i < 256; i++)
    {
        tx_buf[i] = i;
    }

    dis_set_cnt(0);
    dis_set_mode(0);

	para = rf_read_freq();
    dis_set_fq((para/100000));

//    rf_get_para(RF_PARA_TYPE_CR, &para);
//    dis_set_cr(para);

    para = rf_get_sf();
    dis_set_sf(para);

	para = rf_get_bw();
    dis_set_bw(para);

    dis_set_pl(tx_len);
	
    para = rf_get_tx_power();
    dis_set_txpower(para);

    OLED_Refresh_Gram();
}

void rf_rx_done_routine(void)
{
    rx_pkt_cnt ++;
//    DDL_Printf("RF RX:%d\r\n",rx_pkt_cnt);
    dis_set_cnt( rx_pkt_cnt );
    OLED_Refresh_Gram();
}

void rf_tx_done_routine(void)
{
    static uint32_t tx_time;
    tx_pkt_cnt ++;
    DDL_Printf("RF TX:%d\r\n",tx_pkt_cnt);
    dis_set_cnt( tx_pkt_cnt );
    OLED_Refresh_Gram();
    if((tx_pkt_cnt < tx_pkt_max_num) && tx_ing)
    {
        SysTick_Delay(100);
        rf_single_tx_data(tx_buf, tx_len,&tx_time);
    }
    else
    {
        tx_ing = 0;
    }
}

void process_rf_events(void)
{
    uint32_t flag = 0;
	uint8_t i;
    flag = rf_get_transmit_flag();
    if(flag == RADIO_FLAG_TXDONE)
    {
        DDL_Printf("REG_IRQ_TX_DONE \r\n");
        BSP_LED_Toggle();
        rf_tx_done_routine();
        rf_set_transmit_flag(RADIO_FLAG_IDLE);
    }

    flag = rf_get_recv_flag();
    if(flag == RADIO_FLAG_RXDONE)
    {
		BSP_LED_Toggle();
		rf_rx_done_routine();
		
        rf_set_recv_flag(RADIO_FLAG_IDLE);

        DDL_Printf("RxDone : SNR: %f ,RSSI: %d \r\n", RxDoneParams.Snr, RxDoneParams.Rssi);
        for(i = 0; i < RxDoneParams.Size; i++)
        {
            DDL_Printf("0x%02x ", RxDoneParams.Payload[i]);
        }
        DDL_Printf("\r\n");
        DDL_Printf("rf pkt cnt:%x\r\n", rf_read_pkt_cnt());
    }

}

void PwmCoupleChIrqCallback(void)
{
    us_u64TickCount ++;

    TIMER4_PWM_ClearIrqFlag(TIMER4_UNIT, TIMER4_PWM_CH);
}

void Timer4_Init(void)
{
    stc_port_init_t stcPortInit;
    stc_irq_regi_conf_t stcIrqRegiCfg;
    stc_timer4_pwm_init_t stcPwmInit;

    /* Clear structures */
    MEM_ZERO_STRUCT(stcPwmInit);
    MEM_ZERO_STRUCT(stcPortInit);
    MEM_ZERO_STRUCT(stcIrqRegiCfg);

    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Enable;
    PORT_Init(WAVE_IO_PORT, WAVE_IO_PIN, &stcPortInit);
    /* Enable peripheral clock */
    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM41 | PWC_FCG2_PERIPH_TIM42 | PWC_FCG2_PERIPH_TIM43, Enable);

    /* Timer4 PWM: Initialize PWM couple channel configuration structure */
    stcPwmInit.enMode = PwmDeadTimerMode;  /* Change: PwmDeadTimerMode  PwmThroughMode */
    stcPwmInit.enClkDiv = PwmPlckDiv4;
    stcPwmInit.enRtIntMaskCmd = Disable;
    stcPwmInit.enOutputState = PwmHPwmLHold;  /* change: PwmHPwmLHold  PwmHPwmLReverse  PwmHReversePwmLHold  PwmHHoldPwmLReverse */
    TIMER4_PWM_Init(TIMER4_UNIT, TIMER4_PWM_CH, &stcPwmInit);  /* Initialize PWM channel */
    TIMER4_PWM_SetFilterCountValue(TIMER4_UNIT, TIMER4_PWM_CH, TIMER4_PWM_RT_VAL);

    /* Set Timer4 PWM RT IRQ */
    MEM_ZERO_STRUCT(stcIrqRegiCfg);
    stcIrqRegiCfg.enIRQn = Int008_IRQn;
    stcIrqRegiCfg.pfnCallback = &PwmCoupleChIrqCallback;
    stcIrqRegiCfg.enIntSrc = TIMER4_RT_INT_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_14);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    /* Start pwm count */
    TIMER4_PWM_StartTimer(TIMER4_UNIT, TIMER4_PWM_CH);

}


#define RSSI_MAX_NUM  50
uint8_t rssi_num_cnt = 0;
int8_t rssi[100];

int8_t cal_rssi(void)
{
	int16_t i,sum=0,aver;
	for(i=0; i<RSSI_MAX_NUM; i++)
	{
		sum += rssi[i];
	}
	aver =  sum/RSSI_MAX_NUM + 90;
	if(aver<0)
	{
		return 0;
	}else if(aver>100)
	{
		return 100;
	}else{
		return aver;
	}
}

void Timer6_UnderFlow_CallBack(void)
{
    Timer6_tmp++;

    if(Timer6_tmp > Timer6_tmpdelay)
    {
        Timer6_tmp = 0;
		if( get_spi_state() == SPI_IDLE)
		{
			set_spi_state(SPI_BUSY);
			if(rssi_num_cnt < RSSI_MAX_NUM)
			{
				rssi[rssi_num_cnt] = rf_get_channel_rssi();
				rssi_num_cnt ++;				
			}
			set_spi_state(SPI_IDLE);
		}
    }
}

void shou_rssi_noise(void)
{
	if(rssi_num_cnt >= RSSI_MAX_NUM)
	{
		rssi_num_cnt = 0;
		
		OLED_ShowNum(20,16,cal_rssi(),4,16);
		OLED_Refresh_Gram();
	}
}

void Timer6_Cfg(void)
{
    uint16_t                         u16Period;
    uint16_t                         u16Compare;
    stc_timer6_basecnt_cfg_t         stcTIM6BaseCntCfg;
    stc_timer6_port_output_cfg_t     stcTIM6PWMxCfg;
    stc_timer6_gcmp_buf_cfg_t        stcGCMPBufCfg;
    stc_port_init_t                  stcPortInit;
    stc_irq_regi_conf_t              stcIrqRegiConf;
    stc_timer6_deadtime_cfg_t        stcDeadTimeCfg;

    MEM_ZERO_STRUCT(stcTIM6BaseCntCfg);
    MEM_ZERO_STRUCT(stcTIM6PWMxCfg);
    MEM_ZERO_STRUCT(stcGCMPBufCfg);
    MEM_ZERO_STRUCT(stcPortInit);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcDeadTimeCfg);

    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM61, Enable);

    stcTIM6BaseCntCfg.enCntMode   = Timer6CntTriangularModeA;           //Triangular wave mode
    stcTIM6BaseCntCfg.enCntDir    = Timer6CntDirUp;                     //Counter counting up
    stcTIM6BaseCntCfg.enCntClkDiv = Timer6PclkDiv1;                     //Count clock: pclk
    Timer6_Init(M4_TMR61, &stcTIM6BaseCntCfg);                          //timer6 PWM frequency, count mode and clk config

    u16Period = 0x8340u;
    Timer6_SetPeriod(M4_TMR61, Timer6PeriodA, u16Period);               //period set

    u16Compare = 0x3000u;
    Timer6_SetGeneralCmpValue(M4_TMR61, Timer6GenCompareA, u16Compare);  //Set General Compare RegisterA Value
    Timer6_SetGeneralCmpValue(M4_TMR61, Timer6GenCompareC, u16Compare);  //Set General Compare RegisterC Value as buffer register of GCMAR


    /*PWMA/PWMB output buf config*/
    stcGCMPBufCfg.bEnGcmpTransBuf = true;
    stcGCMPBufCfg.enGcmpBufTransType = Timer6GcmpPrdSingleBuf;          //Single buffer transfer
    Timer6_SetGeneralBuf(M4_TMR61, Timer6PWMA, &stcGCMPBufCfg);         //GCMAR buffer transfer set
    Timer6_SetGeneralBuf(M4_TMR61, Timer6PWMB, &stcGCMPBufCfg);         //GCMBR buffer transfer set


    stcTIM6PWMxCfg.enPortMode = Timer6ModeCompareOutput;    //Compare output function
    stcTIM6PWMxCfg.bOutEn     = true;                       //Output enable
    stcTIM6PWMxCfg.enPerc     = Timer6PWMxCompareKeep;      //PWMA port output keep former level when CNTER value match PERAR
    stcTIM6PWMxCfg.enCmpc     = Timer6PWMxCompareInv;       //PWMA port output inverse level when CNTER value match with GCMAR
    stcTIM6PWMxCfg.enStaStp   = Timer6PWMxStateSelSS;       //PWMA output status is decide by STACA STPCA when CNTER start and stop
    stcTIM6PWMxCfg.enStaOut   = Timer6PWMxPortOutLow;       //PWMA port output set low level when CNTER start
    stcTIM6PWMxCfg.enStpOut   = Timer6PWMxPortOutLow;       //PWMA port output set low level when CNTER stop
    stcTIM6PWMxCfg.enDisVal   = Timer6PWMxDisValLow;
    Timer6_PortOutputConfig(M4_TMR61, Timer6PWMA, &stcTIM6PWMxCfg);

    stcTIM6PWMxCfg.enPortMode = Timer6ModeCompareOutput;    //Compare output function
    stcTIM6PWMxCfg.bOutEn     = true;                       //Output enable
    stcTIM6PWMxCfg.enPerc     = Timer6PWMxCompareKeep;      //PWMB port output keep former level when CNTER value match PERAR
    stcTIM6PWMxCfg.enCmpc     = Timer6PWMxCompareInv;       //PWMB port output inverse level when CNTER value match with GCMBR
    stcTIM6PWMxCfg.enStaStp   = Timer6PWMxStateSelSS;       //PWMB output status is decide by STACB STPCB when CNTER start and stop
    stcTIM6PWMxCfg.enStaOut   = Timer6PWMxPortOutHigh;      //PWMB port output set high level when CNTER start
    stcTIM6PWMxCfg.enStpOut   = Timer6PWMxPortOutLow;       //PWMB port output set low level when CNTER stop
    stcTIM6PWMxCfg.enDisVal   = Timer6PWMxDisValLow;
    Timer6_PortOutputConfig(M4_TMR61, Timer6PWMB, &stcTIM6PWMxCfg);

    Timer6_SetDeadTimeValue(M4_TMR61, Timer6DeadTimUpAR, 3360u);     // Set dead time value (up count)

    stcDeadTimeCfg.bEnDeadtime     = true;  //Enable Hardware DeadTime
    stcDeadTimeCfg.bEnDtBufUp      = false; //Disable buffer transfer
    stcDeadTimeCfg.bEnDtBufDwn     = false; //Disable buffer transfer
    stcDeadTimeCfg.bEnDtEqualUpDwn = true;  //Make the down count dead time value equal to the up count dead time setting
    Timer6_ConfigDeadTime(M4_TMR61, &stcDeadTimeCfg);        // Hardware dead time function config

    /*config interrupt*/
    /* Enable timer61 under flow interrupt */
    Timer6_ConfigIrq(M4_TMR61, Timer6INTENUDF, true);

    stcIrqRegiConf.enIRQn = Int002_IRQn;                    //Register INT_TMR61_GUDF Int to Vect.No.002
    stcIrqRegiConf.enIntSrc = INT_TMR61_GUDF;               //Select Event interrupt function
    stcIrqRegiConf.pfnCallback = &Timer6_UnderFlow_CallBack; //Callback function
    enIrqRegistration(&stcIrqRegiConf);                     //Registration IRQ

    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);            //Clear Pending
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_03);//Set priority
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);                   //Enable NVIC
}

void timer6_open_ms(uint32_t ms)
{
    Timer6_tmpdelay = 3*ms;//336us*3=1008us
    Timer6_tmp = 0;
    /*start timer6*/
    Timer6_StartCount(M4_TMR61);
}

uint8_t times = 0;
void Timer0A_CallBack(void)
{
	times ++;
	if(times > 1)
	{
		times = 0;
		if( get_spi_state() == false)
		{
			set_spi_state(true);
			if(!CHECK_CAD())
			{	
				rf_refresh();
			}
			set_spi_state(false);
		}
	}
}
__IO uint16_t u16cmp = 0u;
__IO uint16_t u16cnt = 0u;
#define TMR_UNIT            (M4_TMR02)
#define TMR_INI_GCMA        (INT_TMR02_GCMA)
#define TMR_INI_GCMB        (INT_TMR02_GCMB)

#define ENABLE_TMR0()      (PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM02, Enable))

extern void Timer0A_CallBack(void);
void Timer0_Init(void)
{
	stc_tim0_base_init_t stcTimerCfg;
	stc_irq_regi_conf_t stcIrqRegiConf;
	/* Timer0 peripheral enable */
	ENABLE_TMR0();
	/*config register for channel A */
	stcTimerCfg.Tim0_CounterMode = Tim0_Async;
	stcTimerCfg.Tim0_AsyncClockSource = Tim0_XTAL32;
	stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv4;
	stcTimerCfg.Tim0_CmpValue = (uint16_t)(32000u/4u/2u - 1u);
	stcTimerCfg.Tim0_SyncClockSource = Tim0_Pclk1;
	TIMER0_BaseInit(TMR_UNIT,Tim0_ChannelA,&stcTimerCfg);

	/* Enable channel A interrupt */
	TIMER0_IntCmd(TMR_UNIT,Tim0_ChannelA,Enable);
	/* Register TMR_INI_GCMA Int to Vect.No.001 */
	stcIrqRegiConf.enIRQn = Int001_IRQn;
	/* Select I2C Error or Event interrupt function */
	stcIrqRegiConf.enIntSrc = TMR_INI_GCMA;
	/* Callback function */
	stcIrqRegiConf.pfnCallback =&Timer0A_CallBack;
	/* Registration IRQ */
	enIrqRegistration(&stcIrqRegiConf);
	/* Clear Pending */
	NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
	/* Set priority */
	NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);
	/* Enable NVIC */
	NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

	/*start timer0*/
	TIMER0_Cmd(TMR_UNIT,Tim0_ChannelA,Enable);

	/* Read counter register of channel A, need stop counter function for asynchronous mode*/
	TIMER0_Cmd(TMR_UNIT,Tim0_ChannelA,Disable);
	u16cnt = TIMER0_GetCntReg(TMR_UNIT,Tim0_ChannelA);
	u16cmp = TIMER0_GetCntReg(TMR_UNIT,Tim0_ChannelA);
	TIMER0_Cmd(TMR_UNIT,Tim0_ChannelA,Enable); 
}
#endif /* BSP_EV_HC32F460_LQFP100_V2 */

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
