/**
 *********************************************************************************************
 * @file    main.c
 * @brief   12_fhss/rx: 简化版 FHSS(跳频) 接收端，含配对与数据通信（BW=500K，SF=7）
 * @version V1.0.1
 * @date    2025-08-18
 * @note    工作流程：
 *          1) 启动后进入绑定频点监听 Bind 包；
 *          2) 解析 Bind，确定 fhssSeed/chNum/slotMs/startDelayMs，回复 ACK；
 *          3) 等待到 startDelayMs 后，按序列和时隙跳频接收；
 *          4) 打印接收的数据包信息。
 *********************************************************************************************
 */
#include "bsp.h"
#include "pan_rf.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* 与 TX 保持一致的宏 */
#define FHSS_BASE_FREQ_HZ     490500000u
#define FHSS_CH_SPACING_HZ    1000000u
#define FHSS_BIND_FREQ_HZ     (FHSS_BASE_FREQ_HZ)

#define SYNC_WORD             RF_MAC_PRIVATE_SYNCWORD

#define PKT_MAGIC_BIND        0xB1
#define PKT_MAGIC_ACK         0xA1
#define PKT_MAGIC_DATA        0xD1
#define PKT_VER               0x01

#define RX_UNIQUE_ID          0xCAFEBABEu

/* 全局 */
static uint32_t g_FhssFreqHz[64]; /* 预留到 64 */
static uint8_t  g_FhssOrder[64];
static uint8_t  g_FhssChNum = 0;
static uint16_t g_SlotMs = 10;
static uint16_t g_StartDelayMs = 500;
static uint32_t g_FhssSeed = 0;
static uint32_t g_FhssStartTick = 0;
static bool     g_Paired = false;
static unsigned int g_RxCount = 0;

/* 简易 PRNG，与 TX 一致 */
static uint32_t g_RngState;

static void rng_seed(uint32_t seed)
{
    g_RngState = seed ? seed : 0x6D2B79F5u;
}

static uint32_t rng_next(void)
{
    uint32_t x = g_RngState;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g_RngState = x;
    return x;
}

static void build_fhss_freq_table(uint32_t base, uint32_t step, uint8_t n, uint32_t outHz[])
{
    for (uint8_t i = 0; i < n; i++)
        outHz[i] = base + (uint32_t)i * step;
}

static void build_fhss_order(uint8_t n, uint8_t outIdx[])
{
    for (uint8_t i = 0; i < n; i++)
        outIdx[i] = i;
    for (int i = (int)n - 1; i > 0; i--)
    {
        uint32_t r = rng_next();
        uint32_t j = r % (uint32_t)(i + 1);
        uint8_t t = outIdx[i];
        outIdx[i] = outIdx[j];
        outIdx[j] = t;
    }
}

static void rf_common_config(void)
{
    RF_ConfigUserParams();
    RF_SetBW(RF_BW_500K);
    RF_SetSF(RF_SF7);
    RF_SetSyncWord(SYNC_WORD);
    RF_SetPreamLen(8);
}

/* 解析 Bind 包 */
static bool parse_bind_pkt(const uint8_t *buf, uint8_t len)
{
    if (len < 15)
        return false;

    if (buf[0] != PKT_MAGIC_BIND || buf[1] != PKT_VER)
        return false;

    uint32_t seed = (uint32_t)buf[2] | ((uint32_t)buf[3] << 8) | ((uint32_t)buf[4] << 16) | ((uint32_t)buf[5] << 24);
    uint8_t chNum = buf[6];
    uint16_t slotMs = (uint16_t)buf[7] | ((uint16_t)buf[8] << 8);
    uint16_t startDelayMs = (uint16_t)buf[9] | ((uint16_t)buf[10] << 8);
//    uint32_t txId = (uint32_t)buf[11] | ((uint32_t)buf[12] << 8) | ((uint32_t)buf[13] << 16) | ((uint32_t)buf[14] << 24);
    
    if (chNum == 0 || chNum > 64)
        return false;

    g_FhssSeed = seed;             /* 为发送端提供的随机种子 */
    g_FhssChNum = chNum;           /* 为发送端提供的信道数量 */
    g_SlotMs = slotMs;             /* 为发送端提供的时隙时间 */
    g_StartDelayMs = startDelayMs; /* 为发送端提供的启动延迟 */
    rng_seed(g_FhssSeed);
    build_fhss_freq_table(FHSS_BASE_FREQ_HZ, FHSS_CH_SPACING_HZ, g_FhssChNum, g_FhssFreqHz);
    build_fhss_order(g_FhssChNum, g_FhssOrder);

    return true;
}

/* 发送 ACK: [magic|ver|txId|rxId] */
static void send_ack(uint32_t txId)
{
    uint8_t ack[10];
    uint8_t i = 0;

    ack[i++] = PKT_MAGIC_ACK;
    ack[i++] = PKT_VER;
    ack[i++] = (uint8_t)txId; 
    ack[i++] = (uint8_t)(txId >> 8); 
    ack[i++] = (uint8_t)(txId >> 16); 
    ack[i++] = (uint8_t)(txId >> 24);
    ack[i++] = (uint8_t)RX_UNIQUE_ID; 
    ack[i++] = (uint8_t)(RX_UNIQUE_ID >> 8); 
    ack[i++] = (uint8_t)(RX_UNIQUE_ID >> 16); 
    ack[i++] = (uint8_t)(RX_UNIQUE_ID >> 24);

    RF_TxSinglePkt(ack, i);

    while (1)
    {
        if (CHECK_RF_IRQ())
        {
            uint8_t irq = RF_GetIRQFlag();
            if (irq & RF_IRQ_TX_DONE)
            {
                RF_TurnoffPA();
                RF_ClrIRQFlag(RF_IRQ_TX_DONE);
                irq &= ~RF_IRQ_TX_DONE;
                RF_EnterStandbyState();
                break;
            }
            if (irq) RF_ClrIRQFlag(irq);
        }
    }
}

/* 绑定阶段：监听 Bind 并回 ACK */
static bool do_pairing(void)
{
    printf("[RX] Wait binding on %uHz...\r\n", (unsigned)FHSS_BIND_FREQ_HZ);
    RF_SetFreq(FHSS_BIND_FREQ_HZ);
    RF_EnterContinousRxState();

    uint32_t bindStart = SysTick_GetTick();
    while (!g_Paired)
    {
        if (CHECK_RF_IRQ())
        {
            uint8_t irq = RF_GetIRQFlag();
            if (irq & RF_IRQ_RX_DONE)
            {
                g_RfRxPkt.Snr = RF_GetPktSnr();
                g_RfRxPkt.Rssi = RF_GetPktRssi();
                g_RfRxPkt.RxLen = RF_GetRecvPayload((uint8_t *)g_RfRxPkt.RxBuf);
                RF_ClrIRQFlag(RF_IRQ_RX_DONE); irq &= ~RF_IRQ_RX_DONE;

                if (parse_bind_pkt((uint8_t *)g_RfRxPkt.RxBuf, g_RfRxPkt.RxLen))
                {
                    /* 回复 ACK */
                    uint32_t txId = (uint32_t)g_RfRxPkt.RxBuf[11] | ((uint32_t)g_RfRxPkt.RxBuf[12] << 8) | ((uint32_t)g_RfRxPkt.RxBuf[13] << 16) | ((uint32_t)g_RfRxPkt.RxBuf[14] << 24);
                    send_ack(txId);

                    /* 设定启动时间：当前 tick + startDelay */
                    g_FhssStartTick = SysTick_GetTick() + g_StartDelayMs;
                    g_Paired = true;
                    printf("[RX] Pair OK. ch=%u slot=%ums startDelay=%ums\r\n", (unsigned)g_FhssChNum, (unsigned)g_SlotMs, (unsigned)g_StartDelayMs);
                    break;
                }
            }
            if (irq)
            {
                RF_ClrIRQFlag(irq);
            }
        }
        if (SysTick_GetTick() - bindStart > 10000) /* 10s 超时后继续等待 */
        {
            bindStart = SysTick_GetTick();
            printf("[RX] Binding still waiting...\r\n");
        }
    }
    RF_EnterStandbyState();
    return g_Paired;
}

int32_t main(void)
{
    int ret;

    BSP_ClockInit();
    BSP_SystickInit(1000u);
    BSP_TimerInit();
    BSP_GpioInit();
    BSP_UartInit();
    BSP_RFBusInit();

    ret = RF_Init();
    if (ret != RF_OK)
    {
        printf("RF init fail\r\n");
        while (1);
    }

    rf_common_config();

    if (!do_pairing())
    {
        printf("[RX] Pair failed\r\n");
        while(1);
    }

    /* 等待到启动时间 */
    while ((int32_t)(SysTick_GetTick() - g_FhssStartTick) < 0)
    {
        /* busy wait */
    }

    /* 跳频接收循环：在每个槽开始前切换频点，进入单次接收带超时窗口 */
    while (1)
    {
        uint32_t now = SysTick_GetTick();
        uint32_t elapsed = now - g_FhssStartTick;
        uint32_t slot = (elapsed / g_SlotMs) % g_FhssChNum;
        uint8_t chIdx = g_FhssOrder[slot];
        uint32_t freq = g_FhssFreqHz[chIdx];

        RF_SetFreq(freq);
        RF_EnterSingleRxWithTimeout(g_SlotMs - 1); /* 给一点余量 */

        uint32_t slotStart = g_FhssStartTick + (elapsed / g_SlotMs) * g_SlotMs;
        uint32_t slotEnd = slotStart + g_SlotMs; /* 槽结束时间 */
        while ((int32_t)(SysTick_GetTick() - slotEnd) < 0)
        {
            if (CHECK_RF_IRQ())
            {
                uint8_t irq = RF_GetIRQFlag();
                if (irq & RF_IRQ_RX_DONE)
                {
                    g_RfRxPkt.Snr = RF_GetPktSnr();
                    g_RfRxPkt.Rssi = RF_GetPktRssi();
                    g_RfRxPkt.RxLen = RF_GetRecvPayload((uint8_t *)g_RfRxPkt.RxBuf);
                    g_RxCount++;
                    printf("[RX] slot=%u ch=%u freq=%uHz len=%u\r\n",
                           (uint32_t)slot, (uint32_t)chIdx, (uint32_t)freq, (uint32_t)g_RfRxPkt.RxLen);
                    printf("+RxHexData:\r\n");
                    print_hex((uint8_t *)g_RfRxPkt.RxBuf, g_RfRxPkt.RxLen);
                    printf("SNR:%ddB, RSSI:%ddBm\r\n", (int)g_RfRxPkt.Snr, (int)g_RfRxPkt.Rssi);
                    RF_ClrIRQFlag(RF_IRQ_RX_DONE);
                    irq &= ~RF_IRQ_RX_DONE;
                }
                
                if (irq & RF_IRQ_RX_TIMEOUT)
                {
                    RF_ClrIRQFlag(RF_IRQ_RX_TIMEOUT);
                    irq &= ~RF_IRQ_RX_TIMEOUT;
                }

                if (irq)
                    RF_ClrIRQFlag(irq);
            }
        }
        RF_EnterStandbyState();
    }
}
