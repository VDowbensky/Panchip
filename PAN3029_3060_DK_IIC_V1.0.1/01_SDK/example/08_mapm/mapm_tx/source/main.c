/**
 *********************************************************************************************
 * @file    main.c
 * @brief   本示例演示了如何使用PAN3029/3060发送带MAPM的数据包的功能
 * @version V1.0.1
 * @date    2025-08-18
 * @author  Panchip Team
 * @note    该示例功能和流程如下：
 *          1. 初始化系统时钟、SysTick定时器、定时器、GPIO、UART和SPI。
 *          2. 调用RF_Init函数初始化PAN3029/3060模块。
 *          3. 配置RF模块的用户参数，包括频率、扩频因子(SF)、带宽(BW)、前导码(Preamble)、CRC等。
 *          4. 配置MAPM相关参数，包括地址、fn、fnm、gn、gfs、pg1、pgn和pn等。
 *          5. 将g_TxBuf缓冲区中的数据写入到RF模块中，并开始发送MAPM数据包。
 *          6. 在主循环中检测RF中断信号，并处理以下中断事件：
 *             6.1 发送完成中断：关闭内部PA，清除发送完成中断标志位，并打印发送次数。
 *             6.2 处理其他未处理的中断标志。
 *          7. 延时1秒后继续发送下一个MAPM数据包。
 * @note    该示例需要01_SDK/example/08_mapm/mapm_rx示例配合一起进行功能演示，功能演示需要两块开发板：
 *          1. 一块开发板作为接收端，使用01_SDK/example/08_mapm/mapm_rx示例代码接收带MAPM的数据包。
 *          2. 另一块开发板作为发送端，使用本示例代码发送带MAPM的数据包。
 * Copyright (C) 2025 Panchip Technology Corp. All rights reserved.
 *********************************************************************************************
 */
#include "bsp.h"
#include "pan_rf.h"

RF_MapmCfg_t g_CurrMapmCfg;

/* g_TxCount为PAN3029/3060 发送计数变量 */
unsigned int g_TxCount = 0;

/* g_TxBuf为PAN3029/3060 发送数据缓冲区，大小16字节，内容为0~15 */
unsigned char g_TxBuf[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

/**
 * @brief PAN3029/3060 MAPM配置参数表
 * @note 每个配置包含地址、fn、fnm、gfs、gn、pg1、pgn和pn等参数
 * @note 本例程中仅使用g_MapmCfg[0]作为发送端的MAPM配置作为演示，
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
    },
};

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

    g_CurrMapmCfg = g_MapmCfg[0]; /* 复制g_MapmCfg[0]配置到g_CurrMapmCfg中 */
    RF_SetMapmAddr((uint8_t *)g_CurrMapmCfg.Addr, sizeof(g_CurrMapmCfg.Addr));  /* 发端数据包中MAPM的地址 */
    RF_ConfigMapm((RF_MapmCfg_t *)&g_CurrMapmCfg);                              /* 设置MAPM相关参数 */
    RF_EnableMapm();                                                            /* 开启MAPM相关功能 */
    
    printf("RF mapm_tx test start.\r\n");

    while(1)
    {
        RF_TxSinglePkt(g_TxBuf, sizeof(g_TxBuf)); /* 发送数据包 */

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
                    RF_EnterStandbyState();        /* 发送完成后须设置为RF_STATE_STB3状态 */
                    printf("Tx Count:%d\r\n", ++g_TxCount); /* 打印发送次数 */
                    break;
                }
        
                if (IRQFlag)
                {
                    RF_ClrIRQFlag(IRQFlag); /* 清除未处理的中断标志位 */
                }
            }
        }

        SysTick_Delay(1000); /* 延时1秒 */
    }
}
