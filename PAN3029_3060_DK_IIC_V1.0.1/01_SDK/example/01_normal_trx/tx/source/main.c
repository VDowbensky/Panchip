/**
 *********************************************************************************************
 * @file    main.c
 * @brief   本示例演示了如何使用PAN3029/3060进行简单的发送功能测试
 * @version V1.0.1
 * @date    2025-08-18
 * @author  Panchip Team
 * @note    该示例功能和流程如下：
 *          1. 初始化系统时钟、SysTick定时器、定时器、GPIO、UART和SPI。
 *          2. 调用RF_Init函数初始化PAN3029/3060模块。
 *          3. 配置RF模块的用户参数，包括频率、扩频因子(SF)、带宽(BW)、前导码(Preamble)、CRC等。
 *          4. 将g_TxBuf缓冲区中的数据写入到RF模块中，并开始发送数据包。
 *          5. 在主循环中检测RF中断信号，并处理以下中断事件：
 *             5.1 发送完成中断：关闭内部PA，清除发送完成中断标志位，并打印发送次数。
 *             5.2 处理其他未处理的中断标志。
 *          6. 延时1秒后继续发送下一个数据包。
 *          7. 主循环持续运行，直到断电或复位。
 * @note    该示例需要01_SDK/example/01_normal_trx/rx示例配合一起进行功能演示，功能演示需要两块开发板：
 *          1. 一块开发板作为接收端，使用01_SDK/example/01_normal_trx/rx示例代码进行接收数据。
 *          2. 另一块开发板作为发送端，使用本示例代码进行发送数据。
 * Copyright (C) 2025 Panchip Technology Corp. All rights reserved.
 *********************************************************************************************
 */
#include "bsp.h"
#include "pan_rf.h"

/* g_TxCount为PAN3029/3060 发送计数变量 */
unsigned int g_TxCount = 0;

/* g_TxBuf为PAN3029/3060 发送数据缓冲区，大小16字节，内容为0~15 */
unsigned char g_TxBuf[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

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

    printf("RF tx test start.\r\n");

    RF_ConfigUserParams(); /* 配置 Frequency、SF、BW、Preamble、CRC等参数 */

    while (1)
    {
        memset(g_TxBuf, g_TxCount, sizeof(g_TxBuf));
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
                    RF_EnterStandbyState(); /* 发送完成后须设置为RF_STATE_STB3状态 */

                    BSP_LedToggle();                                 /* 发送完一次数据包后翻转LED指示灯 */
                    printf("Tx done, tx count:%d\r\n", ++g_TxCount); /* 打印发送次数 */
                    break;
                }

                if (IRQFlag)
                {
                    RF_ClrIRQFlag(IRQFlag); /* 清除未处理的中断标志位 */
                }
            }
        }

        SysTick_Delay(500); /* 延时1秒 */
    }
}
