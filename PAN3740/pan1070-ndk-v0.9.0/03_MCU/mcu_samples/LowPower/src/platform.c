/*******************************************************************************
 * @note      Copyright (C) 2024 Shanghai Panchip Microelectronics Co., Ltd.
 *            All rights reserved.
 * @file      platform.c
 * @brief     Source file of PAN10xx SoC platform init flow.
*******************************************************************************/
#include "PanSeries.h"
#include "platform.h"

void SOC_SystemClockInit(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    // Enable XTH Clock
    ANA->LP_FSYN_LDO |= 0X1;
    CLK_XthStartupConfig();
    CLK->XTH_CTRL |= CLK_XTHCTL_XTH_EN_Msk;
    CLK_WaitClockReady(CLK_SYS_SRCSEL_XTH);

    // Enable DPLL and switch system clock to DPLL
#if (CONFIG_SYSTEM_CLOCK == 32)
    CLK_HCLKConfig(1);
    CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH, CLK_DPLL_OUT_64M);
#elif (CONFIG_SYSTEM_CLOCK == 48)
    CLK_HCLKConfig(0);
    CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH, CLK_DPLL_OUT_48M);
#endif
    CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);

    // Disable RCH Clock
    CLK->RCH_CTRL &= ~BIT(0);

    // Configure APB clock divisor
    CLK_PCLK1Config(CONFIG_APB1_CLOCK_DIVISOR >> 1);
    CLK_PCLK2Config(CONFIG_APB2_CLOCK_DIVISOR >> 1);

    /*
     * Note that all clocks on APB are disabled by default after SoC power up, and
     * the default AHB clocks enabled after SoC power up are:
     * eFuse/ROM/RCC_AHB/Systick/GPIO.
     * Here we disable ROM clock as these module is not commonly used. We should 
     * re-enabled them once we use later.
     * Here We also enable APB1 and APB2 clock path to make sure modules on APB can
     * be easily enabled by each peripheral driver by enabling their clock enable
     * bits on APB bus.
     */
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_ROM, DISABLE);
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_APB1 | CLK_AHBPeriph_APB2, ENABLE);

    // Enable low speed clock selected in configuration.h
#if (CONFIG_LOW_SPEED_CLOCK_SRC == 0)  /*LP Clock from RCL, 32000 HZ default*/
    /* Enable RCL */
    CLK->RCL_CTRL_3V |= CLK_RCLCTL_RC32K_EN_Msk_3v;
    /* Wait for stable */
    while (!(CLK->RCL_CTRL_3V & CLK_STABLE_STATUS_Msk));
    /* Select RCL as SoC 32K clock source */
    CLK->CLK_TOP_CTRL_3V &= ~CLK_TOPCTL_32K_CLK_SEL_Msk_3v;
    /* Disable lp xtl src */
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_XTAL32K_EN_Msk_3v;
    ANA->LP_FL_CTRL_3V |= ANAC_FL_RC32K_EN_Msk_3v;
    /* Delay more than two 32K clock cycles */
    SYS_delay_10nop(250);
    /* Disable XTL */
    CLK->XTL_CTRL_3V &= ~CLK_XTLCTL_XTL_EN_Msk_3v;
#elif ( CONFIG_LOW_SPEED_CLOCK_SRC == 2)  /*LP Clock from ACT32K,32000 HZ default*/
	ANA->ACT_32K_CTRL |= (CLK_ACT32K_TMR_EN_Msk | CLK_ACT32K_LL_32KCLK_SEL_Msk);  /*LP Clock from XTL,32768 HZ default*/
#else /*LP Clock from XTL, 32768 HZ */
#ifdef XTL_SLOW_SETUP
    /* Enable XTL clock */
    CLK->XTL_CTRL_3V |= CLK_XTLCTL_XTL_EN_Msk_3v;
    while(!(CLK->XTL_CTRL_3V & CLK_XTLCTL_STABLE_Msk));
#else // Quick Setup
    CLK->MEAS_CLK_CTRL = (CLK->MEAS_CLK_CTRL & ~(CLK_MEASCLK_XTH_DIV_Msk)) | (0x1e8 << CLK_MEASCLK_XTH_DIV_Pos);
    CLK->XTL_CTRL_3V = (CLK->XTL_CTRL_3V & ~(CLK_XTLCTL_DELAY_Msk_3v)) | (3 << CLK_XTLCTL_DELAY_Pos_3v);
    /* Enable quick startup */
    CLK->MEAS_CLK_CTRL |= CLK_MEASCLK_XTL_QUICK_EN_Msk;
    SYS_SET_MFP(P2, 0, XTL_C1_CLK);
    SYS_SET_MFP(P2, 1, XTL_C2_CLK);
    /* Enable XTL clock */
    CLK->XTL_CTRL_3V |= CLK_XTLCTL_XTL_EN_Msk_3v;
    /* Delay a while */
    SYS_delay_10nop(5000);
    /* Disable quick startup */
    CLK->MEAS_CLK_CTRL &= ~CLK_MEASCLK_XTL_QUICK_EN_Msk;
    SYS_SET_MFP(P2, 0, GPIO);
    SYS_SET_MFP(P2, 1, GPIO);
    /* Wait for stable */
    while (!(CLK->XTL_CTRL_3V & CLK_STABLE_STATUS_Msk)) {
        /* Busy wait */
        __NOP();
    }
#endif /* XTL_SLOW_SETUP */
    /* Select XTL as current 32K clock */
    CLK->CLK_TOP_CTRL_3V |= CLK_TOPCTL_32K_CLK_SEL_Msk_3v;
    /* Delay more than two 32K clock cycles */
    SYS_delay_10nop(250);
    /* Disable lp rcl src */
    ANA->LP_FL_CTRL_3V |= ANAC_FL_XTAL32K_EN_Msk_3v;
    ANA->LP_FL_CTRL_3V &= ~ANAC_FL_RC32K_EN_Msk_3v;
    /* Disable RCL clock */
    CLK->RCL_CTRL_3V &= ~BIT(0);
#endif /* CONFIG_LOW_SPEED_CLOCK_SRC */
}

#if CONFIG_UART_LOG_ENABLE
void SOC_LogUartInit(void)
{
    UART_InitTypeDef Init_Struct = {
        .UART_BaudRate = CONFIG_LOG_UART_BAUDRATE,
        .UART_LineCtrl = Uart_Line_8n1,
    };

#if (CONFIG_LOG_UART_RX_PIN == 0)
    SYS_SET_MFP(P0, 6, UART0_RX);
    GPIO_EnableDigitalPath(P0, BIT6);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 1)
    SYS_SET_MFP(P1, 2, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT2);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 2)
    SYS_SET_MFP(P1, 5, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT5);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 3)
    SYS_SET_MFP(P1, 7, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT7);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 4)
    SYS_SET_MFP(P0, 0, UART1_RX);
    GPIO_EnableDigitalPath(P0, BIT0);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 5)
    SYS_SET_MFP(P0, 7, UART1_RX);
    GPIO_EnableDigitalPath(P0, BIT7);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 6)
    SYS_SET_MFP(P2, 4, UART1_RX);
    GPIO_EnableDigitalPath(P2, BIT4);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 7)
    SYS_SET_MFP(P3, 0, UART1_RX);
    GPIO_EnableDigitalPath(P3, BIT0);
#endif

#if (CONFIG_LOG_UART_TX_PIN == 0)
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);
    SYS_SET_MFP(P0, 5, UART0_TX);
    UART_Init(UART0, &Init_Struct);
    UART_EnableFifo(UART0);
#endif
#if (CONFIG_LOG_UART_TX_PIN == 1)
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);
    SYS_SET_MFP(P1, 1, UART0_TX);
    UART_Init(UART0, &Init_Struct);
    UART_EnableFifo(UART0);
#endif
#if (CONFIG_LOG_UART_TX_PIN == 2)
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);
    SYS_SET_MFP(P1, 6, UART0_TX);
    UART_Init(UART0, &Init_Struct);
    UART_EnableFifo(UART0);
#endif
#if (CONFIG_LOG_UART_TX_PIN == 3)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P0, 1, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
#if (CONFIG_LOG_UART_TX_PIN == 4)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P1, 0, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
#if (CONFIG_LOG_UART_TX_PIN == 5)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P1, 2, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
#if (CONFIG_LOG_UART_TX_PIN == 6)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P2, 5, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
#if (CONFIG_LOG_UART_TX_PIN == 7)
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    SYS_SET_MFP(P3, 1, UART1_TX);
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);
#endif
}

void SOC_DisableLogUartRxPath(void)
{
    /* Wait until all UART data sending done before entering low power mode */
#if CONFIG_LOG_UART_RX_PIN < 4
    while (!(UART_GetLineStatus(UART0) & UART_LINE_TXSR_EMPTY)) {
        /* Busy wait */
    }
#else
    while (!(UART_GetLineStatus(UART1) & UART_LINE_TXSR_EMPTY)) {
        /* Busy wait */
    }
#endif

    /*
     * Reset UART PINs to GPIO function and disable digital input path of UART Rx PIN
     * to avoid possible current leakage.
     */
#if (CONFIG_LOG_UART_RX_PIN == 0)
    GPIO_DisableDigitalPath(P0, BIT6);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 1)
    GPIO_DisableDigitalPath(P1, BIT2);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 2)
    GPIO_DisableDigitalPath(P1, BIT5);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 3)
    SYS_SET_MFP(P1, 7, GPIO);
    GPIO_DisableDigitalPath(P1, BIT7);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 4)
    GPIO_DisableDigitalPath(P0, BIT0);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 5)
    GPIO_DisableDigitalPath(P0, BIT7);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 6)
    GPIO_DisableDigitalPath(P2, BIT4);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 7)
    GPIO_DisableDigitalPath(P3, BIT0);
#endif
}

void SOC_ReenableLogUartRxPath(void)
{
    /* Resume UART PIN Configurations to reenable Log UART function */
#if (CONFIG_LOG_UART_RX_PIN == 0)
    GPIO_EnableDigitalPath(P0, BIT6);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 1)
    GPIO_EnableDigitalPath(P1, BIT2);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 2)
    GPIO_EnableDigitalPath(P1, BIT5);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 3)
    SYS_SET_MFP(P1, 7, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT7);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 4)
    GPIO_EnableDigitalPath(P0, BIT0);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 5)
    GPIO_EnableDigitalPath(P0, BIT7);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 6)
    GPIO_EnableDigitalPath(P2, BIT4);
#endif
#if (CONFIG_LOG_UART_RX_PIN == 7)
    GPIO_EnableDigitalPath(P3, BIT0);
#endif
}
#endif // CONFIG_UART_LOG_ENABLE

static int SOC_HwCalibDataInit(void)
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
    }
    SYS_TEST("- Chip Flash UID    : ");
    for (uint32_t i = 0; i < 16; i++) {
        SYS_TEST("%02X", flash_ids.uid[i]);
    }
    SYS_TEST("\n- Chip Flash Size   : %ld KB\n", BIT(flash_ids.memory_density_id) >> 10);

    return 0;
}

uint8_t SOC_ResetReasonGet(void)
{
    /* Store value in rst status reg and lp int ctrl reg for later possible soc_reset_reason_get() use */
    uint32_t rst_status_reg = CLK->RSTSTS;
    uint32_t lp_int_ctrl_reg = ANA->LP_INT_CTRL;
    /* Clear status registers for next time detecting */
    CLK->RSTSTS = CLK->RSTSTS;
    ANA->LP_INT_CTRL = ANA->LP_INT_CTRL;

#if 0
    SYS_TEST("ANA->LP_INT_CTRL: 0x%08x\n", lp_int_ctrl_reg);
    SYS_TEST("CLK->RSTSTS: 0x%08x\n", rst_status_reg);
#endif
    /* Check standby mode int flags to detect standby mode wakeup */
    if (lp_int_ctrl_reg & ANAC_INT_STANDBY_M1_FLAG_Msk) {
        /* Check lptmr wakeup flag */
        if (lp_int_ctrl_reg & ANAC_INT_SLEEP_TMR0_Msk) {
            return SOC_RST_REASON_STBM1_SLPTMR0_WAKEUP;
        } else if (lp_int_ctrl_reg & ANAC_INT_SLEEP_TMR1_Msk) {
            return SOC_RST_REASON_STBM1_SLPTMR1_WAKEUP;
        } else if (lp_int_ctrl_reg & ANAC_INT_SLEEP_TMR2_Msk) {
            return SOC_RST_REASON_STBM1_SLPTMR2_WAKEUP;
        } else {
            return SOC_RST_REASON_STBM1_GPIO_WAKEUP;
        }
    } else if (lp_int_ctrl_reg & ANAC_INT_STANDBY_M0_FLAG_Msk) {
        return SOC_RST_REASON_STBM0_EXTIO_WAKEUP;
    }

    /* Check common reset status flags */
    if (rst_status_reg & BIT0) {
        return SOC_RST_REASON_CHIP_RESET;
    } else if (rst_status_reg & BIT1) {
        return SOC_RST_REASON_PIN_RESET;
    } else if (rst_status_reg & BIT2) {
        return SOC_RST_REASON_WDT_RESET;
    } else if (rst_status_reg & BIT3) {
        return SOC_RST_REASON_LVR_RESET;
    } else if (rst_status_reg & BIT4) {
        return SOC_RST_REASON_BOD_RESET;
    } else if (!(ANA->LP_FL_CTRL_3V & BIT6)) {
        /* (Workaround) Re-set the additional reserved indication flag after use */
        ANA->LP_FL_CTRL_3V |= BIT6;
        return SOC_RST_REASON_SYS_RESET;
    } else if (rst_status_reg & BIT6) {
        return SOC_RST_REASON_POR_RESET;
    }

    return SOC_RST_REASON_UNKNOWN_RESET;
}

void SOC_PlatformInit(void)
{
    // Init essential system clocks
    SOC_SystemClockInit();

    // Init log uart if needed
#if CONFIG_UART_LOG_ENABLE
    SOC_LogUartInit();
#endif

    // Load soc hw calibration parameters
    SOC_HwCalibDataInit();

    // Clear phy rxlna icore register (R0062) to avoid power leakage in low power mode
    PHY_SingleRegWrite(0x62, 0x00);

#if CONFIG_FLASH_LDO_EN
    ANA->LP_HP_LDO &= ~ANAC_HPLDO_FLASHLDO_BP_Msk_3v;
#else
    ANA->LP_HP_LDO |= ANAC_HPLDO_FLASHLDO_BP_Msk_3v;
#endif /* CONFIG_FLASH_LDO_EN */
}
