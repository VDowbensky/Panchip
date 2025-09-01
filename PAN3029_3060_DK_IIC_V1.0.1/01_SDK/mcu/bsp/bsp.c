#include "bsp.h"
#include "pan_rf.h"

#ifdef AT_USING_SERVER
#include "fifo.h"
extern Fifo_t g_UartRxFifo;
#endif

uint32_t Timer6_tmp = 0;
uint32_t Timer6_tmpdelay = 0;

/**
 * @brief 系统时钟初始化
 * @note 该函数配置系统时钟为200MHz，使用外部晶振作为时钟源
 */
void BSP_ClockInit(void)
{
    stc_clk_sysclk_cfg_t stcSysClkCfg;
    stc_clk_xtal_cfg_t   stcXtalCfg;
    stc_clk_mpll_cfg_t   stcMpllCfg;
    stc_sram_config_t    stcSramConfig;

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

/**
 * @brief GPIO初始化
 * @note GPIO配置：
 *       - PB12 connect to rf reset pin
 *       - PB13 connect to rf irq pin
 *       - PA9 connect to rf cad pin
 *       - PC13 connect to led, indicate the rf tx and rx status
 *       - PB10 connect to tx or rx test mode
 */
void BSP_GpioInit(void)
{
    stc_port_init_t stcPortInit;

    MEM_ZERO_STRUCT(stcPortInit);

    /* PB12 connect to rf reset pin */
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enPullUp = Disable;
    PORT_Init(GPIO_PORT_RF_RST, GPIO_PIN_RF_RST, &stcPortInit);
    PORT_SetBits(GPIO_PORT_RF_RST, GPIO_PIN_RF_RST);

    /* PB13 Connect to led */
    PORT_Init(GPIO_PORT_LED, GPIO_PIN_LED, &stcPortInit);
    PORT_SetBits(GPIO_PORT_LED, GPIO_PIN_LED);

    /* PB13 connect to rf irq pin */
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enPullUp = Disable;
    PORT_Init(GPIO_PORT_RF_IRQ, GPIO_PIN_RF_IRQ, &stcPortInit);

    /* PA9 connect to the rf cad pin */
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enPullUp = Disable;
    PORT_Init(GPIO_PORT_RF_CAD, GPIO_PIN_RF_CAD, &stcPortInit);

    /* PB10 is used to select tx or rx test mode */
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enPullUp = Enable;
    PORT_Init(PortB, Pin10, &stcPortInit);

    /* PA15 PB03为特殊调试口，如需要配置成GPIO，请按以下方式配置 */
//    stcPortInit.enPinMode = Pin_Mode_Out;
//    stcPortInit.enPullUp = Disable;

//    PORT_DebugPortSetting(TDI,Disable);
//    PORT_SetFunc(PortA, Pin15, Func_Gpio, Disable);
//    PORT_Init(PortA, Pin15, &stcPortInit);

//    PORT_DebugPortSetting(TDO_SWO,Disable);
//    PORT_SetFunc(PortB, Pin03, Func_Gpio, Disable);
//    PORT_Init(PortB, Pin03, &stcPortInit);

//    PORT_SetFunc(PortB, Pin15, Func_Gpio, Disable);
//    PORT_Init(PortB, Pin15, &stcPortInit);
}

/**
 * @brief RF收发指示灯翻转
 */
void BSP_LedToggle(void)
{
    PORT_Toggle(GPIO_PORT_LED, GPIO_PIN_LED);
}

/**
 * @brief USART1接收中断回调函数
 */
static void UsartRxIrqCallback(void)
{
    uint16_t u16Data = USART_RecData(USART_CH);

#ifdef AT_USING_SERVER
    if(!is_fifo_full(&g_UartRxFifo))
    {
        fifo_push(&g_UartRxFifo, u16Data);
    }
#endif
}

/**
 * @brief USART1异常中断回调函数
 */
static void UsartErrIrqCallback(void)
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

/**
 * @brief USART1初始化
 * @note USART1配置：
 *       - USART1_TX  : PA12
 *       - USART1_RX  : PA11
 *       - 通信波特率：115200bps
 *       - 数据位：8位
 *       - 停止位：1位
 *       - 校验位：无
 */
void BSP_UartInit(void)
{
    en_result_t enRet = Ok;
    stc_irq_regi_conf_t stcIrqRegiCfg;
    uint32_t u32Fcg1Periph = PWC_FCG1_PERIPH_USART1;
    stc_usart_uart_init_t stcInitCfg = 
    {
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

    BSP_ClockInit();
    PWC_Fcg1PeriphClockCmd(u32Fcg1Periph, Enable);
    PORT_SetFunc(USART_RX_PORT, USART_RX_PIN, USART_RX_FUNC, Disable);
    PORT_SetFunc(USART_TX_PORT, USART_TX_PIN, USART_TX_FUNC, Disable);
    enRet = USART_UART_Init(USART_CH, &stcInitCfg);
    if (enRet != Ok)
    {
        while (1);
    }

    enRet = USART_SetBaudrate(USART_CH, USART_BAUDRATE);
    if (enRet != Ok)
    {
        while (1);
    }

    /* Set USART RX IRQ */
    stcIrqRegiCfg.enIRQn = Int002_IRQn;
    stcIrqRegiCfg.pfnCallback = &UsartRxIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART_RI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    /* Set USART RX error IRQ */
    stcIrqRegiCfg.enIRQn = Int003_IRQn;
    stcIrqRegiCfg.pfnCallback = &UsartErrIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART_EI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    USART_FuncCmd(USART_CH, UsartRx, Enable);
    USART_FuncCmd(USART_CH, UsartTx, Enable);
    USART_FuncCmd(USART_CH, UsartRxInt, Enable);
    DDL_PrintfInit(USART_CH, USART_BAUDRATE, NULL);
}

static void BSP_I2CInit(void)
{
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);

    /* 设置软I2C的SCL引脚 */
    PORT_SetFunc(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN, Func_Gpio, Disable);
    BSP_SetGpioMode(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN, Pin_Mode_Out);
    PORT_SetBits(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN);

    /* 设置软I2C的SDA引脚 */
    PORT_SetFunc(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN, Func_Gpio, Disable);
    BSP_SetGpioMode(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN, Pin_Mode_Out);
    PORT_SetBits(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN);

    /* SPI_CS置高，使用IIC总线时，须要将CS引脚拉高或者浮空 */
    PORT_SetFunc(NSS_PORT, NSS_PIN, Func_Gpio, Disable);
    BSP_SetGpioMode(NSS_PORT, NSS_PIN, Pin_Mode_Out);
    PORT_SetBits(NSS_PORT, NSS_PIN);
}

/**
 * @brief PAN3029/3060 SPI/I2C接口初始化
 */
void BSP_RFBusInit(void)
{
    BSP_I2CInit();
}

// for 1ms delay
void SysTick_IrqHandler(void)
{
    SysTick_IncTick();
}

// for 1us delay
void PwmCoupleChIrqCallback(void)
{
    us_u64TickCount++;

    TIMER4_PWM_ClearIrqFlag(TIMER4_UNIT, TIMER4_PWM_CH);
}

void BSP_TimerInit(void)
{
    stc_port_init_t stcPortInit;
    stc_irq_regi_conf_t stcIrqRegiCfg;
    stc_timer4_pwm_init_t stcPwmInit;

    /* Clear structures */
    MEM_ZERO_STRUCT(stcPwmInit);
    MEM_ZERO_STRUCT(stcPortInit);
    MEM_ZERO_STRUCT(stcIrqRegiCfg);
    
    /* Enable peripheral clock */
    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM41 | PWC_FCG2_PERIPH_TIM42 | PWC_FCG2_PERIPH_TIM43, Enable);

    /* Timer4 PWM: Initialize PWM couple channel configuration structure */
    stcPwmInit.enMode = PwmDeadTimerMode; /* Change: PwmDeadTimerMode  PwmThroughMode */
    stcPwmInit.enClkDiv = PwmPlckDiv4;
    stcPwmInit.enRtIntMaskCmd = Disable;
    stcPwmInit.enOutputState = PwmHPwmLHold; /* Change: PwmHPwmLHold  PwmHPwmLReverse  PwmHReversePwmLHold  PwmHHoldPwmLReverse */
    TIMER4_PWM_Init(TIMER4_UNIT, TIMER4_PWM_CH, &stcPwmInit);  /* Initialize PWM channel */
    TIMER4_PWM_SetFilterCountValue(TIMER4_UNIT, TIMER4_PWM_CH, TIMER4_PWM_RT_VAL);

    /* Set Timer4 PWM RT IRQ */
    MEM_ZERO_STRUCT(stcIrqRegiCfg);
    stcIrqRegiCfg.enIRQn = Int008_IRQn;
    stcIrqRegiCfg.pfnCallback = &PwmCoupleChIrqCallback;
    stcIrqRegiCfg.enIntSrc = TIMER4_RT_INT_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_15);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    /* Start pwm count */
    TIMER4_PWM_StartTimer(TIMER4_UNIT, TIMER4_PWM_CH);
}

/**
 * @brief 设置GPIO模式
 * @param[in] enPort 端口号
 * @param[in] u16Pin 引脚号
 * @param[in] mode 引脚模式
 */
void BSP_SetGpioMode(en_port_t enPort, uint16_t u16Pin, en_pin_mode_t Mode)
{
    stc_port_init_t stcPortInit;

    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Mode;
    //stcPortInit.enPullUp = Enable;
    PORT_Init(enPort, u16Pin, &stcPortInit);
}

void Cad_Irq_Callback(void)
{
    
}

void CAD_PA09_Init(void)
{
    stc_exint_config_t stcExtiConfig;
    stc_irq_regi_conf_t stcIrqRegiConf;
    stc_port_init_t stcPortInit;

    MEM_ZERO_STRUCT(stcExtiConfig);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcPortInit);
    stcExtiConfig.enExitCh = ExtiCh09;
    stcExtiConfig.enFilterEn = Enable;
    stcExtiConfig.enFltClk = Pclk3Div8;
    stcExtiConfig.enExtiLvl = ExIntRisingEdge;
    EXINT_Init(&stcExtiConfig);

    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Disable;
    PORT_Init(GPIO_PORT_RF_CAD, GPIO_PIN_RF_CAD, &stcPortInit);

    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ9;
    stcIrqRegiConf.enIRQn = Int009_IRQn;
    stcIrqRegiConf.pfnCallback = &Cad_Irq_Callback;
    enIrqRegistration(&stcIrqRegiConf);

    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_14);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
}
