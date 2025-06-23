/**************************************************************************
 * @file     system_PanSeries.h
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 2023/11/08 $
 * @brief    Panchip series system clock definition file
 *
 * @note     Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
 *****************************************************************************/

#ifndef __SYSTEM_PANSERIES_H__
#define __SYSTEM_PANSERIES_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Stringify the expression
 */
#define __PAN_STRINGIFY(x)      #x
#define PAN_STRINGIFY(s)        __PAN_STRINGIFY(s)

/*
 * Concatenate the values of the arguments into one
 */
#define __PAN_CONCAT(x, y)      x ## y
#define PAN_CONCAT(x, y)        __PAN_CONCAT(x, y)


/* Used internally by PAN_COND_CODE_1 and PAN_COND_CODE_0. */
#define __PAN_COND_CODE_1(_flag, _if_1_code, _else_code)        \
        __PAN_COND_CODE(_AAAAA##_flag, _if_1_code, _else_code)
#define __PAN_COND_CODE_0(_flag, _if_0_code, _else_code)        \
        __PAN_COND_CODE(_BBBBB##_flag, _if_0_code, _else_code)
#define _AAAAA1         _ZZZZZ,
#define _BBBBB0         _ZZZZZ,
#define __PAN_COND_CODE(one_or_two_args, _if_code, _else_code)          \
        __PAN_GET_ARG2_DEBRACKET(one_or_two_args _if_code, _else_code)
/* Gets second argument and removes brackets around that argument. It
   is expected that the parameter is provided in brackets/parentheses. */
#define __PAN_GET_ARG2_DEBRACKET(ignore_this, val, ...) __PAN_DEBRACKET val
/* Used to remove brackets from around a single argument. */
#define __PAN_DEBRACKET(...) __VA_ARGS__

/**
 * @brief Insert code depending on whether _flag expands to 1 or not.
 *
 * As the result of _flag expansion, results in either _if_1_code
 * or _else_code is expanded.
 *
 * To prevent the preprocessor from treating commas as argument
 * separators, the _if_1_code and _else_code expressions must be
 * inside brackets/parentheses: (). These are stripped away
 * during macro expansion.
 *
 * @param _flag evaluated flag
 * @param _if_1_code result if _flag expands to 1; must be in parentheses
 * @param _else_code result otherwise; must be in parentheses
 */
#define PAN_COND_CODE_1(_flag, _if_1_code, _else_code)      \
        __PAN_COND_CODE_1(_flag, _if_1_code, _else_code)

/**
 * @brief Like PAN_COND_CODE_1() except tests if _flag is 0.
 *
 * This is like PAN_COND_CODE_1(), except that it tests whether _flag
 * expands to the integer literal 0. It expands to _if_0_code if
 * so, and _else_code otherwise; both of these must be enclosed in
 * parentheses.
 *
 * @param _flag evaluated flag
 * @param _if_0_code result if _flag expands to 0; must be in parentheses
 * @param _else_code result otherwise; must be in parentheses
 * @see PAN_COND_CODE_1()
 */
#define PAN_COND_CODE_0(_flag, _if_0_code, _else_code)      \
        __PAN_COND_CODE_0(_flag, _if_0_code, _else_code)


#ifndef BUILD_ASSERT
/*
 * Compile-time assertion that makes the build to fail.
 * Common implementation swallows the message.
 */
#define BUILD_ASSERT(EXPR, MSG...)                      \
    enum __PAN_CONCAT(__build_assert_enum, __LINE__) {      \
        PAN_CONCAT(__build_assert, __LINE__) = 1 / !!(EXPR) \
    }
#endif /* BUILD_ASSERT */


/*
 * OTP (One-Time-Program) structure definition
 */
typedef struct _adc_vbg_kb
{
    uint16_t __attribute__((packed)) adc_vbg_k;
    int16_t __attribute__((packed)) adc_vbg_b;
} __attribute__((packed)) ADC_VBG_KB_T;

/* This structure holds soc vendor data in eFuse or flash info */
typedef union _otp_struct
{
    uint8_t d8[0x100];
    struct
    {
        uint8_t rsvd0[30];                          // offset: 0x00 ~ 0x1D
        uint8_t pmu_vbg_trim : 3;                   // offset: 0x1E (bit 0 ~ 2)
        uint8_t rsvd1 : 1;                          // offset: 0x1E (bit 3)
        uint8_t pmu_vbg_volt_l : 4;                 // offset: 0x1E (bit 4 ~ 7)
        uint8_t pmu_vbg_volt_h;                     // offset: 0x1F
        uint8_t hp_ldo_trim : 4;                    // offset: 0x20 (bit 0 ~ 3)
        uint8_t hp_ldo_volt_l : 4;                  // offset: 0x20 (bit 4 ~ 7)
        uint8_t hp_ldo_volt_h;                      // offset: 0x21
        uint8_t lph_ldo_trim : 4;                   // offset: 0x22 (bit 0 ~ 3)
        uint8_t lph_ldo_volt_l : 4;                 // offset: 0x22 (bit 4 ~ 7)
        uint8_t lph_ldo_volt_h;                     // offset: 0x23
        uint8_t lpl_ldo_trim : 4;                   // offset: 0x24 (bit 0 ~ 3)
        uint8_t lpl_ldo_volt_l : 4;                 // offset: 0x24 (bit 4 ~ 7)
        uint8_t lpl_ldo_volt_h;                     // offset: 0x25
        uint8_t flash_ldo_trim : 3;                 // offset: 0x26 (bit 0 ~ 2)
        uint8_t rsvd2 : 1;                          // offset: 0x26 (bit 3)
        uint8_t flash_ldo_volt_l : 4;               // offset: 0x26 (bit 4 ~ 7)
        uint8_t flash_ldo_volt_h;                   // offset: 0x27
        uint8_t ana_ldo_trim : 4;                   // offset: 0x28 (bit 0 ~ 3)
        uint8_t rffe_ldo_trim : 4;                  // offset: 0x28 (bit 4 ~ 7)
        uint8_t fsyn_ldo_trim : 4;                  // offset: 0x29 (bit 0 ~ 3)
        uint8_t adc_ldo_trim : 4;                   // offset: 0x29 (bit 4 ~ 7)
        uint8_t vco_ldo_trim : 4;                   // offset: 0x2A (bit 0 ~ 3)
        uint8_t ipoly_trim : 3;                     // offset: 0x2A (bit 4 ~ 6)
        uint8_t rsvd3 : 1;                          // offset: 0x2A (bit 7)
        uint8_t bod_vref_trim : 3;                  // offset: 0x2B (bit 0 ~ 2)
        uint8_t rsvd4 : 1;                          // offset: 0x2B (bit 3)
        uint8_t bod_vref_volt_l : 4;                // offset: 0x2B (bit 4 ~ 7)
        uint8_t bod_vref_volt_h;                    // offset: 0x2C
        uint8_t adc_vbg_1v17_trim : 6;              // offset: 0x2D (bit 0 ~ 5)
        uint8_t rsvd5 : 2;                          // offset: 0x2D (bit 6 ~ 7)
        uint16_t adc_vbg_1v17_volt;                 // offset: 0x2E, 0x2F
        uint8_t adc_vbg_1v20_trim : 6;              // offset: 0x30 (bit 0 ~ 5)
        uint8_t rsvd6 : 2;                          // offset: 0x30 (bit 6 ~ 7)
        uint16_t adc_vbg_1v20_volt;                 // offset: 0x31, 0x32
        uint8_t rch_trim;                           // offset: 0x33
        uint32_t rch_freq;                          // offset: 0x34 ~ 0x37
        uint8_t rcl_coarse_trim : 4;                // offset: 0x38 (bit 0 ~ 3)
        uint8_t rsvd7 : 4;                          // offset: 0x38 (bit 4 ~ 7)
        uint8_t rcl_fine_trim;                      // offset: 0x39
        uint16_t rcl_freq;                          // offset: 0x3A, 0x3B
        float xth_xocap_a;                          // offset: 0x3C ~ 0x3F
        float xth_xocap_m;                          // offset: 0x40 ~ 0x43
        float xth_xocap_n;                          // offset: 0x44 ~ 0x47
        float xtl_xocap_a;                          // offset: 0x48 ~ 0x4B
        float xtl_xocap_m;                          // offset: 0x4C ~ 0x4F
        float xtl_xocap_n;                          // offset: 0x50 ~ 0x53
        uint8_t buck_imax_cal_trim : 5;             // offset: 0x54 (bit 0 ~ 4)
        uint8_t rsvd8 : 3;                          // offset: 0x54 (bit 5 ~ 7)
        uint8_t buck_zero_cal_trim : 5;             // offset: 0x55 (bit 0 ~ 4)
        uint8_t rsvd9 : 3;                          // offset: 0x55 (bit 5 ~ 7)
        uint8_t buck_out_trim : 5;                  // offset: 0x56 (bit 0 ~ 4)
        uint8_t rsvd10 : 3;                         // offset: 0x56 (bit 5 ~ 7)
        uint16_t buck_vout_volt;                    // offset: 0x57, 0x58
        uint16_t adc_vdd_k;                         // offset: 0x59, 0x5A
        int8_t adc_vdd_b;                           // offset: 0x5B
        uint16_t adc_vbg_k;                         // offset: 0x5C, 0x5D
        int16_t adc_vbg_b;                          // offset: 0x5E, 0x5F
        uint16_t adc_temp_volt;                     // offset: 0x60, 0x61
        int16_t current_temp_value;                 // offset: 0x62, 0x63
        uint8_t battery_trim;                       // offset: 0x64
        uint8_t pa_power_calib_trim_6dbm : 4;       // offset: 0x65 (bit 0 ~ 3)
        uint8_t pa_power_calib_trim_0dbm : 4;       // offset: 0x65 (bit 4 ~ 7)
        uint8_t mac_addr[6];                        // offset: 0x66 ~ 0x6B
        uint8_t uid[9];                             // offset: 0x6C ~ 0x74
        uint8_t chip_info;                          // offset: 0x75
        uint8_t cp_version;                         // offset: 0x76
        uint8_t ft_version;                         // offset: 0x77
        uint8_t cp_pass_flag;                       // offset: 0x78
        uint8_t ft_pass_flag;                       // offset: 0x79
        uint8_t cp_checksum;                        // offset: 0x7A
        uint8_t ft_checksum;                        // offset: 0x7B
        uint8_t user_rw_data[4];                    // offset: 0x7C ~ 0x7F
    } __attribute__((packed)) m;
    struct
    {
        uint8_t rsvd0[30];                          // offset: 0x00 ~ 0x1D
        uint8_t pmu_vbg_trim : 3;                   // offset: 0x1E (bit 0 ~ 2)
        uint8_t rsvd1 : 1;                          // offset: 0x1E (bit 3)
        uint8_t pmu_vbg_volt_l : 4;                 // offset: 0x1E (bit 4 ~ 7)
        uint8_t pmu_vbg_volt_h;                     // offset: 0x1F
        uint8_t hp_ldo_trim : 4;                    // offset: 0x20 (bit 0 ~ 3)
        uint8_t hp_ldo_volt_l : 4;                  // offset: 0x20 (bit 4 ~ 7)
        uint8_t hp_ldo_volt_h;                      // offset: 0x21
        uint8_t lph_ldo_trim : 4;                   // offset: 0x22 (bit 0 ~ 3)
        uint8_t lph_ldo_volt_l : 4;                 // offset: 0x22 (bit 4 ~ 7)
        uint8_t lph_ldo_volt_h;                     // offset: 0x23
        uint8_t lpl_ldo_trim : 4;                   // offset: 0x24 (bit 0 ~ 3)
        uint8_t lpl_ldo_volt_l : 4;                 // offset: 0x24 (bit 4 ~ 7)
        uint8_t lpl_ldo_volt_h;                     // offset: 0x25
        uint8_t flash_ldo_trim : 3;                 // offset: 0x26 (bit 0 ~ 2)
        uint8_t rsvd2 : 1;                          // offset: 0x26 (bit 3)
        uint8_t flash_ldo_volt_l : 4;               // offset: 0x26 (bit 4 ~ 7)
        uint8_t flash_ldo_volt_h;                   // offset: 0x27
        uint8_t ana_ldo_trim : 4;                   // offset: 0x28 (bit 0 ~ 3)
        uint8_t rffe_ldo_trim : 4;                  // offset: 0x28 (bit 4 ~ 7)
        uint8_t fsyn_ldo_trim : 4;                  // offset: 0x29 (bit 0 ~ 3)
        uint8_t adc_ldo_trim : 4;                   // offset: 0x29 (bit 4 ~ 7)
        uint8_t vco_ldo_trim : 4;                   // offset: 0x2A (bit 0 ~ 3)
        uint8_t ipoly_trim : 3;                     // offset: 0x2A (bit 4 ~ 6)
        uint8_t rsvd3 : 1;                          // offset: 0x2A (bit 7)
        uint8_t bod_vref_trim : 3;                  // offset: 0x2B (bit 0 ~ 2)
        uint8_t rsvd4 : 1;                          // offset: 0x2B (bit 3)
        uint8_t bod_vref_volt_l : 4;                // offset: 0x2B (bit 4 ~ 7)
        uint8_t bod_vref_volt_h;                    // offset: 0x2C
        uint8_t rsvd5;                              // offset: 0x2D
        uint8_t xtl_core: 3;                        // offset: 0x2E (bit 0 ~ 2) <ft v6>
        uint8_t xth_icore: 1;                       // offset: 0x2E (bit 3)     <ft v6>
        uint8_t rch_bias: 2;                        // offset: 0x2E (bit 4 ~ 5) <ft v6>
        uint8_t dpll_vco_freq_trim: 2;              // offset: 0x2E (bit 6 ~ 7) <ft v6>
        uint8_t dpll_kvco_ctrl: 3;                  // offset: 0x2F (bit 0 ~ 2) <ft v6>
        uint8_t dpll_icp_ctrl: 2;                   // offset: 0x2F (bit 3 ~ 4) <ft v6>
        uint8_t dpll_icp_bias: 2;                   // offset: 0x2F (bit 5 ~ 6) <ft v6>
        uint8_t rsvd6: 1;                           // offset: 0x2F (bit 7)
        uint8_t adc_vbg_1v20_trim : 6;              // offset: 0x30 (bit 0 ~ 5)
        uint8_t ptat_temp_trim : 2;                 // offset: 0x30 (bit 6 ~ 7) <ft v6>
        uint16_t adc_vbg_1v20_volt;                 // offset: 0x31, 0x32
        uint8_t rch_trim;                           // offset: 0x33
        uint32_t rch_freq;                          // offset: 0x34 ~ 0x37
        uint8_t rcl_coarse_trim : 4;                // offset: 0x38 (bit 0 ~ 3)
        uint8_t lph_ldo_vref_trim : 3;              // offset: 0x38 (bit 4 ~ 6) <v2>
        uint8_t rsvd7 : 1;                          // offset: 0x38 (bit 7)
        uint8_t rcl_fine_trim;                      // offset: 0x39
        uint16_t rcl_freq;                          // offset: 0x3A, 0x3B
        uint16_t analog_temp_volt;                  // offset: 0x3C, 0x3D       <v2>
        uint8_t rsvd_0x3e[10];                      // offset: 0x3E ~ 0x47
        uint16_t analog_0p6v_volt;                  // offset: 0x48, 0x49       <v2>
        uint8_t rsvd8[10];                          // offset: 0x4A ~ 0x53
        uint8_t buck_imax_cal_trim : 5;             // offset: 0x54 (bit 0 ~ 4)
        uint8_t rsvd9 : 3;                          // offset: 0x54 (bit 5 ~ 7)
        uint8_t buck_zero_cal_trim : 5;             // offset: 0x55 (bit 0 ~ 4)
        uint8_t rsvd10 : 3;                         // offset: 0x55 (bit 5 ~ 7)
        uint8_t buck_out_trim : 5;                  // offset: 0x56 (bit 0 ~ 4)
        uint8_t rsvd11 : 3;                         // offset: 0x56 (bit 5 ~ 7)
        uint16_t buck_vout_volt;                    // offset: 0x57, 0x58
        uint16_t adc_vdd_k;                         // offset: 0x59, 0x5A
        int8_t adc_vdd_b;                           // offset: 0x5B
        uint16_t adc_vbg_k;                         // offset: 0x5C, 0x5D
        int16_t adc_vbg_b;                          // offset: 0x5E, 0x5F
        uint16_t adc_temp_volt;                     // offset: 0x60, 0x61
        int16_t current_temp_value;                 // offset: 0x62, 0x63
        uint8_t battery_trim;                       // offset: 0x64
        uint8_t rsvd12[7];                          // offset: 0x65 ~ 0x6B
        uint8_t uid[9];                             // offset: 0x6C ~ 0x74
        uint8_t chip_info;                          // offset: 0x75
        uint8_t cp_version;                         // offset: 0x76
        uint8_t ft_version;                         // offset: 0x77
        uint8_t cp_pass_flag;                       // offset: 0x78
        uint8_t ft_pass_flag;                       // offset: 0x79
        uint8_t cp_checksum;                        // offset: 0x7A
        uint8_t ft_checksum;                        // offset: 0x7B
        uint8_t user_rw_data[4];                    // offset: 0x7C ~ 0x7F
        uint8_t mac_addr[6];                        // offset: 0x80 ~ 0x85  <v2>
        float xth_xocap_a;                          // offset: 0x86 ~ 0x89  <v2>
        float xth_xocap_m;                          // offset: 0x8A ~ 0x8D  <v2>
        float xth_xocap_n;                          // offset: 0x8E ~ 0x91  <v2>
        uint8_t xth_xocap_trim;                     // offset: 0x92         <v2>
        float xtl_xocap_a;                          // offset: 0x93 ~ 0x96  <v2>
        float xtl_xocap_m;                          // offset: 0x97 ~ 0x9A  <v2>
        float xtl_xocap_n;                          // offset: 0x9B ~ 0x9E  <v2>
        uint8_t xtl_xocap_trim;                     // offset: 0x9F         <v2>
        ADC_VBG_KB_T adc_vbg_kb[7];                 // offset: 0xA0 ~ 0xBB  <ft v3>
        uint16_t adc_vbat_k;                        // offset: 0xBC, 0xBD   <ft v3>
        int16_t adc_vbat_b;                         // offset: 0xBE, 0xBF   <ft v3>
        int16_t adc_vbat_dtemp_k;                   // offset: 0xC0, 0xC1   <ft v5>
        int16_t adc_vbat_dtemp_b;                   // offset: 0xC2, 0xC3   <ft v5>
        float adc_temp_k;                           // offset: 0xC4 ~ 0xC7  <ft v6>
        uint32_t adc_ctrl2;                         // offset: 0xC8 ~ 0xCB  <ft v6>
        uint32_t adc_extsmpt;                       // offset: 0xCC ~ 0xCF  <ft v6>
        uint8_t xth_fix_code1;                      // offset: 0xD0         <ft v6>
        uint8_t xth_fix_code2;                      // offset: 0xD1         <ft v6>
        uint8_t xtl_fix_code1;                      // offset: 0xD2         <ft v6>
        uint8_t xtl_fix_code2;                      // offset: 0xD3         <ft v6>
        uint8_t rsvd13[43];                         // offset: 0xD4 ~ 0xFE
        uint8_t ft_checksum2;                       // offset: 0xFF         <v2>
    } __attribute__((packed)) m_v2;
} OTP_STRUCT_T;

extern bool isFtDataValid;
extern uint32_t SystemCoreClock;

/**
 * Update SystemCoreClock variable
 *
 * @param  None
 * @return None
 *
 * @brief  Updates the SystemCoreClock with current core Clock
 *         retrieved from CPU registers.
 */
extern void SystemCoreClockUpdate(void);
extern void SystemInit(void);
extern bool SystemHwParamLoader(OTP_STRUCT_T *otp);
#ifdef __cplusplus
}
#endif

#endif  //__SYSTEM_PANSERIES_H__


/*** (C) COPYRIGHT 2016 Panchip Technology Corp. ***/

