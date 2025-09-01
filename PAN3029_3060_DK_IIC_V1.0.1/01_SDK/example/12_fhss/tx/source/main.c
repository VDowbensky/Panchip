/**
 *********************************************************************************************
 * @file    main.c
 * @brief   12_fhss/tx: 简化版 FHSS(跳频) 发送端，含配对与数据通信（BW=500K，SF=7）
 * @version V1.0.1
 * @date    2025-08-18
 * @note    工作流程：
 *          1) 上电后进入配对模式：在绑定频点循环发送 Bind 包并短暂监听 ACK；
 *          2) 收到 ACK 后，双方以相同随机种子生成相同 FHSS 跳频序列；
 *          3) 在约定的 startDelayMs 之后，以 slotMs 为周期按序列跳频并发送数据包；
 *          4) 若收不到 ACK，可继续保持配对广播。
 *********************************************************************************************
 */
#include "bsp.h"
#include "pan_rf.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* ---------------- 配置参数（可按需修改） ---------------- */
#define FHSS_BASE_FREQ_HZ     490500000u          /* 跳频基准频点（示例） */
#define FHSS_CH_SPACING_HZ    1000000u            /* 频点间隔：1MHz */
#define FHSS_CH_NUM           16u                 /* 跳频信道数量 */
#define FHSS_SLOT_MS          100u                /* 每个信道驻留时间(ms) */
#define FHSS_GUARD_MS         2u                  /* 保护间隔(ms)，TX发送后到下个槽前的冗余 */
#define FHSS_BIND_FREQ_HZ     (FHSS_BASE_FREQ_HZ) /* 绑定频点：使用基准频点 */
#define FHSS_BIND_INTERVAL_MS 50u                 /* 绑定广播周期 */
#define FHSS_START_DELAY_MS   500u                /* 配对成功后，双方约定延迟启动 FHSS 的时间 */

#define SYNC_WORD             RF_MAC_PRIVATE_SYNCWORD

/* Bind/ACK 包标识 */
#define PKT_MAGIC_BIND        0xB1
#define PKT_MAGIC_ACK         0xA1
#define PKT_MAGIC_DATA        0xD1
#define PKT_VER               0x01

/* TX 与 RX 的简易 ID（示例用） */
#define TX_UNIQUE_ID          0x1234ABCDu

/* ---------------- FHSS 全局 ---------------- */
static uint32_t g_FhssFreqHz[FHSS_CH_NUM];
static uint8_t  g_FhssOrder[FHSS_CH_NUM];
static uint32_t g_FhssSeed = 0;     /* 由配对确定 */
static uint32_t g_FhssStartTick = 0;/* 槽0的起始 tick（ms） */
static bool     g_Paired = false;

/* 发送计数与工作缓冲 */
static uint32_t g_TxCount = 0;
static uint8_t  g_TxBuf[24];

/* ---------------- 简易随机与序列生成 ---------------- */
static uint32_t g_RngState;

static void rng_seed(uint32_t seed)
{
    g_RngState = seed ? seed : 0x6D2B79F5u; /* 避免 0 种子 */
}
static uint32_t rng_next(void)
{
    /* xorshift32 */
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
    {
        outHz[i] = base + (uint32_t)i * step;
    }
}

static void build_fhss_order(uint8_t n, uint8_t outIdx[])
{
    /* 初始化 0..n-1 */
    for (uint8_t i = 0; i < n; i++)
        outIdx[i] = i;
    /* Fisher-Yates 洗牌 */
    for (int i = (int)n - 1; i > 0; i--)
    {
        uint32_t r = rng_next();
        uint32_t j = r % (uint32_t)(i + 1);
        uint8_t tmp = outIdx[i];
        outIdx[i] = outIdx[j];
        outIdx[j] = tmp;
    }
}

/* ---------------- RF 公共配置 ---------------- */
static void rf_common_config(void)
{
    RF_ConfigUserParams();          /* 默认参数 */
    RF_SetBW(RF_BW_500K);           /* 指定 500K */
    RF_SetSF(RF_SF7);               /* 指定 SF7 */
    RF_SetSyncWord(SYNC_WORD);      /* 设置私有同步字 */
    RF_SetPreamLen(8);              /* 示例前导码 8 */
}

/* ---------------- Bind/ACK 与数据帧封装 ---------------- */
/* Bind 包: [magic|ver|seed32|chNum|slotMs|startDelayMs|txId] = 1+1+4+1+2+2+4 = 15 字节 */
static uint8_t build_bind_pkt(uint8_t *buf)
{
    uint8_t i = 0;
    uint32_t seed = g_FhssSeed;
    buf[i++] = PKT_MAGIC_BIND;
    buf[i++] = PKT_VER;
    buf[i++] = (uint8_t)(seed);
    buf[i++] = (uint8_t)(seed >> 8);
    buf[i++] = (uint8_t)(seed >> 16);
    buf[i++] = (uint8_t)(seed >> 24);
    buf[i++] = FHSS_CH_NUM;
    buf[i++] = (uint8_t)(FHSS_SLOT_MS);
    buf[i++] = (uint8_t)(FHSS_SLOT_MS >> 8);
    buf[i++] = (uint8_t)(FHSS_START_DELAY_MS);
    buf[i++] = (uint8_t)(FHSS_START_DELAY_MS >> 8);
    buf[i++] = (uint8_t)(TX_UNIQUE_ID);
    buf[i++] = (uint8_t)(TX_UNIQUE_ID >> 8);
    buf[i++] = (uint8_t)(TX_UNIQUE_ID >> 16);
    buf[i++] = (uint8_t)(TX_UNIQUE_ID >> 24);
    return i;
}

/* 解析 ACK: [magic|ver|txId|rxId] = 1+1+4+4 = 10 字节 */
static bool parse_ack_pkt(const uint8_t *buf, uint8_t len)
{
    if (len < 10) return false;
    if (buf[0] != PKT_MAGIC_ACK || buf[1] != PKT_VER) return false;
    uint32_t txId = (uint32_t)buf[2] | ((uint32_t)buf[3] << 8) | ((uint32_t)buf[4] << 16) | ((uint32_t)buf[5] << 24);
    (void)txId;
    return (txId == TX_UNIQUE_ID);
}

/* 数据包: [magic|ver|cnt32|pay...], 这里放 8 字节演示数据 */
static uint8_t build_data_pkt(uint8_t *buf)
{
    uint8_t i = 0;

    buf[i++] = PKT_MAGIC_DATA;
    buf[i++] = PKT_VER;
    buf[i++] = (uint8_t)(g_TxCount >> 24);
    buf[i++] = (uint8_t)(g_TxCount >> 16);
    buf[i++] = (uint8_t)(g_TxCount >> 8);
    buf[i++] = (uint8_t)(g_TxCount);

    /* 示例负载 */
    for (uint8_t k = 0; k < 8; k++)
    {
        buf[i++] = (uint8_t)(0xA0 + ((g_TxCount + k) & 0x0F));
    }

    return i;
}

/* ---------------- 配对阶段：广播 Bind 并接收 ACK ---------------- */
static bool do_pairing(void)
{
    uint32_t lastBindTick = 0;
    uint8_t pkt[32];

    printf("[TX] Enter binding... base=%uHz, ch=%u, slot=%ums, delay=%ums\r\n",
           (unsigned)FHSS_BASE_FREQ_HZ, (unsigned)FHSS_CH_NUM,
           (unsigned)FHSS_SLOT_MS, (unsigned)FHSS_START_DELAY_MS);

    /* 在绑定频点上反复发送 bind 并短暂监听 ack */
    while (!g_Paired)
    {
        uint32_t now = SysTick_GetTick();
        if (now - lastBindTick >= FHSS_BIND_INTERVAL_MS)
        {
            lastBindTick = now;

            /* 构建并发送 bind 包 */
            RF_SetFreq(FHSS_BIND_FREQ_HZ);
            uint8_t len = build_bind_pkt(pkt);
            RF_TxSinglePkt(pkt, len);

            /* 等待 TX_DONE */
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

            /* 发送后切到带超时短接收，等待 ACK */
            RF_EnterSingleRxWithTimeout(200); /* 200ms 窗口 */
            uint32_t rxWaitStart = SysTick_GetTick();
            while (SysTick_GetTick() - rxWaitStart < 250)
            {
                if (CHECK_RF_IRQ())
                {
                    uint8_t irq = RF_GetIRQFlag();
                    if (irq & RF_IRQ_RX_DONE)
                    {
                        g_RfRxPkt.Snr = RF_GetPktSnr();
                        g_RfRxPkt.Rssi = RF_GetPktRssi();
                        g_RfRxPkt.RxLen = RF_GetRecvPayload((uint8_t *)g_RfRxPkt.RxBuf);
                        RF_ClrIRQFlag(RF_IRQ_RX_DONE);
                        irq &= ~RF_IRQ_RX_DONE;

                        if (parse_ack_pkt((uint8_t *)g_RfRxPkt.RxBuf, g_RfRxPkt.RxLen))
                        {
                            printf("[TX] Got ACK. Pair success.\r\n");
                            g_Paired = true;
                            /* 设定 FHSS 启动时间 */
                            g_FhssStartTick = SysTick_GetTick() + FHSS_START_DELAY_MS;
                            break;
                        }
                    }
                    if (irq)
                    {
                        RF_ClrIRQFlag(irq);
                    }
                }
            }
            /* 回到待机，下一轮 */
            RF_EnterStandbyState();
        }
    }
    return g_Paired;
}

/* ---------------- 主程序 ---------------- */
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
        while(1);
    }

    /* 固定 BW=500K, SF=7 */
    rf_common_config();

    /* 生成随机种子并构建 FHSS 表（最终以配对包为准，这里先准备一份） */
    stc_efm_unique_id_t uuid = EFM_ReadUID(); /* 为EVB板上唯一ID号 */
    g_FhssSeed = uuid.uniqueID3;
    rng_seed(g_FhssSeed);
    build_fhss_freq_table(FHSS_BASE_FREQ_HZ, FHSS_CH_SPACING_HZ, FHSS_CH_NUM, g_FhssFreqHz);
    build_fhss_order(FHSS_CH_NUM, g_FhssOrder);

    /* 进入绑定流程 */
    if (!do_pairing())
    {
        printf("[TX] Pairing failed.\r\n");
        while (1);
    }

    /* 配对成功：再次用种子生成（此处仍用同一 g_FhssSeed），确保双方一致 */
    rng_seed(g_FhssSeed);
    build_fhss_order(FHSS_CH_NUM, g_FhssOrder);

    printf("[TX] FHSS start at tick=%u, slot=%ums, ch=%u\r\n",
           (unsigned)g_FhssStartTick, (unsigned)FHSS_SLOT_MS, (unsigned)FHSS_CH_NUM);

    /* 等待到启动时间 */
    while ((int32_t)(SysTick_GetTick() - g_FhssStartTick) < 0) { /* busy wait */ }

    /* 进入 FHSS 数据发送循环 */
    while (1)
    {
        uint32_t now = SysTick_GetTick();
        uint32_t elapsed = now - g_FhssStartTick;
        uint32_t slot = (elapsed / FHSS_SLOT_MS) % FHSS_CH_NUM;
        uint8_t chIdx = g_FhssOrder[slot];
        uint32_t freq = g_FhssFreqHz[chIdx];

        /* 设置频点并发送一帧数据 */
        RF_SetFreq(freq);
        uint8_t dlen = build_data_pkt(g_TxBuf);
        RF_TxSinglePkt(g_TxBuf, dlen);

        /* 等待 TX_DONE */
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
                    BSP_LedToggle();
                    printf("[TX] slot=%lu ch=%u freq=%luHz cnt=%lu\r\n",
                           (unsigned long)slot, (unsigned)chIdx, (unsigned long)freq, (unsigned long)g_TxCount);
                    g_TxCount++;
                    break;
                }
                if (irq) RF_ClrIRQFlag(irq);
            }
        }

        /* 等待到下个槽起点（保留保护时间） */
        uint32_t nextSlotTick = g_FhssStartTick + (elapsed / FHSS_SLOT_MS + 1) * FHSS_SLOT_MS;
        while ((int32_t)(SysTick_GetTick() - nextSlotTick) < 0)
        {
            /* 空转 */
        }
    }
}
