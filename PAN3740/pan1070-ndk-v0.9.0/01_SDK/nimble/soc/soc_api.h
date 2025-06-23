#ifndef SOC_API_H_
#define SOC_API_H_

#include <stdint.h>
#include <Panseries.h>
#include "sdk_config.h"

#if CONFIG_KEEP_FLASH_POWER_IN_LP_MODE
    #define LP_DLY_TICK             6     // unit: 32k tick
#else
    #define LP_DLY_TICK             7     // unit: 32k tick
#endif

enum _soc_reset_reason {
    SOC_RST_REASON_CHIP_RESET               = 0,
    SOC_RST_REASON_PIN_RESET                = 1,
    SOC_RST_REASON_WDT_RESET                = 2,
    SOC_RST_REASON_LVR_RESET                = 3,
    SOC_RST_REASON_BOD_RESET                = 4,
    SOC_RST_REASON_SYS_RESET                = 5,
    SOC_RST_REASON_POR_RESET                = 6,
    SOC_RST_REASON_STBM0_EXTIO_WAKEUP       = 10,    /* P00/P01/P02 */
    SOC_RST_REASON_STBM1_SLPTMR0_WAKEUP     = 11,
    SOC_RST_REASON_STBM1_SLPTMR1_WAKEUP     = 12,
    SOC_RST_REASON_STBM1_SLPTMR2_WAKEUP     = 13,
    SOC_RST_REASON_STBM1_GPIO_WAKEUP        = 14,    /* All GPIOs */
    SOC_RST_REASON_UNKNOWN_RESET            = 255
};

enum _stbm1_wakeup_src {
    STBM1_WAKEUP_SRC_GPIO           = BIT0,
    STBM1_WAKEUP_SRC_SLPTMR         = BIT1,
};

enum _stbm1_retention_sram {
    STBM1_RETENTION_SRAM_NONE       = 0,
    STBM1_RETENTION_SRAM_BLOCK0     = BIT0, /* 32 KB */
    STBM1_RETENTION_SRAM_BLOCK1     = BIT1, /* 16 KB */
    STBM1_RETENTION_SRAM_LL         = BIT4, /* 8  KB */
    STBM1_RETENTION_SRAM_ALL        = (BIT0 | BIT1 | BIT4),
};

extern uint32_t clk_32k_freq;

/*******************************************************************************
 * 32K SleepTimer/LpTimer API
 ******************************************************************************/
/**
 * @brief Read the current counter of LP Timer.
 *
 * This routine returns the current cycle count, as measured by the SoC's
 * hardware LP TImer.
 *
 * @return Current hardware LP Timer clock up-counter (in cycles).
 */
__STATIC_FORCEINLINE uint32_t soc_lptmr_cycle_get(void)
{
    return (*(volatile uint32_t *)(0x50020014));
}

/**
 * @brief Get current 32K low speed clock freqency.
 *
 * @return Current 32K low speed clock freqency in Hz.
 */
__STATIC_FORCEINLINE uint32_t soc_32k_clock_freq_get(void)
{
#if 0
    return (CLK->CLK_TOP_CTRL_3V & CLK_TOPCTL_32K_CLK_SEL_Msk_3v ? 32768 : 32000);
#else
    return clk_32k_freq;
#endif
}

/**
 * @brief Read the current counter of LP Timer.
 *
 * This routine returns the current cycle count, as measured by the SoC's
 * hardware LP TImer.
 *
 * @return Current hardware LP Timer clock up-counter (in cycles).
 */
__STATIC_FORCEINLINE uint32_t soc_lptmr_uptime_get_ms(void)
{
    return soc_lptmr_cycle_get() * 1000ull / soc_32k_clock_freq_get();
}

/*******************************************************************************
 * 32K ClockTrim APIs
 ******************************************************************************/
/**
 * @brief : 32K RCL calibration 
 * @param : expected_freq    expect frequence
 */
void calibrate_rcl_clk(uint32_t expected_freq);

/**
 * @brief : 32K Clock (RCL/XTL) frequence measure
 * @param : dst_clk_cnt     the number of 32k cycle 
 * @return: return 32k RCL frequence
 */
uint32_t clktrim_measure_32k_clk(uint32_t dst_clk_cnt);

/**
 * @brief : Enable ClkTrim module and start 32K Clock (RCL/XTL) measuring
 * @param : dst_clk_cnt     the number of 32k cycle 
 * @return: None
 */
void clktrim_32k_clk_measure_start(uint32_t dst_clk_cnt);

/**
 * @brief : Get last 32k Clock measure result and disable ClkTrim module
 * @return: 32k clock measure count (based on 32M XTH clock)
 */
uint32_t clktrim_32k_clk_measure_value_get(void);



/*******************************************************************************
 * SoC Low Power APIs
 ******************************************************************************/
/**
 * @brief : Get GPIO wakeup pin after waking up from standby mode 1
 * @return: Numeric code to indicate wakeup IO
 */
extern uint32_t soc_stbm1_gpio_wakeup_src_get(void);

/**
 * @brief : Enter SoC standby mode 1
 * @param : wakeup_src      Wakeup source, refer to the enum _stbm1_wakeup_src
 * @param : retention_sram  Retention SRAM(s) in standby mode 1, refer to the enum _stbm1_retention_sram
 * @return: None
 */
extern void soc_enter_standby_mode_1(uint32_t wakeup_src, uint32_t retention_sram);

/**
 * @brief : Enter SoC standby mode 0
 * @return: None
 */
extern void soc_enter_standby_mode_0(void);

/*******************************************************************************
 * Misc SoC APIs
 ******************************************************************************/
/**
 * @brief : Busy wait until timeout
 * @param : us    Timeout in microsecond
 */
extern void soc_busy_wait(uint32_t us);

/**
 * @brief : Get SoC Reset Reason
 * @return: Numeric code to indicate reset reason, refer to the enum _soc_reset_reason
 */
extern uint8_t soc_reset_reason_get(void);

#endif // SOC_API_H_

