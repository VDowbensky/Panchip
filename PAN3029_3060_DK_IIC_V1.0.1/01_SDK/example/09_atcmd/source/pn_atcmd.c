/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file pn_atcmd.c
 * @brief
 *
 * @history - V0.8, 2024-4
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "at.h"
#include "at_log.h"
#include "pan_rf.h"

#define RF_FIFO_LEN           255  // RF RX/TX FIFO length
#define MAC_PRIVATE_SYNCWORD  0x12 // Private MAC sync word
#define MAC_PUBLIC_SYNCWORD   0x34 // Public MAC sync word

typedef struct
{
    u32 Freq;         // Frequency, unit is Hz
    u8 CodeRate;      // Coding rate
    u8 Bandwidth;     // Bandwidth
    u8 SpreadFactor;  // Spread factor
    u8 TxPwr;         // TxPower level index, not the actual RF output power
    u8 CrcEnable;     // CRC enable
    u8 LdrEnable;     // LDR enable
    u8 ModemMode;     // Modem mode
    u8 PreamLen;      // Preamble length
    u8 DCDC;          // DCDC mode
    u8 ChipMode;      // Chip mode
    u8 InvertIQ;      // Invert I/Q
    u8 SyncWord;      // Sync word
}AtRfConfig_t;

uint8_t g_AtRfRxLen;
uint8_t g_AtRfRxBuf[RF_FIFO_LEN];

uint8_t g_AtRfTxLen = 10;
uint8_t g_AtRfTxBuf[RF_FIFO_LEN] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

/**
 * @brief PAN3029/3060 atcmd default configuration
 */
AtRfConfig_t g_RfAtConfig = 
{
    .Freq = 490000000,
    .CodeRate = RF_CR_4_5,
    .Bandwidth = RF_BW_125K,
    .SpreadFactor = RF_SF7,
    .TxPwr = 22,    // TxPower level index, not the actual RF output power
    .CrcEnable = RF_CRC_ON,
    .LdrEnable = RF_LDR_OFF,
    .ModemMode = MODEM_MODE_NORMAL,
    .PreamLen = 8,  // Preamble length
    .DCDC = USE_LDO,
    .ChipMode = CHIPMODE_MODE0,
    .InvertIQ = RF_IQ_NORMAL,
    .SyncWord = MAC_PRIVATE_SYNCWORD,
};

/**
 * @brief Read a character from UART RX FIFO
 */
extern int UART_ReadFifo(uint8_t *Char);

/**
 * @brief Flush UART RX FIFO
 */
extern void UART_FlushRxFifo(void);

void at_rf_config(AtRfConfig_t *p)
{
    RF_SetRfState(RF_STATE_STB3);

    RF_SetFreq(p->Freq);
    RF_SetCR(p->CodeRate);
    RF_SetBW(p->Bandwidth);
    RF_SetSF(p->SpreadFactor);
    RF_SetTxPower(p->TxPwr);
    RF_SetCRC(p->CrcEnable);
    RF_SetLDR(p->LdrEnable);
    RF_SetPreamLen(p->PreamLen);
    RF_SetModemMode(p->ModemMode);
    RF_SetRegulatorMode((RfRegulatorMode_t)p->DCDC);
    RF_SetChipMode((RfChipMode_t)p->ChipMode);
    RF_SetInvertIQ((RfIQModes_t)p->InvertIQ);
}

bool ATCMD_CheckBreak(void)
{
    char Char;

    do{
        int Ret = UART_ReadFifo((uint8_t *)&Char);
        if (Ret > 0)
        {
            // Check for break character
            if (Char == 'c' || Char == 'C')
            {
                UART_FlushRxFifo();
                return 1;
            }
        }
        else
        {
            break;
        }
    } while (1);

    return 0;
}

static at_result_t ATCMD_EnterDeepsleep(void)
{
    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    RF_EnterDeepsleepState();

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+SHUTDOWN", PN_NULL, PN_NULL, PN_NULL, PN_NULL, ATCMD_EnterDeepsleep);
AT_CMD_EXPORT("AT+DEEPSLEEP", PN_NULL, PN_NULL, PN_NULL, PN_NULL, ATCMD_EnterDeepsleep);

static at_result_t ATCMD_EnterSleepState(void)
{
    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    RF_EnterSleepState();

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+SLEEPMODE", PN_NULL, PN_NULL, PN_NULL, PN_NULL, ATCMD_EnterSleepState);

static at_result_t ATCMD_GetParams(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+CONFIG=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                       p->Freq, p->CodeRate, p->Bandwidth, p->SpreadFactor, p->TxPwr,
                       p->CrcEnable, p->LdrEnable, p->ModemMode, p->PreamLen,
                       p->DCDC);

    return AT_RESULT_OK;
}

static char *g_UI_CrCfgList[5] = {"4/5", "4/6", "4/7", "4/8"};
static char *g_UI_BwCfgList[9] = {"62.5K", "125K", "250K", "500K"};

static at_result_t ATCMD_SetParams(const char *args)
{
    int Freq, CodeRate, Bandwidth, SpreadFactor, txpower, CrcEnable, lowdron, ModemMode, PreamLen, DCDC, argc;
    const char *req_expr = "=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d";

    argc = at_req_parse_args(args, req_expr, &Freq, &CodeRate, &Bandwidth, &SpreadFactor,
                             &txpower, &CrcEnable, &lowdron, &ModemMode,
                             &PreamLen, &DCDC);
    if (argc != 10)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("Frequency:%3.1fMHz", (float)Freq/1000000.0f);
    atDebug("CodeRate:%s", g_UI_CrCfgList[CodeRate - RF_CR_4_5]);
    atDebug("BandWidth:%s", g_UI_BwCfgList[Bandwidth - RF_BW_062K]);
    atDebug("SpreadFactor:SF%u", SpreadFactor);
    atDebug("TxPower:%u", txpower);
    atDebug("CRC:%u", CrcEnable);
    atDebug("LDR:%u", lowdron);
    atDebug("ModemMode:%u", ModemMode);
    atDebug("Preamble:%u", PreamLen);
    atDebug("DCDC:%u", DCDC);

    g_RfAtConfig.Freq = Freq;
    g_RfAtConfig.CodeRate = CodeRate;
    g_RfAtConfig.Bandwidth = Bandwidth;
    g_RfAtConfig.SpreadFactor = SpreadFactor;
    g_RfAtConfig.TxPwr = txpower;
    g_RfAtConfig.CrcEnable = CrcEnable;
    g_RfAtConfig.LdrEnable = lowdron;
    g_RfAtConfig.ModemMode = ModemMode;
    g_RfAtConfig.PreamLen = PreamLen;
    g_RfAtConfig.DCDC = DCDC;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }
    
    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);
    
    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+CONFIG", "=<Frequency(Hz)>,<CodeRate>,<BandWidth>,<SpreadFactor>,<TxPower>,<CRC>,<LDR>,<ModemMode>,<Preamble>,<DCDC>", PN_NULL, ATCMD_GetParams, ATCMD_SetParams, PN_NULL);

static at_result_t ATCMD_GetFreq(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+FREQ=%d", p->Freq);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetFreq(const char *args)
{
    int Freq, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &Freq);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("Freq:%u", Freq);
    g_RfAtConfig.Freq = Freq;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+FREQ", "=<Freq[(Hz)]>", PN_NULL, ATCMD_GetFreq, ATCMD_SetFreq, PN_NULL);

static at_result_t ATCMD_GetCodeRate(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+CR=%d", p->CodeRate);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetCodeRate(const char *args)
{
    int CodeRate, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &CodeRate);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("CodeRate:%u", CodeRate);
    g_RfAtConfig.CodeRate = CodeRate;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+CR", "=<CR[1-4]>", PN_NULL, ATCMD_GetCodeRate, ATCMD_SetCodeRate, PN_NULL);

static at_result_t ATCMD_GetBandWidth(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+BW=%d", p->Bandwidth);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetBandWidth(const char *args)
{
    int Bandwidth, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &Bandwidth);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("BandWidth:%u", Bandwidth);
    g_RfAtConfig.Bandwidth = Bandwidth;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+BW", "=<BW[6(62.5K)-9(500K)]>", PN_NULL, ATCMD_GetBandWidth, ATCMD_SetBandWidth, PN_NULL);

static at_result_t ATCMD_GetSpreadFactor(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+SF=%d", p->SpreadFactor);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetSpreadFactor(const char *args)
{
    int SpreadFactor, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &SpreadFactor);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("SpreadFactor:%u", SpreadFactor);
    g_RfAtConfig.SpreadFactor = SpreadFactor;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+SF", "=<SF[5-12]>", PN_NULL, ATCMD_GetSpreadFactor, ATCMD_SetSpreadFactor, PN_NULL);

static at_result_t ATCMD_GetTxPwr(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+PWR=%d", p->TxPwr);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetTxPwr(const char *args)
{
    int TxPwr, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &TxPwr);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("TxPwr:%u", TxPwr);
    g_RfAtConfig.TxPwr = TxPwr;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+PWR", "=<Txpower[1-22]>", PN_NULL, ATCMD_GetTxPwr, ATCMD_SetTxPwr, PN_NULL);
AT_CMD_EXPORT("AT+TXPWR", "=<Txpower[1-22]>", PN_NULL, ATCMD_GetTxPwr, ATCMD_SetTxPwr, PN_NULL);

static at_result_t ATCMD_GetCrc(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+CRC=%d", p->CrcEnable);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetCrc(const char *args)
{
    int CrcEnable, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &CrcEnable);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("CrcEnable:%u", CrcEnable);
    g_RfAtConfig.CrcEnable = CrcEnable > 0 ? 1 : 0;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+CRC", "=<CRC[0/1]>", PN_NULL, ATCMD_GetCrc, ATCMD_SetCrc, PN_NULL);

static at_result_t ATCMD_GetLdr(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+LDR=%d", p->LdrEnable);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetLdr(const char *args)
{
    int LdrEnable, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &LdrEnable);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("LDR:%u", LdrEnable);
    g_RfAtConfig.LdrEnable = LdrEnable > 0 ? 1 : 0;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+LDR", "=<low datarate[0/1]>", PN_NULL, ATCMD_GetLdr, ATCMD_SetLdr, PN_NULL);

static at_result_t ATCMD_GetModemMode(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+MODEMMODE=%d", p->ModemMode);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetModemMode(const char *args)
{
    int ModemMode, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &ModemMode);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    if (ModemMode != MODEM_MODE_NORMAL && ModemMode != MODEM_MODE_MULTI_SECTOR)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("ModemMode:%u", ModemMode);
    g_RfAtConfig.ModemMode = ModemMode;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+MODEMMODE", "=<MODEMMODE[0/1]>", PN_NULL, ATCMD_GetModemMode, ATCMD_SetModemMode, PN_NULL);

static at_result_t ATCMD_GetPreamLen(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+PREAMLEN=%d", p->PreamLen);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetPreamLen(const char *args)
{
    int PreamLen, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &PreamLen);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("PREAMLEN:%u", PreamLen);

    g_RfAtConfig.PreamLen = PreamLen;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+PREAMLEN", "=<Preamblelen[4-65535]>", PN_NULL, ATCMD_GetPreamLen, ATCMD_SetPreamLen, PN_NULL);

static at_result_t ATCMD_GetDCDC(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+DCDC=%d", p->DCDC);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetDCDC(const char *args)
{
    int DCDC, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &DCDC);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("DCDC:%u", DCDC);
    g_RfAtConfig.DCDC = DCDC > 0 ? 1 : 0;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+DCDC", "=<DCDC[0/1]>", PN_NULL, ATCMD_GetDCDC, ATCMD_SetDCDC, PN_NULL);

static at_result_t ATCMD_GetChipMode(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+ChipMode=%d", p->ChipMode);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetChipMode(const char *args)
{
    int ChipMode, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &ChipMode);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("ChipMode:%u", ChipMode);
    g_RfAtConfig.ChipMode = ChipMode;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+CHIPMODE", "=<CHIPMODE[0/1]>", PN_NULL, ATCMD_GetChipMode, ATCMD_SetChipMode, PN_NULL);

static at_result_t ATCMD_GetInvertIQ(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+InvertIQ=%d", p->InvertIQ);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetInvertIQ(const char *args)
{
    int InvertIQ, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &InvertIQ);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("InvertIQ:%u", InvertIQ);
    g_RfAtConfig.InvertIQ = InvertIQ;

    int ret = RF_Init();
    if (ret != RF_OK)
    {
        atDebug("RF init fail");
        return AT_RESULT_FAILE;
    }
    else
    {
        atDebug("RF init ok");
    }

    at_rf_config((AtRfConfig_t *)&g_RfAtConfig);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+INVERTIQ", "=<INVERTIQ[0/1]>", PN_NULL, ATCMD_GetInvertIQ, ATCMD_SetInvertIQ, PN_NULL);

static at_result_t ATCMD_GetSyncWord(void)
{
    AtRfConfig_t *p = &g_RfAtConfig;

    at_server_printfln("+SyncWord=0x%x", p->SyncWord);

    return AT_RESULT_OK;
}

static at_result_t ATCMD_SetSyncWord(const char *args)
{
    int SyncWord, argc;
    const char *req_expr = "=%x";

    argc = at_req_parse_args(args, req_expr, &SyncWord);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("SyncWord:0x%x", SyncWord);
    g_RfAtConfig.SyncWord = SyncWord;

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+SYNCWORD", "=<SYNCWORD[12/34]>", PN_NULL, ATCMD_GetSyncWord, ATCMD_SetSyncWord, PN_NULL);

static at_result_t ATCMD_SetCarryWave(const char *args)
{
    int on, argc;
    const char *req_expr = "=%d";

    argc = at_req_parse_args(args, req_expr, &on);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("Set carry wave:%u", on);

    if (on)
    {
        int ret = RF_Init();
        if (ret != RF_OK)
        {
            atDebug("RF init fail");
            return AT_RESULT_FAILE;
        }
        else
        {
            atDebug("RF init ok.");
        }
        
        /* 发送载波前须设置好频率和功率 */
        RF_SetFreq(g_RfAtConfig.Freq);
        RF_SetTxPower(g_RfAtConfig.TxPwr);
        RF_StartTxContinuousWave();
    }
    else
    {
        RF_StopTxContinuousWave();
    }

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+CW", "=<OnOff[0/1]>", PN_NULL, PN_NULL, ATCMD_SetCarryWave, PN_NULL);
AT_CMD_EXPORT("AT+CARRYWAVE", "=<OnOff[0/1]>", PN_NULL, PN_NULL, ATCMD_SetCarryWave, PN_NULL);

static at_result_t ATCMD_SetRxMode(const char *args)
{
    int RxCount = 0;
    int RxTimeoutMs, argc;
    uint32_t LastSystemTickMs;
    const char *req_expr = "=%d";

    atDebug("RF enter rx mode");
    atDebug("Receive \'C\' or \'c\' from uart will break rx task.");

    argc = at_req_parse_args(args, req_expr, &RxTimeoutMs);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    RF_SetSyncWord(g_RfAtConfig.SyncWord);
    
    RF_EnterContinousRxState();

    LastSystemTickMs = SysTick_GetTick();

    while (1)
    {
        if (CHECK_RF_IRQ()) /* 检测到RF中断，高电平表示有中断 */
        {
            uint8_t IRQFlag;
            IRQFlag = RF_GetIRQFlag();    /* 获取中断标志位 */
            if (IRQFlag & RF_IRQ_RX_DONE) /* 接收完成中断 */
            {
                g_RfRxPkt.Snr = RF_GetPktSnr();   /* 获取接收数据包的SNR值 */
                g_RfRxPkt.Rssi = RF_GetPktRssi(); /* 获取接收数据包的RSSI值 */
                
                /* 获取接收数据和长度 */
                g_RfRxPkt.RxLen = RF_GetRecvPayload((uint8_t *)g_RfRxPkt.RxBuf);

                printf("RxLen=%d, Rx Count=%d.\r\n", g_RfRxPkt.RxLen, ++RxCount);
                printf("RxHexString:");
                print_hex((uint8_t *)g_RfRxPkt.RxBuf, g_RfRxPkt.RxLen); /* 以十六进制方式打印接收数据 */
                printf("SNR:%ddB, RSSI:%ddBm \r\n", (int)g_RfRxPkt.Snr, (int)g_RfRxPkt.Rssi);
                RF_ClrIRQFlag(RF_IRQ_RX_DONE); /* 清除接收完成中断标志位 */
                IRQFlag &= ~RF_IRQ_RX_DONE;
                LastSystemTickMs = SysTick_GetTick();
            }
            if (IRQFlag & RF_IRQ_CRC_ERR) /* CRC错误中断 */
            {
                RF_ClrIRQFlag(RF_IRQ_CRC_ERR); /* 清除CRC错误中断标志位 */
                IRQFlag &= ~RF_IRQ_CRC_ERR;
                printf(">>RF_IRQ_CRC_ERR\r\n");
                LastSystemTickMs = SysTick_GetTick();
            }

            if (IRQFlag)
            {
                RF_ClrIRQFlag(IRQFlag); /* 清除未处理的中断标志位 */
            }
        }

        /* Rx timeout, exit rx state */
        if (RxTimeoutMs > 0 && (SysTick_GetTick() - LastSystemTickMs) >= RxTimeoutMs)
        {
            atDebug("RF rx timeout: %dms", RxTimeoutMs);
            goto exit;
        }

        if (ATCMD_CheckBreak())
        {
            atDebug("User break");
            goto exit;
        }
    }

exit:
    atDebug("RF exit rx mode");
    RF_ShutdownAnt();
    RF_SetRfState(RF_STATE_STB3);
    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+RX", "=<Rx timeout[1000(ms)-1000000(ms)]>", PN_NULL, PN_NULL, ATCMD_SetRxMode, PN_NULL);
AT_CMD_EXPORT("AT+RXMODE", "=<Rx timeout[1000(ms)-1000000(ms)]>", PN_NULL, PN_NULL, ATCMD_SetRxMode, PN_NULL);

static at_result_t ATCMD_SetTxMode(const char *args)
{
    int ret = RF_OK;
    uint32_t LastSystemTickMs;
    uint32_t TxCount, TxIntervalMs, argc;
    uint32_t CurrentCount = 0;
    const char *req_expr = "=%d,%d";
    
    atDebug("RF enter tx mode");

    argc = at_req_parse_args(args, req_expr, &TxCount, &TxIntervalMs);
    if (argc != 2)
    {
        return AT_RESULT_PARSE_FAILE;
    }
    
    RF_SetSyncWord(g_RfAtConfig.SyncWord);

    uint32_t TxOtaTimeMs;
    TxOtaTimeMs = RF_GetTxTimeMs(g_AtRfTxLen) + 100; /* 多加100ms，保证判别余量 */
    atDebug("Start to tx %d bytes.", g_AtRfTxLen);
    atDebug("Packet ota time:%dms.", TxOtaTimeMs);
    
    while (1)
    {
        RF_TxSinglePkt(g_AtRfTxBuf, g_AtRfTxLen);
        LastSystemTickMs = SysTick_GetTick();
        
        while (1)
        {
            if (CHECK_RF_IRQ()) /* 检测到RF中断，高电平表示有中断 */
            {
                uint8_t IRQFlag;
        
                IRQFlag = RF_GetIRQFlag();    /* 获取中断标志位 */
                if (IRQFlag & RF_IRQ_TX_DONE) /* 发送完成中断 */
                {
                    RF_TurnoffPA();                /* 发送完成后须关闭PA */
                    RF_ClrIRQFlag(RF_IRQ_TX_DONE); /* 清除发送完成中断标志位 */
                    IRQFlag &= ~RF_IRQ_TX_DONE;
                    RF_EnterStandbyState(); /* 发送完成后须设置为RF_STATE_STB3状态 */
                    atDebug("RF TX DONE! Tx Count:%d.", ++CurrentCount); /* 打印发送次数 */
                    break;
                }
        
                if (IRQFlag)
                {
                    RF_ClrIRQFlag(IRQFlag); /* 清除未处理的中断标志位 */
                }
            }

            if ((SysTick_GetTick() - LastSystemTickMs) >= TxOtaTimeMs)
            {
                LastSystemTickMs = SysTick_GetTick();
                atDebug("RF tx timeout: 5s");
                goto exit;
            }

            if (ATCMD_CheckBreak())
            {
                atDebug("user break");
                goto exit;
            }
        }

        if (CurrentCount >= TxCount)
        {
            CurrentCount = 0;
            atDebug("RF tx %d packets done.", TxCount);
            goto exit;
        }
        else
        {
            RF_DelayMs(TxIntervalMs); /* 发送间隔，ms*/
        }
    }

exit:
    RF_SetRfState(RF_STATE_STB3);
    RF_ShutdownAnt();

    atDebug("RF exit tx mode");

    if (ret != RF_OK)
    {
        return AT_RESULT_FAILE;
    }
    else
    {
        return AT_RESULT_OK;
    }
}
AT_CMD_EXPORT("AT+TX", "=<Tx count>,<Tx interval in ms>", PN_NULL, PN_NULL, ATCMD_SetTxMode, PN_NULL);
AT_CMD_EXPORT("AT+TXMODE", "=<Tx count>,<Tx interval in ms>", PN_NULL, PN_NULL, ATCMD_SetTxMode, PN_NULL);

static at_result_t ATCMD_WriteReg(const char *args)
{
    int argc, page, reg, val;
    const char *req_expr = "=%x,%x,%x";

    argc = sscanf(args, req_expr, &page, &reg, &val);
    if (argc != 3)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("page:%02x", page);
    atDebug("reg:%02x", reg);
    atDebug("val:%02x", val);

    if (page > 1 || page < 0)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    if (reg > 0x7F || reg < 0x00)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    if (val > 0xFF || val < 0x00)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    RF_Err_t ret = RF_WritePageReg(page, reg, val);
    if (ret != RF_OK)
    {
        return AT_RESULT_FAILE;
    }
    else
    {
        return AT_RESULT_OK;
    }
}
AT_CMD_EXPORT("AT+WREG", "=<PAGE>,<REG>,<VAL>", PN_NULL, PN_NULL, ATCMD_WriteReg, PN_NULL);

static at_result_t ATCMD_ReadReg(const char *args)
{
    int argc, page, reg;
    const char *req_expr = "=%x,%x";

    argc = sscanf(args, req_expr, &page, &reg);
    if (argc != 2)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("page:0x%02x", page);
    atDebug("reg:0x%02x", reg);

    if (page > 1 || page < 0)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    if (reg > 0x7F || reg < 0x00)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    u8 val = RF_ReadPageReg(page, reg);

    atDebug("val:%d", val);
    printf("+RREG=%02X,%02X,%02X\r\n", page, reg, val);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+RREG", "=<PAGE>,<REG>", PN_NULL, PN_NULL, ATCMD_ReadReg, PN_NULL);

static at_result_t ATCMD_ReadRegs(const char *args)
{
    int argc, Page, Reg, Val;
    const char *req_expr = "=%x,%x,%x";
    uint8_t Buffer[255];

    argc = sscanf(args, req_expr, &Page, &Reg, &Val);
    if (argc != 3)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("page:0x%02x", Page);
    atDebug("reg:0x%02x", Reg);
    atDebug("val:0x%02x", Val);

    if (Page > 1 || Page < 0)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    if (Reg > 0x7F || Reg < 0x00)
    {
        return AT_RESULT_PARSE_FAILE;
    }
    if (Val > 0xFF || Val < 0x00)
    {
        return AT_RESULT_PARSE_FAILE;
    }
    
    RF_ReadPageRegs(Page, Reg, Buffer, Val);
    printf("+RREG=%02X ", Val);
    for (int i = 0; i < Val; i++)
    {
        printf("%02X ", Buffer[i]);
    }
    printf("\r\n");
    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+RREGS", "=<Len Hex0 Hex1 ...>", PN_NULL, PN_NULL, ATCMD_ReadRegs, PN_NULL);

static at_result_t ATCMD_WriteRegs(const char *args)
{
    int argc, page, reg, val;
    const char *req_expr = "=%x,%x,%x";
    uint8_t Buffer[255];

    argc = sscanf(args, req_expr, &page, &reg, &val);
    if (argc != 3)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    atDebug("page:%02x", page);
    atDebug("reg:%02x", reg);
    atDebug("val:%02x", val);

    if (page > 1 || page < 0)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    if (reg > 0x7F || reg < 0x00)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    if (val > 0xFF || val < 0x00)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    char *param = strchr(args, ' ');
    if (param)
    {
        param++;
    }
    param = strchr(param, ' ');
    if (param)
    {
        param++;
    }
    param = strchr(param, ' ');
    if (param)
    {
        param++;
    }

    // Parse the parameters and fill the buffer
    for (int i = 0; i < val; i++)
    {
        int value;

        argc = sscanf(param, "%x", &value);
        if (argc != 1)
        {
            return AT_RESULT_PARSE_FAILE;
        }

        Buffer[i] = (uint8_t)value;

        // Move to the next parameter
        param = strchr(param, ' ');
        if (param)
        {
            param++;
        }
    }
    
    RF_WritePageRegs(page, reg, Buffer, val);
    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+WREGS", "=<PAGE>,<REG>,<VAL>", PN_NULL, PN_NULL, ATCMD_WriteRegs, PN_NULL);

/**
 * @brief Dump all registers of PAN3029
 */
static at_result_t ATCMD_DumpInitVals(void)
{
    uint8_t i;
    uint8_t Buffer[128];

    for (i = 0; i <= 3; i++)
    {
        printf("dump page%d: \r\n", i);
        RF_ReadPageRegs(i, 0, Buffer, 128);
        for (i = 0; i < 128; i++)
        {
            printf("%02X ", Buffer[i]);
            if ((i + 1) % 16 == 0)
            {
                printf("\r\n");
            }
        }
    }

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+DUMP", PN_NULL, PN_NULL, PN_NULL, PN_NULL, ATCMD_DumpInitVals);

static at_result_t ATCMD_GetTxBuf(void)
{
    uint8_t *p = g_AtRfTxBuf;
    int count = g_AtRfTxLen;

    printf("+TXBUF=");

    printf("%02X ", (u8)count);

    for (int i = 0; i < count; i++)
    {
        printf("%02X ", p[i]);
    }

    printf("\r\n");

    return AT_RESULT_OK;
}

/**
 * @brief Set the transmit buffer
 * @param param The parameters string containing the length and hex values
 * @return AT_RESULT_OK on success, error code on failure
 * @note If you want to set the transmit buffer, use this command.
 *       AT+TXBUF=<Len> <Hex0> <Hex1> <Hex2> ...
 *       for example: 
 *       1. Tx 3bytes
 *       AT+TXBUF=03 01 02 03\r\n
 *       2. Tx 17bytes
 *       AT+TXBUF=11 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11\r\n
 */
static at_result_t ATCMD_SetTxBuf(const char *param)
{
    int argc, count;
    unsigned int value;

    // Parse the first parameter as the count
    argc = sscanf((const char *)param, "=%x ", &value);
    if (argc != 1)
    {
        return AT_RESULT_PARSE_FAILE;
    }

    count = (int)value;
    if (count > sizeof(g_AtRfTxBuf))
    {
        return AT_RESULT_PARSE_FAILE;
    }

    g_AtRfTxLen = (u8)value;

    param = strchr(param, ' ');
    if (param)
        param++;

    // Parse the parameters and fill the buffer
    for (int i = 0; i < count; i++)
    {
        argc = sscanf(param, "%x", &value);
        if (argc != 1)
        {
            return AT_RESULT_PARSE_FAILE;
        }

        g_AtRfTxBuf[i] = (u8)value;

        // Move to the next parameter
        param = strchr(param, ' ');
        if (param)
        {
            param++;
        }
    }

    // printf("TxLen:%d", g_AtRfTxLen);
    // print_hex((uint8_t *)g_AtRfTxBuf, g_AtRfTxLen);

    return AT_RESULT_OK;
}
AT_CMD_EXPORT("AT+TXBUF", "=<Len Hex0 Hex1 Hex2 ....>", PN_NULL, ATCMD_GetTxBuf, ATCMD_SetTxBuf, PN_NULL);
