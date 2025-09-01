#include "oled.h"
#include "pan_rf.h"
#include "display.h"
#include "key_handle.h"

bool g_UserTxMode;
static char g_UI_TxModeCfgList[4] = {'A', 'B', 'C', 'D'};
static char g_UI_TestModeCfgList[3] = {' ', '1', '2'};
static char *g_UI_BwCfgList[9] = {"0", "0", "0", "0", "0", "62.5", "125 ", "250 ", "500 "};
static char *g_UI_CrCfgList[5] = {"0", "4/5", "4/6", "4/7", "4/8"};

extern uint32_t g_KeyRfTxLen;

void UI_ShowError(char *str)
{
    OLED_Clear();
    OLED_ShowString(0, 22, str, 16);
    OLED_Refresh();
}

void UI_SetSF(uint32_t val)
{
    OLED_ShowNum(85, 32, val, 4, 16);
}

void UI_SetBW(uint32_t val)
{
    OLED_ShowString(30, 32, g_UI_BwCfgList[val - 1], 16);
}

void UI_SetPayloadLen(uint32_t val)
{
    OLED_ShowNum(20, 48, val, 4, 16);
}

void UI_SetTxMode(uint32_t val)
{
    OLED_ShowChar(103, 0, g_UI_TxModeCfgList[val], 16, 1);
}

void UI_SetTestMode(uint32_t val)
{
    OLED_ShowChar(110, 0, g_UI_TestModeCfgList[val], 16, 1);
}

void UI_SetCR(uint32_t val)
{
    OLED_ShowString(30, 16, g_UI_CrCfgList[val], 16);
}

void UI_SetFreq(uint32_t val)
{
    OLED_ShowNum(85, 16, val, 4, 16);
}

void UI_SetTRxCnt(uint32_t val)
{
    OLED_ShowNum(85, 48, val, 4, 16);
    OLED_Refresh();
}

void UI_SetTxPower(uint32_t val)
{
    OLED_ShowNum(22, 0, val, 4, 16);
}

void UI_SetCWMode(uint32_t val)
{
    OLED_ShowString(32, 48, "POWER:", 16);
    OLED_ShowNum(85, 48, val, 4, 16);
}

void UI_RefreshScreen(void)
{
    OLED_Refresh();
}

void UI_Init(void)
{
    uint32_t para;

    OLED_ShowString(0, 0, "PW:", 16);
    OLED_ShowString(0, 16, "CR:", 16);
    OLED_ShowString(0, 32, "BW:", 16);
    OLED_ShowString(0, 48, "PL:", 16);
    OLED_ShowString(64, 0, "MODE:", 16);
    OLED_ShowString(64, 16, "FQ:", 16);
    OLED_ShowString(64, 32, "SF:", 16);

    g_UserTxMode = (bool)TX_RX_SELECT(); /* 通过拨码开关(EVB-J5)选择TX或RX模式 */
    
    if (g_UserTxMode)
    {
        OLED_ShowString(64, 48, "TX:", 16);
    }
    else
    {
        OLED_ShowString(64, 48, "RX:", 16);
    }

    UI_SetTRxCnt(0);
    UI_SetTxMode(0);

    para = RF_GetFreq();
    UI_SetFreq((para / 100000));

    para = RF_GetCR();
    UI_SetCR(para);

    para = RF_GetSF();
    UI_SetSF(para);

    para = RF_GetBandWidth();
    UI_SetBW(para);

    UI_SetPayloadLen(g_KeyRfTxLen);

    para = RF_GetTxPower();
    UI_SetTxPower(para);

    OLED_Refresh();
}
