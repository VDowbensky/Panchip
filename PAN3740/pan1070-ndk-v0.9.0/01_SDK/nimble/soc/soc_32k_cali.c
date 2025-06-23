/**
 *******************************************************************************
 * @file     soc_32k_cali.c
 * @create   2024-12-11
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "soc_api.h"

/* For RCL Clock test */
//#define PRINT_RCL_FREQ_BEFORE_AND_AFTER_CALIBRATE

#define DST_CLK_CNT     100     /* measure 100 clock-32k period */

#if (CONFIG_LOW_SPEED_CLOCK_SRC == 1)
uint32_t clk_32k_freq = 32768;
#else
uint32_t clk_32k_freq = 32000;
#endif


uint32_t clktrim_measure_32k_clk(uint32_t dst_clk_cnt)
{
    uint32_t counter_result;
    uint32_t calib_freq;

    CLK_EnableClkTrim(ENABLE);
    CLK_SelectClkTrimSrc(CLKTRIM_CALC_CLK_SEL_32K);

    TRIM_SetCalCnt(TRIM, dst_clk_cnt);

    TRIM_StartTuning(TRIM, TRIM_MEASURE_TUNING_EN_Msk);
    while (!TRIM_IsIntStatusOccured(TRIM, TRIM_FLAG_MEASURE_STOP_Msk)) {
        /* busy-wait */
    }
    TRIM_ClearIntStatusMsk(TRIM, TRIM_FLAG_MEASURE_STOP_Msk);
    counter_result = TRIM_GetRealCnt(TRIM);

    calib_freq = ((uint64_t)FREQ_32MHZ * (uint64_t)dst_clk_cnt) / counter_result;
    clk_32k_freq = calib_freq;
    CLK_EnableClkTrim(DISABLE);

    return calib_freq;
}

CONFIG_RAM_CODE void clktrim_32k_clk_measure_start(uint32_t dst_clk_cnt)
{
    CLK_EnableClkTrim(ENABLE);
    CLK_SelectClkTrimSrc(CLKTRIM_CALC_CLK_SEL_32K);
    TRIM_SetCalCnt(TRIM, dst_clk_cnt);

    TRIM_StartTuning(TRIM, TRIM_MEASURE_TUNING_EN_Msk);
}

CONFIG_RAM_CODE uint32_t clktrim_32k_clk_measure_value_get(void)
{
    uint32_t counter_result;

    while (!TRIM_IsIntStatusOccured(TRIM, TRIM_FLAG_MEASURE_STOP_Msk)) {
        /* busy-wait */
    }
    TRIM_ClearIntStatusMsk(TRIM, TRIM_FLAG_MEASURE_STOP_Msk);
    counter_result = TRIM_GetRealCnt(TRIM);

    CLK_EnableClkTrim(DISABLE);

    return counter_result;
}

#if (CONFIG_LOW_SPEED_CLOCK_SRC == 0)
void calibrate_rcl_clk(uint32_t expected_freq)
{
    uint32_t AHB_clk_reg;

    const uint8_t coarse_tune_bitwidth = 3;
    const uint8_t fine_tune_bitwidth = 7;

#ifdef PRINT_RCL_FREQ_BEFORE_AND_AFTER_CALIBRATE
    SYS_PRINT("RCL Freq before calib: %d Hz\n", clktrim_measure_32k_clk(DST_CLK_CNT));
#endif

    AHB_clk_reg = CLK->AHB_CLK_CTRL;
    CLK->AHB_CLK_CTRL |= CLK_AHBPeriph_APB2;

    TRIM_EnableInt(TRIM, ENABLE);

    /* Start calibration */
    /* Disable hardware calculate. */
    ANA->RCL_HW_CAL_CTRL &= ~ANAC_RCL_HW_CAL_EN_Msk;
    /* Enable Clktrim */
    CLK_SelectClkTrimSrc(CLKTRIM_CALC_CLK_SEL_32K);
    CLK_EnableClkTrim(ENABLE);

    TRIM_SetCalCnt(TRIM, DST_CLK_CNT);
    TRIM_SetIdealCnt(TRIM, FREQ_32MHZ / expected_freq * DST_CLK_CNT - 1);

    /* a. Coarse tuning proc */
    TRIM_SetBitWidth(TRIM, coarse_tune_bitwidth);
    TRIM_SetCoarseCode(TRIM, 1 << coarse_tune_bitwidth);
    TRIM_StartTuning(TRIM, TRIM_COARSE_TUNING_EN_Msk);
    while (!TRIM_IsIntStatusOccured(TRIM, TRIM_FLAG_CTUNE_STOP_Msk)) {
        /* busy-wait */
    }
    TRIM_ClearIntStatusMsk(TRIM, TRIM_FLAG_CTUNE_STOP_Msk);

    /* b. Fine tuning proc */
    TRIM_SetBitWidth(TRIM, fine_tune_bitwidth);
    TRIM_SetFineCode(TRIM, 1 << fine_tune_bitwidth);
    TRIM_StartTuning(TRIM, TRIM_FINE_TUNING_EN_Msk);
    while (!TRIM_IsIntStatusOccured(TRIM, TRIM_FLAG_FTUNE_STOP_Msk)) {
        /* busy-wait */
    }
    TRIM_ClearIntStatusMsk(TRIM, TRIM_FLAG_FTUNE_STOP_Msk);

    /* Disable Clktrim */
    CLK_EnableClkTrim(DISABLE);

    CLK->AHB_CLK_CTRL = AHB_clk_reg;

#ifdef PRINT_RCL_FREQ_BEFORE_AND_AFTER_CALIBRATE
    SYS_PRINT("RCL Freq after calib: %d Hz\n", clktrim_measure_32k_clk(DST_CLK_CNT));
#endif
}
#endif /* CONFIG_LOW_SPEED_CLOCK_SRC == 0 */
