/************************************************************************************************
 * @file    main.c
 * @brief   本示例演示了如何使用PAN3029/3060进行简单的ping pong测试
 * @version V1.0.1
 * @date    2025-08-18
 * @author  Panchip Team
 * @note    该示例功能和流程如下：
 *          1. 初始化系统时钟、SysTick定时器、定时器、GPIO、UART和SPI。
 *          2. 调用RF_Init函数初始化PAN3029/3060模块。
 *          3. 配置RF模块的用户参数，包括频率、扩频因子(SF)、带宽(BW)、前导码(Preamble)、CRC等。
 *          4. 进入连续接收模式，等待接收数据。
 *          5. 在主循环中检测RF中断信号，并处理以下中断事件：
 *            5.1 接收完成中断：获取接收数据包的SNR、RSSI和数据内容，并打印，
 *                将响应数据包写入到RF模块中，并开始发送数据包。
 *            5.2 发送完成中断：关闭内部功率放大器（PA），清除发送完成中断标志位，并打印发送次数，
 *                RF进入连续接收模式，继续接收数据。
 *            5.3 CRC错误中断：清除CRC错误标志并打印错误信息。
 *            5.4 接收超时中断：清除超时标志并打印超时信息。
 *            5.5 处理其他未处理的中断标志。
 * @note    该示例需要配合01_SDK/example/02_pingpong/ping示例一起进行功能演示，功能演示需要两块开发板：
 *          1. 一块开发板作为接收端，使用本示例代码进行接收数据和发送数据。
 *          2. 另一块开发板作为发送端，使用01_SDK/example/02_pingpong/ping示例代码进行发送数据和接收数据。
 * Copyright (C) 2025 Panchip Technology Corp. All rights reserved.
 *************************************************************************************************
 */
#include "bsp.h"
#include "pan_rf.h"

/* g_TxCount为PAN3029/3060 发送计数变量 */
unsigned int g_TxCount = 0;

/* g_RxCount为PAN3029/3060 接收计数变量 */
unsigned int g_RxCount = 0;

/* g_TxBuf为PAN3029/3060 发送数据缓冲区，大小16字节，内容为0~15 */
unsigned char g_TxBuf[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

int32_t main(void)
{
    int ret;

    BSP_ClockInit();        /* 初始化系统时钟 */
    BSP_SystickInit(1000u); /* 初始化SysTick定时器，1ms */
    BSP_GpioInit();         /* 初始化GPIO */
    BSP_UartInit();         /* 初始化UART */
    BSP_RFBusInit();        /* 初始化SPI/I2C */

    ret = RF_Init(); /* PAN3029/3060初始化 */
    if (ret != RF_OK)
    {
        printf("RF init fail\r\n");
        while (1);
    }

    RF_ConfigUserParams();      /* 配置 Frequency、SF、BW、Preamble、CRC等参数 */
    RF_EnterContinousRxState(); /* 进入连续接收模式 */

    printf("RF ping pong test.\r\n");

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
                
                printf(">>RF_IRQ_TX_DONE!\r\n");
                printf("The response packet has been sent!\r\nTx Count:%d\r\n", ++g_TxCount); /* 打印发送次数 */
                
                //!< 须在清除发送中断标志位后，再进入连续接收模式
                RF_EnterContinousRxState(); /* 发送完数据包响应后，立即进入连续接收模式 */
            }
            if (IRQFlag & RF_IRQ_RX_DONE) /* 接收完成中断 */
            {
                g_RfRxPkt.Snr = RF_GetPktSnr();   /* 获取接收数据包的SNR值 */
                g_RfRxPkt.Rssi = RF_GetPktRssi(); /* 获取接收数据包的RSSI值 */
                
                /* 获取接收数据和长度 */
                g_RfRxPkt.RxLen = RF_GetRecvPayload((uint8_t *)g_RfRxPkt.RxBuf);
                printf("<=======================================================>\r\n");
                printf(">>RF_IRQ_RX_DONE!\r\n");
                printf("+RxLen=%d, Rx Count=%d\n", g_RfRxPkt.RxLen, ++g_RxCount);
                printf("+RxHexData:\r\n");
                print_hex((uint8_t *)g_RfRxPkt.RxBuf, g_RfRxPkt.RxLen); /* 以十六进制方式打印接收数据 */
                printf("SNR:%ddB, RSSI:%ddBm \r\n", (int)g_RfRxPkt.Snr, (int)g_RfRxPkt.Rssi);
                RF_ClrIRQFlag(RF_IRQ_RX_DONE); /* 清除接收完成中断标志位 */
                IRQFlag &= ~RF_IRQ_RX_DONE;
                
                //!< 须在清除接收中断标志位后，再发送数据包
                printf("Start to send response packet.\r\n");
                memcpy(g_TxBuf, (void *)g_RfRxPkt.RxBuf, sizeof(g_TxBuf)); /* 将收到的数据包内容复制到发送数据包缓冲区 */
                RF_TxSinglePkt(g_TxBuf, sizeof(g_TxBuf)); /* 收到一包数据后，立即发送一包回复数据包 */
            }
            if (IRQFlag & RF_IRQ_CRC_ERR) /* CRC错误中断 */
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
}
