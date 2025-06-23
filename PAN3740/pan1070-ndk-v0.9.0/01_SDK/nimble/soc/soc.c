/**
 *******************************************************************************
 * @file     soc.c
 * @create   2024-12-11
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "soc_api.h"
#if (!IS_BOOTLOADER && CONFIG_ENABLE_BOOTLOADER && CONFIG_APP_USE_IMAGE_HEADER)
#include "img_hdr.h"
#endif
#if BLE_EN
#include "pan_ble.h"
#endif
#if CONFIG_RTT_LOG_ENABLE
#include "SEGGER_RTT.h"
#endif
#include "app_log.h"
#include "utility.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/
#define CLK_ACT32K_TMR_EN_Pos               (0)
#define CLK_ACT32K_TMR_EN_Msk               (0x1ul << CLK_ACT32K_TMR_EN_Pos)
#define CLK_ACT32K_LL_32KCLK_SEL_Pos           (1)
#define CLK_ACT32K_LL_32KCLK_SEL_Msk           (0x1ul << CLK_ACT32K_LL_32KCLK_SEL_Pos)

/* Do not modify this definition */
#define LPTMR_CURR_CNT_ENA_REG  (0x5002000C)

/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/
uint32_t lp_int_ctrl_reg;
uint32_t rst_status_reg;
uint8_t m_chip_mac[6];

/*******************************************************************************
 * Constant Data Structure
 ******************************************************************************/
#if (!IS_BOOTLOADER && CONFIG_ENABLE_BOOTLOADER && CONFIG_APP_USE_IMAGE_HEADER)
IMG_HDR_SECTION const struct img_hdr image_header = {
    .ih_magic = IMAGE_MAGIC,
    .ih_hdr_size = IMAGE_HEADER_SIZE,
    .ih_img_size = 0,
    .ih_ver.iv_major = CONFIG_APP_IMG_VER_MAJOR,
    .ih_ver.iv_minor = CONFIG_APP_IMG_VER_MINOR,
    .ih_ver.iv_revision = CONFIG_APP_IMG_VER_REVISION,
    .ih_ver.iv_build_num = CONFIG_APP_IMG_VER_BUILD,
};
#endif

/*******************************************************************************
 * Public Function Define
 ******************************************************************************/
__attribute__((section(".ramfunc"))) void soc_busy_wait(uint32_t us)
{
    while (us--) {
#if (CONFIG_SYSTEM_CLOCK == 32)
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP();
#elif (CONFIG_SYSTEM_CLOCK == 48)
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP();
#else
#error "Unsupported system clock for soc_busy_wait()!"
#endif
    }
}

uint8_t soc_reset_reason_get(void)
{
#if 0
    SYS_PRINT("ANA->LP_INT_CTRL: 0x%08x\n", lp_int_ctrl_reg);
    SYS_PRINT("CLK->RSTSTS: 0x%08x\n", rst_status_reg);
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

/*******************************************************************************
 * Private Function Define
 ******************************************************************************/
void sys_clock_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    ANA->LP_FSYN_LDO |= 0X1;

    CLK_XthStartupConfig();
    CLK->XTH_CTRL |= CLK_XTHCTL_XTH_EN_Msk;
    CLK_WaitClockReady(CLK_SYS_SRCSEL_XTH);

#if (CONFIG_SYSTEM_CLOCK == 32)
    CLK_HCLKConfig(1);
    CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH, CLK_DPLL_OUT_64M);
#elif (CONFIG_SYSTEM_CLOCK == 48)
    CLK_HCLKConfig(0);
    CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH, CLK_DPLL_OUT_48M);
#endif
    CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);

    CLK->RCH_CTRL &= ~BIT(0);

    CLK_PCLK1Config(CONFIG_APB1_CLOCK_DIVISOR >> 1);
    CLK_PCLK2Config(CONFIG_APB2_CLOCK_DIVISOR >> 1);
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
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_APB1 | CLK_AHBPeriph_APB2, ENABLE);

    /*Basic clock for BLE*/
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_BLE_32M | CLK_AHBPeriph_BLE_32K, ENABLE);

    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_USB_AHB | CLK_AHBPeriph_USB_48M, DISABLE);
}

#define APP_POWER_TEST_EN    0
#if APP_POWER_TEST_EN
void app_power_test(OTP_STRUCT_T *otp)
{
    uint32_t tmp;
    int32_t val;

    //buck out(DCDC): default:8; FT-2
    val = (otp->m.buck_out_trim >> 1) - 2;
    if (val < 0x0) {
        val = 0x0;
    }
    tmp = ANA->LP_BUCK_3V;
    tmp &= ~(0xFul<<2);
    tmp |= (val << 2);
    ANA->LP_BUCK_3V = tmp;

    //HPLDO(DVDD) default:8;  FT-1
    val = otp->m.hp_ldo_trim - 1;
    if (val < 0x0) {
        val = 0x0;
    }
    tmp = ANA->LP_HP_LDO;
    tmp &= ~(0xFul <<3);
    tmp |= (val << 3); //~1.16V
    ANA->LP_HP_LDO = tmp;
}
#endif

static int pan10xx_hw_calib_init(void)
{
    OTP_STRUCT_T otp;

    SYS_PRINT("Try to load HW calibration data..");
    if (!SystemHwParamLoader(&otp)) {
        SYS_PRINT("\nERROR: Cannot find valid calib data in current chip!\n");
        SYS_ABORT();
    } else {
        SYS_PRINT(" DONE.\n");
        SYS_PRINT("- Chip Info         : 0x%x\n", otp.m.chip_info);
        SYS_PRINT("- Chip CP Version   : %d\n", otp.m.cp_version);
        SYS_PRINT("- Chip FT Version   : %d\n", otp.m.ft_version);
        if (otp.m.ft_version >= 2) {
            memcpy(m_chip_mac, otp.m_v2.mac_addr, 6);
            SYS_PRINT("- Chip MAC Address  : %02X%02X%02X%02X%02X%02X\n", otp.m_v2.mac_addr[0], otp.m_v2.mac_addr[1],
                otp.m_v2.mac_addr[2], otp.m_v2.mac_addr[3], otp.m_v2.mac_addr[4], otp.m_v2.mac_addr[5]);
        } else {
            memcpy(m_chip_mac, otp.m.mac_addr, 6);
            SYS_PRINT("- Chip MAC Address  : %02X%02X%02X%02X%02X%02X\n", otp.m.mac_addr[0], otp.m.mac_addr[1],
                otp.m.mac_addr[2], otp.m.mac_addr[3], otp.m.mac_addr[4], otp.m.mac_addr[5]);
        }
        SYS_PRINT("- Chip UID          : %02X%02X%02X%02X%02X%02X%02X%02X%02X\n", otp.m.uid[0], otp.m.uid[1],
            otp.m.uid[2], otp.m.uid[3], otp.m.uid[4], otp.m.uid[5], otp.m.uid[6], otp.m.uid[7], otp.m.uid[8]);

    #if APP_POWER_TEST_EN
        app_power_test(&otp);
    #endif
    }

    return 0;
}

static void dump_flash_infomation(void)
{
    SYS_PRINT("- Chip Flash UID    : ");
    for (uint32_t i = 0; i < 16; i++) {
        SYS_PRINT("%02X", flash_ids.uid[i]);
    }
    SYS_PRINT("\n- Chip Flash Size   : %ld KB (Inc. 4KB Panchip Info Area)\n", BIT(flash_ids.memory_density_id) >> 10);

#if !IS_BOOTLOADER
    // Draw and print current flash map (partitions)
    SYS_PRINT("\n- Current Flash Map :\n");

#if CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE
    SYS_PRINT("  +-------------------------+ <- Addr: 0x00000\n");
    SYS_PRINT("  |  Bootloader Partition   |\n");
    SYS_PRINT("  |        (%3d KB)         |\n", CONFIG_FLASH_PARTITION_BOOTLOADER_SIZE_KB);
#endif
#if CONFIG_FLASH_PARTITION_APP_SIZE
    SYS_PRINT("  +-------------------------+ <- Addr: 0x%05X\n", CONFIG_FLASH_PARTITION_APP_ADDR);
    SYS_PRINT("  |      App Partition      |\n");
    SYS_PRINT("  |        (%3d KB)         |\n", CONFIG_FLASH_PARTITION_APP_SIZE_KB);
#endif
#if CONFIG_FLASH_PARTITION_APP_BACKUP_SIZE
    SYS_PRINT("  +-------------------------+ <- Addr: 0x%05x\n", CONFIG_FLASH_PARTITION_APP_BACKUP_ADDR);
    SYS_PRINT("  |  App Backup Partition   |\n");
    SYS_PRINT("  |        (%3d KB)         |\n", CONFIG_FLASH_PARTITION_APP_BACKUP_SIZE_KB);
#endif
#if CONFIG_FLASH_PARTITION_KVSTORE_SIZE
    SYS_PRINT("  +-------------------------+ <- Addr: 0x%05X\n", CONFIG_FLASH_PARTITION_KVSTORE_ADDR);
    SYS_PRINT("  |    KVStore Partition    |\n");
    SYS_PRINT("  |        (%3d KB)         |\n", CONFIG_FLASH_PARTITION_KVSTORE_SIZE_KB);
#endif
#if CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE
    SYS_PRINT("  +-------------------------+ <- Addr: 0x%05X\n", CONFIG_FLASH_PARTITION_USER_CUSTOM_ADDR);
    SYS_PRINT("  |  User Custom Partition  |\n");
    SYS_PRINT("  |        (%3d KB)         |\n", CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE_KB);
#endif
    SYS_PRINT("  +-------------------------+ <- Addr: 0x%05X\n", CONFIG_FLASH_PARTITION_CHIP_INFO_ADDR);
    SYS_PRINT("  |    Panchip Info Area    |\n");
    SYS_PRINT("  |     (4 KB, Hidden)      |\n");
    SYS_PRINT("  +-------------------------+ <- End : 0x%x (%d KB)\n", CONFIG_FLASH_PARTITION_CHIP_INFO_ADDR + 0x1000, (CONFIG_FLASH_PARTITION_CHIP_INFO_ADDR + 0x1000) >> 10);
#endif // !IS_BOOTLOADER

    if (BIT(flash_ids.memory_density_id) != CONFIG_FLASH_SIZE + 0x1000) {
        SYS_PRINT("\n  WARNING: Detected chip flash size does not match the CONFIG_FLASH_SIZE in sdk_config.h!\n");
    }

    SYS_PRINT("\n");
}

__WEAK void sleep_timer0_handler(void)
{
    /* This function can be overridden in application */
    SYS_PRINT("%s in..\n", __func__);
}

__WEAK void sleep_timer1_handler(void)
{
    /* This function can be overridden in application */
    SYS_PRINT("%s in..\n", __func__);
}

__WEAK void sleep_timer2_handler(void)
{
    /* This function can be overridden in application */
    SYS_PRINT("%s in..\n", __func__);
}

__WEAK void sleep_timer_post_irq_handler(void)
{
    /* This function can be overridden in application */
}

CONFIG_RAM_CODE void SLPTMR_IRQHandler(void)
{
    PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_SLPTMR_IRQ, 1);

    /* Handle os clock timeout */
    if (ANA->LP_INT_CTRL & ANAC_INT_SLEEP_TMR0_Msk) {
        /*
         * Clear sleep timer 0 interrupt flags (write 1 to clear) in this register,
         * and retain other settings / flags.
         */
        ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_SLEEP_TMR0_Msk)
            & ~(ANAC_INT_SLEEP_TMR1_Msk | ANAC_INT_SLEEP_TMR2_Msk
            | ANAC_INT_DP_FLAG_Msk | ANAC_INT_STANDBY_M1_FLAG_Msk
            | ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);
        /* Execute slptmr0 handler */
        sleep_timer0_handler();
    }

    /* Handle custom sleep timer1 event */
    if (ANA->LP_INT_CTRL & ANAC_INT_SLEEP_TMR1_Msk) {
        /*
         * Clear sleep timer 1 interrupt flags (write 1 to clear) in this register,
         * and retain other settings / flags.
         */
        ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_SLEEP_TMR1_Msk)
            & ~(ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_SLEEP_TMR2_Msk
            | ANAC_INT_DP_FLAG_Msk | ANAC_INT_STANDBY_M1_FLAG_Msk
            | ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);
        /* Execute slptmr1 handler */
        sleep_timer1_handler();
    }

    /* Handle custom sleep timer2 event */
    if (ANA->LP_INT_CTRL & ANAC_INT_SLEEP_TMR2_Msk) {
        /*
         * Clear sleep timer 2 interrupt flags (write 1 to clear) in this register,
         * and retain other settings / flags.
         */
        ANA->LP_INT_CTRL = (ANA->LP_INT_CTRL | ANAC_INT_SLEEP_TMR2_Msk)
            & ~(ANAC_INT_SLEEP_TMR0_Msk | ANAC_INT_SLEEP_TMR1_Msk
            | ANAC_INT_DP_FLAG_Msk | ANAC_INT_STANDBY_M1_FLAG_Msk
            | ANAC_INT_STANDBY_M0_FLAG_Msk | ANAC_INT_SRAM_RET_FLAG_Msk);
        /* Execute slptmr2 handler */
        sleep_timer2_handler();
    }

    sleep_timer_post_irq_handler();

    PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_SLPTMR_IRQ, 0);
}

void pan10xx_platform_init()
{
#if CONFIG_VECTOR_REMAP_TO_RAM
    static __ALIGNED(256) uint32_t ram_vector[64] = {0};
    extern uint32_t __Vectors;
    memcpy((void*)ram_vector, &__Vectors, 256);
    ANA->CPU_ADDR_REMAP_CTRL = ((uint32_t)ram_vector >> 8) | BIT(31);
#endif /* CONFIG_VECTOR_REMAP_TO_RAM */

    pan10xx_hw_calib_init();

#if CONFIG_FLASH_LDO_EN
    ANA->LP_HP_LDO &= ~ANAC_HPLDO_FLASHLDO_BP_Msk_3v;
#else
    ANA->LP_HP_LDO |= ANAC_HPLDO_FLASHLDO_BP_Msk_3v;
#endif /* CONFIG_FLASH_LDO_EN */

    // Ensure SoC LP configure is sleep mode (default state)
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    LP_SetSleepMode(ANA, LP_MODE_SEL_SLEEP_MODE);

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
#if CONFIG_FORCE_CALIB_RCL_CLK
    /* Calibrate RCL to 32K Hz */
    calibrate_rcl_clk(32000);
#endif /* CONFIG_FORCE_CALIB_RCL_CLK */
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

    /* Store value in rst status reg and lp int ctrl reg for later possible soc_reset_reason_get() use */
    rst_status_reg = CLK->RSTSTS;
    lp_int_ctrl_reg = ANA->LP_INT_CTRL;

    /* Clear status registers for next time detecting. And would not clear it in bootloader so that these
       flags can also be obtained in app. */
#if !IS_BOOTLOADER
    CLK->RSTSTS = CLK->RSTSTS;
    ANA->LP_INT_CTRL = ANA->LP_INT_CTRL;
#endif

    /* Enable sleeptimer counter */
    ANA->LP_FL_CTRL_3V |= ANAC_FL_SLEEP_CNT_EN_Msk;
    (*(volatile uint32_t *)LPTMR_CURR_CNT_ENA_REG) |= BIT1;
    /* Clear configured time of sleep timers */
    ANA->LP_SPACING_TIME0 = 0;
    ANA->LP_SPACING_TIME1 = 0;
    ANA->LP_SPACING_TIME2 = 0;
    /* Enable sleeptimer interrupt */
    ANA->LP_INT_CTRL |= ANAC_INT_SLEEP_TMR_INT_EN_Msk;
    NVIC_EnableIRQ(SLPTMR_IRQn);
    NVIC_SetPriority(SLPTMR_IRQn, 3);   // Lowest prio
}

__WEAK void HAL_DriverInit(void)
{
    // This weak function impl would be replaced by corresponding function in pan_hal.c
    // when hal driver is added in project build.
}

void $Sub$$main(void)
{
    /* system clock initialization */
    sys_clock_Init();

#if CONFIG_STARTUP_LONG_DELAY
    soc_busy_wait(1000*1000);
#endif

    /* Init IO Timing Track Pins */
#if CONFIG_IO_TIMING_TRACK
    track_pin_init();
#endif // CONFIG_IO_TIMING_TRACK

    /* Init Logging System */
#if PAN_LOG_ENABLE
#if CONFIG_UART_LOG_ENABLE
    debug_uart_init();
#endif // CONFIG_UART_LOG_ENABLE
#if CONFIG_RTT_LOG_ENABLE
    SEGGER_RTT_Init();
#endif // CONFIG_RTT_LOG_ENABLE
#if BLE_EN
#if (CONFIG_UART_LOG_ENABLE || CONFIG_RTT_LOG_ENABLE)
#if BLE_LL_LOG_PRINT_MODE == 0
    pan_misc_register_print(NULL);
#elif BLE_LL_LOG_PRINT_MODE == 1
    pan_misc_register_print(vprintf);
#elif BLE_LL_LOG_PRINT_MODE == 2
    extern void vprintk(const char *fmt, va_list ap);   // Declares in printk.h
    pan_misc_register_print(vprintk);
#endif
#endif // CONFIG_UART_LOG_ENABLE || CONFIG_RTT_LOG_ENABLE
#endif // BLE_EN
#endif // PAN_LOG_ENABLE

#if (!IS_BOOTLOADER && CONFIG_ENABLE_BOOTLOADER)
    SYS_PRINT("Application in..\n\n");
#endif

    /* Platform misc configurations (32K Clock, FT load, Flash LDO) */
    pan10xx_platform_init();

    dump_flash_infomation();

    /* init hal driver when needed */
    HAL_DriverInit();

    /* PM initialization */
#if CONFIG_PM
    extern void soc_pm_init(void);
    soc_pm_init();
#endif

    /* get clock frequence, no matter rcl or xtl */
#if ((CONFIG_LOW_SPEED_CLOCK_SRC == 0) || (CONFIG_LOW_SPEED_CLOCK_SRC == 1))
    clktrim_measure_32k_clk(1000);
#endif

#if (!IS_BOOTLOADER && CONFIG_ENABLE_BOOTLOADER && CONFIG_APP_USE_IMAGE_HEADER)
    struct img_hdr *image_header_p = (void *)CONFIG_FLASH_PARTITION_APP_ADDR;
    if (image_header_p->ih_magic == IMAGE_MAGIC) {
        SYS_PRINT("APP image header check passed, image version: %d.%d.%d.%d\n",
            image_header_p->ih_ver.iv_major,
            image_header_p->ih_ver.iv_minor,
            image_header_p->ih_ver.iv_revision,
            image_header_p->ih_ver.iv_build_num
        );
    }
#endif /* !IS_BOOTLOADER && CONFIG_ENABLE_BOOTLOADER && CONFIG_APP_USE_IMAGE_HEADER */

#if CONFIG_OS_EN
    extern __NO_RETURN void os_schedule_main(void);
    os_schedule_main();
#else
    extern int $Super$$main(void);
    $Super$$main();
#endif /* CONFIG_OS_EN */
}
