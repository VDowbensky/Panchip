
#include "pan_rf.h"
#include "oled.h"
#include "display.h"
#include "key_handle.h"
#include "utilities.h"

#define FREQ_LIST_NUM   3

/* Global variables for key handling */
uint8_t g_KeyRfTxBuf[256];     // Buffer for RF TX data
uint32_t g_KeyRfTxLen = 10;    // Length of RF TX data
uint32_t g_KeyRfTxMode = 0;    // RF TX mode
uint32_t g_KeyRfTxPower = 22;  // RF TX power
uint32_t g_KeyRfFreqIndex = 0; // RF frequency index
uint32_t g_KeyRfTxNumMax = 1;  // Max number of RF TX
uint32_t g_KeyRfTxCnt = 0;     // RF TX count
uint32_t g_KeyRfRxCnt = 0;     // RF RX count

/* Indicates whether repeat (burst/continuous) TX is active when TxMode > 0 */
bool g_TxRepeatActive = false;

/* Indicates whether the key is pressed */
uint32_t g_KeyPressed = 0;

/* Indicates whether the second key is pressed */
bool g_IsSecondKeyPressed = false;

/* UI test mode indicator cycling in [0,1,2].
 * 1: CHIPMODE_MODE1; others: CHIPMODE_MODE0. */
uint8_t g_UiTestModeIndex = 0;

/* Indicates whether TX continuous carrier wave test is active */
bool g_TxCarrierWaveActive = false;

/* User can change frequency list, FREQ_LIST_NUM defined in display.h*/
uint32_t g_KeyRfFreqList[FREQ_LIST_NUM] = {RF_FREQ_DEFAULT, 491000000, 510000000};

/* Indicates whether OTA is in progress */
bool g_RFTxOta = false;

typedef struct
{
    en_port_t port;
    en_pin_t  pin;
} BSP_KeyIn_Config;

static const BSP_KeyIn_Config BSP_KEYIN_PORT_PIN[BSP_KEY_ROW_NUM] = 
{
    {BSP_KEY1_PORT, BSP_KEY1_PIN},
    {BSP_KEY2_PORT, BSP_KEY2_PIN},
    {BSP_KEY3_PORT, BSP_KEY3_PIN},
    {BSP_KEY4_PORT, BSP_KEY4_PIN},
    {BSP_KEY5_PORT, BSP_KEY5_PIN},
    {BSP_KEY6_PORT, BSP_KEY6_PIN},
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

key_info_t key_list[6] =
{
    [0] = {
        .port = BSP_KEY2_PORT,
        .pin = BSP_KEY2_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = KEY_ClrCnt,             // Press K2
        .func_second = KEY_SetTestMode, // Press K1 and K2
    },
    [1] = {
        .port = BSP_KEY3_PORT,
        .pin = BSP_KEY3_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = KEY_StartTxRoutine,        // Press K3
        .func_second = KEY_SetFreqRoutine, // Press K1 and K3
    },
    [2] = {
        .port = BSP_KEY4_PORT,
        .pin = BSP_KEY4_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = KEY_SetModeRoutine,      // Press K4
        .func_second = KEY_SetCrRoutine, // Press K1 and K4
    },
    [3] = {
        .port = BSP_KEY5_PORT,
        .pin = BSP_KEY5_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = KEY_SetSfRoutine,        // Press K5
        .func_second = KEY_SetPlRoutine, // Press K1 and K5
    },
    [4] = {
        .port = BSP_KEY6_PORT,
        .pin = BSP_KEY6_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = KEY_SetBwRoutine,             // Press K6
        .func_second = KEY_SetTxpowerRoutine, // Press K1 and K6
    },

    [5] = {
        .port = BSP_KEY1_PORT,
        .pin = BSP_KEY1_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = NULL,
    },
};

void KEY_SetSfRoutine(void)
{
    uint8_t sf;

    RF_SetRfState(RF_STATE_STB3);
    sf = RF_GetSF();
    sf++;
    if (sf > 12)
    {
        sf = 5;
    }
    UI_SetSF(sf);
    OLED_Refresh();
    RF_SetSF(sf);
    if (g_UserTxMode)
    {
    }
    else
    {
        RF_TurnonRxAnt();
        RF_SetRfState(RF_STATE_RX);
    }
}

void KEY_SetBwRoutine(void)
{
    uint8_t bw;

    RF_SetRfState(RF_STATE_STB3);
    bw = RF_GetBandWidth();
    bw++;
    if (bw > 9)
    {
        bw = 6;
    }
    UI_SetBW(bw);
    OLED_Refresh();
    RF_SetBW(bw);
    if (g_UserTxMode)
    {
    }
    else
    {
        RF_TurnonRxAnt();
        RF_SetRfState(RF_STATE_RX);
    }
}

void KEY_SetPlRoutine(void)
{
    g_KeyRfTxLen += 10;
    if (g_KeyRfTxLen > 240)
    {
        g_KeyRfTxLen = 10;
    }
    UI_SetPayloadLen(g_KeyRfTxLen);

    /* 每次切换数据长度时，重新填充数据及计算CRC校验值 */
    for(int i = 0; i < g_KeyRfTxLen; i++)
    {
        g_KeyRfTxBuf[i] = i;
    }

    uint16_t crc16 = crc16_ccitt(g_KeyRfTxBuf, g_KeyRfTxLen - 2); /* 计算CRC校验值 */
    g_KeyRfTxBuf[g_KeyRfTxLen - 2] = (uint8_t)(crc16 >> 8);       /* CRC高字节 */
    g_KeyRfTxBuf[g_KeyRfTxLen - 1] = crc16 & 0xFF;                /* CRC低字节 */
    
    print_hex(g_KeyRfTxBuf, g_KeyRfTxLen); /* 打印发送数据 */

    OLED_Refresh();
}

void KEY_SetModeRoutine(void)
{
    g_KeyRfTxMode++;
    if (g_KeyRfTxMode > 3)
    {
        g_KeyRfTxMode = 0;
    }
    UI_SetTxMode(g_KeyRfTxMode);
    OLED_Refresh();

    if (g_TxCarrierWaveActive)
    {
        RF_StopTxContinuousWave();
        RF_ClrIRQFlag(RF_IRQ_TX_DONE);
        g_TxCarrierWaveActive = false;
    }

    if (g_KeyRfTxMode == 0)
    {
        g_KeyRfTxNumMax = 1;
    }
    else if (g_KeyRfTxMode == 1)
    {
        g_KeyRfTxNumMax = 100;
    }
    else if (g_KeyRfTxMode == 2)
    {
        g_KeyRfTxNumMax = 9999;
    }
    else
    {
        if (g_UserTxMode)
        {
            // 发送载波前须设置好频率和功率
            RF_SetFreq(g_KeyRfFreqList[g_KeyRfFreqIndex]);
            RF_SetTxPower(g_KeyRfTxPower);
            RF_StartTxContinuousWave();
            g_TxCarrierWaveActive = true;
        }
    }
}

void KEY_SetTxpowerRoutine(void)
{
    uint8_t TxPower;

    RF_SetRfState(RF_STATE_STB3);
    TxPower = RF_GetTxPower();

    if (TxPower >= 22)
    {
        TxPower = 1;
    }
    else
    {
        TxPower++;
    }
    g_KeyRfTxPower = TxPower;
    RF_SetTxPower(TxPower);
    SysTick_Delay(1);
    TxPower = RF_GetTxPower();

    UI_SetTxPower(TxPower);
    OLED_Refresh();
    if (g_UserTxMode)
    {
        if (g_TxCarrierWaveActive)
        {
            RF_StopTxContinuousWave();
            /* 发送载波前须设置好频率和功率 */
            RF_SetFreq(g_KeyRfFreqList[g_KeyRfFreqIndex]);
            RF_SetTxPower(g_KeyRfTxPower);
            RF_StartTxContinuousWave();
        }
    }
    else
    {
        RF_TurnonRxAnt();
        RF_SetRfState(RF_STATE_RX);
    }
}

void KEY_SetCrRoutine(void)
{
    uint8_t cr;

    RF_SetRfState(RF_STATE_STB3);
    cr = RF_GetCR();
    cr++;
    if (cr > 4)
    {
        cr = 1;
    }
    UI_SetCR(cr);
    OLED_Refresh();
    RF_SetCR(cr);
    if (g_UserTxMode)
    {
    }
    else
    {
        RF_TurnonRxAnt();
        RF_SetRfState(RF_STATE_RX);
    }
}

void KEY_StartTxRoutine(void)
{
    if (g_UserTxMode)
    {
        /* Toggle repeat TX flag when in TX mode */
        g_TxRepeatActive = !g_TxRepeatActive;
        
        if(g_RFTxOta)
        {
            printf("Tx packet is ongoing.\r\n");
            return; /* 如果有单包数据正在空中发送，则不处理发送 */
        }

        if (g_TxRepeatActive || (g_KeyRfTxNumMax == 1))
        {
            if (g_KeyRfTxNumMax != 1)
            {
                g_KeyRfTxCnt = 0;
            }
            g_RFTxOta = true; /* 标记有单包数据正在空中发送 */
            RF_TxSinglePkt(g_KeyRfTxBuf, g_KeyRfTxLen);
        }
    }
}

void KEY_SetFreqRoutine(void)
{
    uint32_t Freq;

    g_KeyRfFreqIndex++;
    if (g_KeyRfFreqIndex > FREQ_LIST_NUM - 1)
    {
        g_KeyRfFreqIndex = 0;
    }

    if (g_KeyRfFreqIndex == 0)
    {
        Freq = g_KeyRfFreqList[g_KeyRfFreqIndex];
    }
    else if (g_KeyRfFreqIndex == 1)
    {
        Freq = g_KeyRfFreqList[g_KeyRfFreqIndex];
    }
    else
    {
        Freq = g_KeyRfFreqList[g_KeyRfFreqIndex];
    }

    RF_SetRfState(RF_STATE_STB3);
    RF_SetFreq(Freq);

    Freq = RF_GetFreq();

    UI_SetFreq((Freq / 100000));

    OLED_Refresh();

    if (g_UserTxMode)
    {
    if (g_TxCarrierWaveActive)
        {
            RF_StopTxContinuousWave();
            /* 发送载波前须设置好频率和功率 */
            RF_SetFreq(g_KeyRfFreqList[g_KeyRfFreqIndex]);
            RF_SetTxPower(g_KeyRfTxPower);
            RF_StartTxContinuousWave();
        }
    }
    else
    {
        RF_TurnonRxAnt();
        RF_SetRfState(RF_STATE_RX);
    }
}

void KEY_ClrCnt(void)
{
    g_KeyRfTxCnt = 0;
    g_KeyRfRxCnt = 0;
    UI_SetTRxCnt(0);
    OLED_Refresh();
}

void KEY_SetTestMode(void)
{
    RF_SetRfState(RF_STATE_STB3);
    g_UiTestModeIndex++;
    if (g_UiTestModeIndex > 2)
    {
        g_UiTestModeIndex = 0;
    }

    if (g_UiTestModeIndex == 1)
    {
        RF_SetChipMode(CHIPMODE_MODE1); /* 设置芯片模式为MODE1 */
        RF_SetCRC(RF_CRC_OFF);          /* 关闭CRC校验 */
    }
    else
    {
        RF_SetChipMode(CHIPMODE_MODE0); /* 设置芯片模式为MODE0 */
        RF_SetCRC(RF_CRC_ON);           /* 开启CRC校验 */
    }

    UI_SetTestMode(g_UiTestModeIndex);
    OLED_Refresh();

    if (g_UserTxMode != TRUE)
    {
        RF_TurnonRxAnt();
        RF_SetRfState(RF_STATE_RX);
    }
}

void KEY_Scan(void)
{
    uint8_t i;
    uint8_t pin_state;
    uint8_t pin_state_second;
    static uint32_t sys_tick;
    
    sys_tick = SysTick_GetTick();

    if (sys_tick % 5)
    {
        return;
    }

    for (i = 0; i < 5; i++)
    {
        pin_state = PORT_GetBit(key_list[i].port, key_list[i].pin);

        if ((pin_state == RESET) && (key_list[i].first_state == SET) && (key_list[i].second_state == RESET))
        {
            g_KeyPressed |= (1 << i);
        }

        key_list[i].first_state = key_list[i].second_state;
        key_list[i].second_state = pin_state;
    }
    pin_state_second = PORT_GetBit(key_list[5].port, key_list[5].pin);
    if (pin_state_second == RESET)
    {
        g_IsSecondKeyPressed = true;
    }
    else
    {
        g_IsSecondKeyPressed = false;
    }
}

void KEY_Process(void)
{
    KEY_Scan();

    for (int i = 0; i < 5; i++)
    {
        if (g_KeyPressed & (1 << i))
        {
            if (g_IsSecondKeyPressed != true)
            {
                if (key_list[i].func != NULL)
                {
                    key_list[i].func();
                }
            }
            else
            {
                if (key_list[i].func_second != NULL)
                {
                    key_list[i].func_second();
                }
            }

            g_KeyPressed = 0;
            break;
        }
    }
}

void RF_RxDoneCb(void)
{
    g_KeyRfRxCnt++;
    UI_SetTRxCnt(g_KeyRfRxCnt);
    
    printf(">>Received a packet!\r\n");
    printf("+RxLen=%d, Rx Count=%d\n", g_RfRxPkt.RxLen, g_KeyRfRxCnt);
    printf("+RxHexData:\r\n");
    print_hex((uint8_t *)g_RfRxPkt.RxBuf, g_RfRxPkt.RxLen); /* 以十六进制方式打印接收数据 */
    printf("SNR:%ddB, RSSI:%ddBm \r\n", (int)g_RfRxPkt.Snr, (int)g_RfRxPkt.Rssi);
}

void RF_TxDoneCb(void)
{
    g_KeyRfTxCnt++;
    UI_SetTRxCnt(g_KeyRfTxCnt);
    OLED_Refresh();
    if ((g_KeyRfTxCnt < g_KeyRfTxNumMax) && g_TxRepeatActive)
    {
        SysTick_Delay(100);
        RF_TxSinglePkt(g_KeyRfTxBuf, g_KeyRfTxLen);
    }
    else
    {
        g_RFTxOta = false;  /* 标记数据发送完成 */
        g_TxRepeatActive = false;
    }
}

void RF_Process(void)
{
    if(g_TxCarrierWaveActive)
    {
        return;
    }
    
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
            BSP_LedToggle();
            RF_TxDoneCb();
            printf("Tx Count:%d\r\n", g_KeyRfTxCnt); /* 打印发送次数 */
        }
        if (IRQFlag & RF_IRQ_RX_DONE) /* 接收完成中断 */
        {
            g_RfRxPkt.Snr = RF_GetPktSnr();   /* 获取接收数据包的SNR值 */
            g_RfRxPkt.Rssi = RF_GetPktRssi(); /* 获取接收数据包的RSSI值 */
            
            /* 获取接收数据和长度 */
            g_RfRxPkt.RxLen = RF_GetRecvPayload((uint8_t *)g_RfRxPkt.RxBuf);

            if(g_UiTestModeIndex != 1)  /* CHIPMODE_MODE0 */
            {
                BSP_LedToggle();
                RF_RxDoneCb();
            }
            else /* CHIPMODE_MODE1 */
            {
                /* 获取接收数据和长度 */
                g_RfRxPkt.RxLen = RF_GetRecvPayload((uint8_t *)g_RfRxPkt.RxBuf);

                uint8_t DateLen = g_RfRxPkt.RxLen - 2; /* 数据长度为接收长度减去CRC校验的2字节 */
                uint16_t crc16 = crc16_ccitt((const uint8_t *)g_RfRxPkt.RxBuf, DateLen); /* 计算CRC校验值 */
                
                printf("CRC16=0x%04X\r\n", crc16); /* 打印CRC校验值 */
                
                /* 检查CRC校验 */
                if ((g_RfRxPkt.RxBuf[g_RfRxPkt.RxLen - 2] == (crc16 >> 8)) 
                     && (g_RfRxPkt.RxBuf[g_RfRxPkt.RxLen - 1] == (crc16 & 0xFF))) 
                {
                    BSP_LedToggle();
                    RF_RxDoneCb();
                }
                else
                {
                    printf("CRC error, RxLen=%d\r\n", g_RfRxPkt.RxLen);
                    printf("RxHexData:\r\n");
                    print_hex((uint8_t *)g_RfRxPkt.RxBuf, g_RfRxPkt.RxLen); /* 以十六进制方式打印接收数据 */
                }
            }
            RF_ClrIRQFlag(RF_IRQ_RX_DONE); /* 清除接收完成中断标志位 */
            IRQFlag &= ~RF_IRQ_RX_DONE;
        }
        if (IRQFlag & RF_IRQ_CRC_ERR && (g_UiTestModeIndex == 1)) /* CRC错误中断 */
        {
            RF_ClrIRQFlag(RF_IRQ_CRC_ERR); /* 清除CRC错误中断标志位 */
            IRQFlag &= ~RF_IRQ_CRC_ERR;
            printf(">>RF_IRQ_CRC_ERR\r\n");
        }
        if (IRQFlag & RF_IRQ_RX_TIMEOUT) /* 接收超时中断 */
        {
            RF_ClrIRQFlag(RF_IRQ_RX_TIMEOUT); /* 清除接收超时中断标志位 */
            IRQFlag &= ~RF_IRQ_RX_TIMEOUT;
            printf(">>RF_IRQ_RX_TIMEOUT\r\n");
        }

        if (IRQFlag)
        {
            RF_ClrIRQFlag(IRQFlag); /* 清除未处理的中断标志位 */
        }
    }
}

void KEY_Init(void)
{
    int i;

    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Enable;
    stcPortInit.enPinMode = Pin_Mode_In;
    PORT_DebugPortSetting(TRST, Disable);

    for (i = 0U; i < BSP_KEY_ROW_NUM; i++)
    {
        PORT_Init(BSP_KEYIN_PORT_PIN[i].port, BSP_KEYIN_PORT_PIN[i].pin, &stcPortInit);
    }

    for (i = 0U; i < g_KeyRfTxLen; i++)
    {
        g_KeyRfTxBuf[i] = i;
    }

    uint16_t crc16 = crc16_ccitt(g_KeyRfTxBuf, g_KeyRfTxLen - 2); /* 计算CRC校验值 */
    g_KeyRfTxBuf[g_KeyRfTxLen - 2] = (uint8_t)(crc16 >> 8);       /* CRC高字节 */
    g_KeyRfTxBuf[g_KeyRfTxLen - 1] = crc16 & 0xFF;                /* CRC低字节 */
}


