/**
 *********************************************************************************************
 * @file    main.c
 * @brief   本示例演示了如何使用PAN3029/3060接收带MAPM的数据包的功能
 * @version V1.0.1
 * @date    2025-08-18
 * @author  Panchip Team
 * @note    该示例功能和流程如下：
 *          1. 初始化系统时钟、SysTick定时器、定时器、GPIO、UART和SPI。
 *          2. 调用RF_Init函数初始化PAN3029/3060模块。
 *          3. 配置RF模块的用户参数，包括频率、扩频因子(SF)、带宽(BW)、前导码(Preamble)、CRC等。
 *          4. 配置MAPM相关参数，包括地址、fn、fnm、gn、gfs、pg1、pgn和pn等。
 *          5. 进入连续接收模式，通过CAD检测信道状态：
 *             - 如果信道忙（检测到有效chirp符号），进入接收模式，接收带MAPM的数据包。
 *             - 如果信道空闲（未检测到有效chirp符号），进入睡眠模式，降低功耗。
 *          6. 在接收模式下，接收到MAPM的地址和计数器值
 *             - 如果接收到的MAPM地址与预期地址匹配，计算需要的休眠时间，并进入睡眠模式。
 *             - 如果接收到的MAPM地址不匹配，忽略该数据包，进入休眠模式。
 *          7. 在睡眠模式下，定时唤醒后
 *             - 如果判断MAPM匹配标志位为true，则进入接收状态接收数据包。
 *             - 如果判断MAPM匹配标志位为false，则继续检测信道状态，循环执行上述流程。
 * @note    该示例需要01_SDK/example/08_mapm/mapm_tx示例配合一起进行功能演示，功能演示需要两块开发板：
 *          1. 一块开发板作为发送端，使用01_SDK/example/08_mapm/mapm_tx示例代码发送带MAPM的数据包。
 *          2. 另一块开发板作为接收端，使用本示例代码接收带MAPM的数据包。
 * Copyright (C) 2025 Panchip Technology Corp. All rights reserved.
 *********************************************************************************************
 */
#include "bsp.h"
#include "pan_rf.h"

#define MAPM_SF_INDEX 0 // 使用g_MapmCfg[0]作为接收端的MAPM配置

/**
 * @brief 一个符号的时间，单位为微秒
 * @note 这个时间是根据当前的SF和BW计算得出的
 */
static uint32_t g_OneSymbolTimeUs;

/**
 * @brief 预期的CAD检测时间 
 * @note 不同的SF和BW具有不同的CAD检测时间
 */
static uint32_t g_ExpectedCadTimeMs;

/**
 * @brief 预期的休眠时间
 * @note 这个休眠时间是根据MAPM中的最后一个group中的计数器来计算的
 * @note 休眠时间的计算公式为：g_ExpectedSleepTimeMs = g_OneSymbolTimeUs * (g_CurrMapmCfg.fn * g_CurrMapmCfg.fnm + g_CurrMapmCfg.pn + g_CurrMapmCfg.pgn * g_CurrMapmCfg.gn)
 */
static uint32_t g_ExpectedSleepTimeMs;  //Sleep timing

/**
 * @brief 当前系统时钟的毫秒计数
 * @note 这个变量用于记录当前系统时钟的毫秒数
 */
static uint32_t g_CurrentSysTickMs;

/**
 * @brief 当前的MAPM配置
 * @note 这个变量用于存储当前的MAPM配置参数
 */
RF_MapmCfg_t g_CurrMapmCfg;

/**
 * @brief PAN3029/3060 MAPM配置参数表
 * @note 每个配置包含地址、fn、fnm、gfs、gn、pg1、pgn和pn等参数
 * @note 本例程中仅使用g_MapmCfg[0]作为接收端的MAPM配置作为演示，
 *       如有需要，可以根据实际情况选择其他配置。
 * @note 发送端的Addr[0]须与接收端的Addr[0]一致，否则接收端不会触发mapm中断
 */
const RF_MapmCfg_t g_MapmCfg[] =
{
    { /* SF7 */
        .Addr = {0x73, 0x62, 0x21, 0x81}, /* 发送的MAPM地址, 当gn=3且gfs=RF_MAPM_GRP_COUNTER时，仅Addr[0]和 Addr[1]有效 */
        .fn = 64,                         /* 发送64个field */
        .fnm = 0,                         /* 每个field发送1次 */
        .gn = 3,                          /* 每个field中有3个group */
        .gfs = RF_MAPM_GRP_COUNTER,       /* 最后一个group的载荷功能为计数器 */
        .pg1 = 10,                        /* 第一个group的前导码个数为10 */
        .pgn = 120,                       /* 其它group的前导码个数为120 */
        .pn = 8                           /* 数据包的前导码个数为8 */
    },
    { /* SF8 */
        .Addr = {0x73, 0x62, 0x54, 0x81}, /* 发送的MAPM地址, 当gn=3且gfs=RF_MAPM_GRP_COUNTER时，仅Addr[0]和 Addr[1]有效 */
        .fn = 32,                         /* 发送32个field */
        .fnm = 0,                         /* 每个field发送1次 */
        .gn = 3,                          /* 每个field中有3个group */
        .gfs = RF_MAPM_GRP_COUNTER,       /* 最后一个group的载荷功能为计数器 */
        .pg1 = 10,                        /* 第一个group的前导码个数为10 */
        .pgn = 120,                       /* 其它group的前导码个数为120 */
        .pn = 8                           /* 数据包的前导码个数为8 */
    },
    { /* SF9 */
        .Addr = {0x73, 0x62, 0x54, 0x81}, /* 发送的MAPM地址, 当gn=3且gfs=RF_MAPM_GRP_COUNTER时，仅Addr[0]和 Addr[1]有效 */
        .fn = 16,                         /* 发送16个field */
        .fnm = 0,                         /* 每个field发送1次 */
        .gn = 3,                          /* 每个field中有3个group */
        .gfs = RF_MAPM_GRP_COUNTER,       /* 最后一个group的载荷功能为计数器 */
        .pg1 = 10,                        /* 第一个group的前导码个数为10 */
        .pgn = 120,                       /* 其它group的前导码个数为120 */
        .pn = 8                           /* 数据包的前导码个数为8 */
    },
    { /* SF10, only for PAN3029 */
        .Addr = {0x73, 0x62, 0x54, 0x81}, /* 发送的MAPM地址, 当gn=3且gfs=RF_MAPM_GRP_COUNTER时，仅Addr[0]和 Addr[1]有效 */
        .fn = 8,                          /* 发送8个field */
        .fnm = 0,                         /* 每个field发送1次 */
        .gn = 3,                          /* 每个field中有3个group */
        .gfs = RF_MAPM_GRP_COUNTER,       /* 最后一个group的载荷功能为计数器 */
        .pg1 = 10,                        /* 第一个group的前导码个数为10 */
        .pgn = 120,                       /* 其它group的前导码个数为120 */
        .pn = 8                           /* 数据包的前导码个数为8 */
    },
    { /* SF11, only for PAN3029 */
        .Addr = {0x73, 0x62, 0x54, 0x81}, /* 发送的MAPM地址, 当gn=3且gfs=RF_MAPM_GRP_COUNTER时，仅Addr[0]和 Addr[1]有效 */
        .fn = 6,                          /* 发送6个field */
        .fnm = 0,                         /* 每个field发送1次 */
        .gn = 3,                          /* 每个field中有3个group */
        .gfs = RF_MAPM_GRP_COUNTER,       /* 最后一个group的载荷功能为计数器 */
        .pg1 = 10,                        /* 第一个group的前导码个数为10 */
        .pgn = 120,                       /* 其它group的前导码个数为120 */
        .pn = 8                           /* 数据包的前导码个数为8 */
    },
    { /* SF12, only for PAN3029 */
        .Addr = {0x73, 0x62, 0x54, 0x81}, /* 发送的MAPM地址, 当gn=3且gfs=RF_MAPM_GRP_COUNTER时，仅Addr[0]和 Addr[1]有效 */
        .fn = 6,                          /* 发送6个field */
        .fnm = 0,                         /* 每个field发送1次 */
        .gn = 3,                          /* 每个field中有3个group */
        .gfs = RF_MAPM_GRP_COUNTER,       /* 最后一个group的载荷功能为计数器 */
        .pg1 = 10,                        /* 第一个group的前导码个数为10 */
        .pgn = 120,                       /* 其它group的前导码个数为120 */
        .pn = 8                           /* 数据包的前导码个数为8 */
    }
};

/**
 * @brief MAPM操作状态机
 * @note 该状态机用于控制MAPM的操作流程
 *       - STATE_SLEEP: 休眠状态
 *       - STATE_IDLE: 空闲状态，等待CAD检测
 *       - STATE_CAD: CAD检测状态
 *       - STATE_RX: 接收状态，接收MAPM数据包
 */
typedef enum
{
    STATE_SLEEP = 0,
    STATE_IDLE,
    STATE_CAD,
    STATE_RX,
}MapmOpState_t;

/* g_MapmOpState为MAPM操作状态机变量 */
MapmOpState_t g_MapmOpState = STATE_IDLE;

/* g_MapmMatchFlag为MAPM匹配标志位 */
bool g_MapmMatchFlag = false;

/* g_RxCount为PAN3029/3060 接收计数变量 */
unsigned int g_RxCount = 0;

int32_t main(void)
{
    int ret;

    BSP_ClockInit();        /* 初始化系统时钟 */
    BSP_SystickInit(1000u); /* 初始化SysTick定时器，1ms */
    BSP_TimerInit();        /* 初始化定时器，1us */
    BSP_GpioInit();         /* 初始化GPIO */
    BSP_UartInit();         /* 初始化UART */
    BSP_RFBusInit();        /* 初始化SPI/I2C */

    ret = RF_Init();        /* PAN3029/3060初始化 */
    if (ret != RF_OK)
    {
        printf("RF init fail\r\n");
        while(1);
    }

    RF_ConfigUserParams(); /* 配置 Frequency、SF、BW、Preamble、CRC等参数 */

    g_CurrMapmCfg = g_MapmCfg[MAPM_SF_INDEX]; /* 复制g_MapmCfg[0]配置到g_CurrMapmCfg中 */
    RF_ConfigMapm((RF_MapmCfg_t *)&g_CurrMapmCfg);
    RF_SetMapmAddr((uint8_t *)g_CurrMapmCfg.Addr, g_CurrMapmCfg.gn);
    RF_EnableMapm();

    RF_ClrIRQFlag(0xFF);        /* 清除中断标志位 */
    RF_EnterContinousRxState(); /* 进入连续接收模式 */

    g_OneSymbolTimeUs = RF_GetOneSymbolTime(RF_BW_DEFAULT, RF_SF_DEFAULT); /* 每个symbol的时间(us) */

    printf("RF mapm rx test start.\r\n");

    while(1)
    {
        switch(g_MapmOpState)
        {
            case STATE_IDLE:
            {
                RF_StartCad(RF_CAD_THRESHOLD_10, RF_CAD_04_SYMBOL); /* Start CAD detection */
                /* 下面代码中的RF_CAD_04_SYMBOL + 1表示多加一个符号时间的余量来保证CAD检测的可靠性
                   下面代码中360us时间对应的是PAN3029/3060的从STB3到接收状态的建立时间 */
                g_ExpectedCadTimeMs = (g_OneSymbolTimeUs * (RF_CAD_04_SYMBOL + 1) + 360) / 1000; /* Expected CAD detection time */
                if (g_ExpectedCadTimeMs == 0)
                {
                    g_ExpectedCadTimeMs = 1;
                }
                printf("Expected CAD Time: %d ms\r\n", g_ExpectedCadTimeMs);
                g_CurrentSysTickMs = SysTick_GetTick(); /* Get current system tick */
                g_MapmOpState = STATE_CAD;
                break;
            }
            case STATE_CAD:
            {
                if(SysTick_GetTick() - g_CurrentSysTickMs >= g_ExpectedCadTimeMs) /* CAD detection timeout */
                {
                    if (CHECK_RF_CAD()) /* CAD detection result */
                    {
                        printf("CAD detected\r\n");
                        RF_StopCad(); /* Stop CAD detection */
                        RF_EnterContinousRxState(); /* 进入连续接收模式 */
                        g_RfRxPkt.MapmRxIndex = 0;  /* Reset MAPM RX index */
                        g_MapmOpState = STATE_RX;
                    }
                    else
                    {
                        printf("CAD not detected, RF enter sleep.\r\n");
                        RF_StopCad(); /* Stop CAD detection */
                        RF_EnterSleepState(); /* Enter sleep mode */
                        g_CurrMapmCfg.fn = g_MapmCfg[MAPM_SF_INDEX].fn; /* 将g_CurrMapmCfg.fn还原为初始化的默认值，以获得正确的CAD not detected的休眠时间 */
                        g_ExpectedSleepTimeMs = RF_GetLeftMapmTime(&g_CurrMapmCfg, g_OneSymbolTimeUs) / 2;
                        printf("Expected Sleep Time: %d ms\r\n", g_ExpectedSleepTimeMs);
                        g_CurrentSysTickMs = SysTick_GetTick(); /* Get current system tick */
                        g_MapmOpState = STATE_SLEEP;
                    }
                }
                break;
            }
            case STATE_RX:
            {
                if (CHECK_RF_IRQ()) /* 检测到RF中断，高电平表示有中断 */
                {
                    uint8_t IRQFlag;
                    g_CurrentSysTickMs = SysTick_GetTick(); /* Get current system tick */
                    IRQFlag = RF_GetIRQFlag();              /* 获取中断标志位 */
                    if ((IRQFlag & RF_IRQ_MAPM_DONE))       /* MAPM完成中断且还未进行MAPM校验 */
                    {
                        uint8_t MapmAddr;

                        IRQFlag &= ~RF_IRQ_MAPM_DONE;
                        MapmAddr = RF_ReadPageReg(0, 0x6E); /* 获取收到的MAPM地址 */
                        // printf("Recv Mapm:0x%02X\r\n", MapmAddr);
                        g_RfRxPkt.MapmRxBuf[g_RfRxPkt.MapmRxIndex++] = MapmAddr; /* 存储MAPM地址至缓存 */
                        RF_DelayUs(g_OneSymbolTimeUs);                           /* 每次处理MAPM中断间隔需大于1个Chirp时长 */
                        if (g_RfRxPkt.MapmRxIndex == g_CurrMapmCfg.gn)           /* MAPM地址接收完成 */
                        {
                            printf("Recv MAPM Addr: ");
                            print_hex((uint8_t *)g_RfRxPkt.MapmRxBuf, g_RfRxPkt.MapmRxIndex); /* 打印收到的MAPM地址 */
                            g_CurrMapmCfg.fn = g_RfRxPkt.MapmRxBuf[g_CurrMapmCfg.gn - 1];     /* 获取剩余filed个数 */
                            
                            /* 比较接收的MAPM地址和本地的MAPM地址是否匹配 */
                            if (memcmp((const uint8_t *)g_RfRxPkt.MapmRxBuf, g_CurrMapmCfg.Addr, g_CurrMapmCfg.gn - 1))
                            {
                                /* Mapm的字节地址不匹配，rf进入睡眠状态 */
                                printf("MAPM Addr not match, RF enter sleep.\r\n");
                                g_RfRxPkt.MapmRxIndex = 0; /* 重置MAPM RX索引 */
                                RF_EnterSleepState();      /* Enter sleep mode */

                                /* 根据当前的MAPM配置和剩余的filed个数计算预期的休眠时间 */
                                g_ExpectedSleepTimeMs = RF_GetLeftMapmTime(&g_CurrMapmCfg, g_OneSymbolTimeUs);
                                
                                 /* 加上数据包的前导码时间（因为接收端无法预知对方发送的数据长度，此处仅加上数据包的前导码长度）*/
                                g_ExpectedSleepTimeMs += g_MapmCfg[MAPM_SF_INDEX].pn * g_OneSymbolTimeUs / 1000;
                                printf("Sleep time:%d ms\r\n", g_ExpectedSleepTimeMs);
                                g_MapmOpState = STATE_SLEEP; /* 进入休眠状态 */
                            }
                            else /* Mapm的3个字节地址完全匹配，需要继续接收数据 */
                            {
                                printf("MAPM Addr match\r\n");
                                g_RfRxPkt.MapmRxIndex = 0;                        /* 重置MAPM RX索引 */
                                printf("MAPM Counter: %d\r\n", g_CurrMapmCfg.fn); /* 打印MAPM counter值 */

                                /* 根据当前的MAPM配置和剩余的filed个数计算预期的休眠时间 */
                                int32_t LeftMapmTimeMs = RF_GetLeftMapmTime(&g_CurrMapmCfg, g_OneSymbolTimeUs);

                                /* 减掉1个field的时间是为了提前1个field醒来, 保证提前1个field时间进入接收状态 */
                                LeftMapmTimeMs -= RF_GetMapmOneFieldTime(&g_CurrMapmCfg, g_OneSymbolTimeUs);

                                /* 如果剩余的MAPM时间小于10ms，则RF不需要休眠，继续待续等待接收数据。 */
                                if(LeftMapmTimeMs < 10) 
                                {
                                    printf("Left Mapm Time: %d ms, No need to Sleep! goon rx\r\n", LeftMapmTimeMs);
                                }
                                else
                                {
                                    printf("Left Mapm Time: %d ms, Should Sleep!\r\n", LeftMapmTimeMs);
                                    RF_EnterSleepState(); /* Enter sleep mode */
                                    g_ExpectedSleepTimeMs = LeftMapmTimeMs;
                                    g_MapmOpState = STATE_SLEEP; /* 进入休眠状态 */
                                }
                                g_MapmMatchFlag = true; /* 设置MAPM匹配标志位 */
                            }
                        }
                    }
                    if (IRQFlag & RF_IRQ_RX_DONE) /* 接收完成中断 */
                    {
                        g_RfRxPkt.Snr = RF_GetPktSnr();   /* 获取接收数据包的SNR值 */
                        g_RfRxPkt.Rssi = RF_GetPktRssi(); /* 获取接收数据包的RSSI值 */
                        
                        /* 获取接收数据和长度 */
                        g_RfRxPkt.RxLen = RF_GetRecvPayload((uint8_t *)g_RfRxPkt.RxBuf);

                        g_MapmMatchFlag = false;

                        printf("+Rx Len=%d, Count=%d\n", g_RfRxPkt.RxLen, ++g_RxCount);
                        printf("+RxHexData:\r\n");
                        print_hex((uint8_t *)g_RfRxPkt.RxBuf, g_RfRxPkt.RxLen); /* 以十六进制方式打印接收数据 */
                        printf("SNR:%ddB, RSSI:%ddBm \r\n", (int)g_RfRxPkt.Snr, (int)g_RfRxPkt.Rssi);
                        RF_ClrIRQFlag(RF_IRQ_RX_DONE); /* 清除接收完成中断标志位 */
                        IRQFlag &= ~RF_IRQ_RX_DONE;
                        printf("Enter sleep state.\r\n");
                        RF_EnterSleepState();          /* Enter sleep mode */
                        g_ExpectedSleepTimeMs = 1050;  /* 发送端将在1000ms后发送下一包带MAPM数据, 其中50ms是一个余量时间 */
                        g_MapmOpState = STATE_SLEEP;
                    }
                    if (IRQFlag & RF_IRQ_RX_TIMEOUT) /* 接收超时中断 */
                    {
                        RF_ClrIRQFlag(RF_IRQ_RX_TIMEOUT); /* 清除接收超时中断标志位 */
                        g_MapmMatchFlag = false;
                        IRQFlag &= ~RF_IRQ_RX_TIMEOUT;
                        RF_EnterSleepState(); /* Enter sleep mode */
                        g_ExpectedSleepTimeMs = RF_GetLeftMapmTime(&g_CurrMapmCfg, g_OneSymbolTimeUs) / 2;
                        g_MapmOpState = STATE_SLEEP;
                    }
                    if (IRQFlag)
                    {
                        RF_ClrIRQFlag(IRQFlag); /* 清除未处理的中断标志位 */
                    }
                }
                break;
            }
            case STATE_SLEEP:
            {
                if(SysTick_GetTick() - g_CurrentSysTickMs >= g_ExpectedSleepTimeMs) /* Sleep timeout */
                {
                    RF_ExitSleepState(); /* Exit sleep mode */
                    if(g_MapmMatchFlag)
                    {
                        RF_EnterSingleRxWithTimeout(3000); /* Enter RX mode with timeout */
                        g_RfRxPkt.MapmRxIndex = 0;         /* Reset MAPM RX index */
                        g_MapmOpState = STATE_RX;
                        printf("enter the STATE_RX.\r\n");
                    }
                    else
                    {
                        g_MapmOpState = STATE_IDLE; /* 先进入空闲状态，再进行CAD检测 */
                    }
                    printf("RF wakeup.\r\n");
                }
                break;
            }
            default:
                break;
        }
    }
}
