#include "PanSeries.h"
#include "pan_power.h"

power_param_t m_power_param;

static uint8_t config_flag = 0xFF;

bool PW_ParamExists(void)
{
    return m_power_param.hp_ldo_trim;
}
void PW_ParamsSet(OTP_STRUCT_T *p_otp)
{
    m_power_param.buck_out_trim = p_otp->m.buck_out_trim;
    m_power_param.hp_ldo_trim = p_otp->m.hp_ldo_trim;
    m_power_param.lph_ldo_vref_trim = p_otp->m_v2.lph_ldo_vref_trim;
    m_power_param.lpl_ldo_trim = p_otp->m.lpl_ldo_trim;
}

void PW_AutoOptimizeParams(int16_t temp)
{
    uint32_t tmp;
    int32_t val;

    if (temp <= 0) {    // Low temperature (<= 0 Celsius)
        if (config_flag == 1)
            return;
        config_flag = 1;

        //buck out(DCDC): default:8; FT+2
        val = (m_power_param.buck_out_trim >> 1) + 2;
        if (val > 0xFu) {
            val = 0xFu;
        }
        tmp = ANA->LP_BUCK_3V;
        tmp &= ~(0xFu << 2);
        tmp |= (val << 2);
        ANA->LP_BUCK_3V = tmp;

        //LPLDOH vref: Fixed to Maximum (0x7)
        tmp = ANA->LP_LP_LDO_3V;
        tmp |= (0x7u << 21);
        ANA->LP_LP_LDO_3V = tmp;

        //LPLDOL trim: Fixed to Maximum (0xF)
        tmp = ANA->LP_LP_LDO_3V;
        tmp |= (0xFu << 1);
        ANA->LP_LP_LDO_3V = tmp;

        //HPLDO(DVDD): default:8; FT
        tmp = ANA->LP_HP_LDO;
        tmp &= ~(0xFu <<3);
        tmp |= ((m_power_param.hp_ldo_trim) << 3);
        ANA->LP_HP_LDO = tmp;
    } else if (temp >= 50) {    // High temperature (>= 50 Celsius)
        if (config_flag == 2)
            return;
        config_flag = 2;

        //buck out(DCDC): default:8; FT+2
        val = (m_power_param.buck_out_trim >> 1) + 2;
        if (val > 0xFu) {
            val = 0xFu;
        }
        tmp = ANA->LP_BUCK_3V;
        tmp &= ~(0xFu << 2);
        tmp |= (val << 2);
        ANA->LP_BUCK_3V = tmp;

        //LPLDOH vref: FT
        tmp = ANA->LP_LP_LDO_3V;
        tmp &= ~(0x7u << 21);
        tmp |= (m_power_param.lph_ldo_vref_trim << 21);
        ANA->LP_LP_LDO_3V = tmp;

        //LPLDOL trim: FT+2
        val = m_power_param.lpl_ldo_trim + 2;
        if (val > 0xFu) {
            val = 0xFu;
        }
        tmp = ANA->LP_LP_LDO_3V;
        tmp &= ~(0xFu << 1);
        tmp |= (val << 1);
        ANA->LP_LP_LDO_3V = tmp;

        //HPLDO(DVDD): default:8; FT
        tmp = ANA->LP_HP_LDO;
        tmp &= ~(0xFul <<3);
        tmp |= ((m_power_param.hp_ldo_trim) << 3);
        ANA->LP_HP_LDO = tmp;
    } else {    // Normal temperature (0 Celsius < temp < 50 Celsius)
        if (config_flag == 0)
            return;
        config_flag = 0;

        //buck out(DCDC): default:8; FT-2
        val = (m_power_param.buck_out_trim >> 1) - 2;
        if (val < 0x0) {
            val = 0x0;
        }
        uint32_t tmp = ANA->LP_BUCK_3V;
        tmp &= ~(0xFul<<2);
        tmp |= (val << 2);
        ANA->LP_BUCK_3V = tmp;

        //LPLDOH vref; FT
        tmp = ANA->LP_LP_LDO_3V;
        tmp &= ~(0x7u << 21);
        tmp |= ((m_power_param.lph_ldo_vref_trim) << 21);
        ANA->LP_LP_LDO_3V = tmp;

        //LPLDOL trim; FT
        tmp = ANA->LP_LP_LDO_3V;
        tmp &= ~(0xFu << 1);
        tmp |= ((m_power_param.lpl_ldo_trim) << 1);
        ANA->LP_LP_LDO_3V = tmp;

#if CONFIG_DVDD_VOL_OPTIMIZE_EN
        //HPLDO(DVDD) default - 1or2; default:8
        val = m_power_param.hp_ldo_trim - 2;
        if (val < 0x0) {
            val = 0x0;
        }
        tmp = ANA->LP_HP_LDO;
        tmp &= ~(0xFul <<3);
        tmp |= (val << 3); //~1.12V
        ANA->LP_HP_LDO = tmp;
#else
        //HPLDO(DVDD) default:8;  FT-1
        val = m_power_param.hp_ldo_trim - 1;
        if (val < 0x0) {
            val = 0x0;
        }
        tmp = ANA->LP_HP_LDO;
        tmp &= ~(0xFul <<3);
        tmp |= (val << 3); //~1.16V
        ANA->LP_HP_LDO = tmp;
#endif
    }
}
