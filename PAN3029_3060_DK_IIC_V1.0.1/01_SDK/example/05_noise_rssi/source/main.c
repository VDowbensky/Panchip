/************************************************************************************************
 * @file    main.c
 * @brief   本示例演示了如何使用PAN3029/3060获取空中实时RSSI值的功能
 * @version V1.0.1
 * @date    2025-08-18
 * @author  Panchip Team
 * @note    该示例功能和流程如下：
 *          1. 初始化系统时钟、SysTick定时器、定时器、GPIO、UART和SPI。
 *          2. 调用RF_Init函数初始化PAN3029/3060模块。
 *          3. 配置RF模块的用户参数，包括频率、扩频因子(SF)、带宽(BW)、前导码(Preamble)、CRC等。
 *          4. 进入连续接收状态，等待接收数据。
 *          5. 在主循环中每200ms打印一次噪声RSSI值。
 *          6. 在主循环中同时检测RF中断信号，并处理以下中断事件：
 *             6.1 接收完成中断：获取接收数据包的SNR、RSSI和数据内容，并打印。
 *             6.2 清除接收完成中断标志位。
 *          7. 主循环持续运行，等待接收更多数据。
 * @note    该示例需要01_SDK/example/01_normal_trx/tx示例配合一起进行功能演示，功能演示需要两块开发板：
 *          1. 一块开发板作为发送端，使用01_SDK/example/01_normal_trx/tx示例代码进行发送数据。
 *          2. 另一块开发板作为接收端，使用本示例代码进行接收数据及获取实时RSSI值。
 * @note   该示例使用了PAN3029/3060的RF_GetRealTimeRssi函数来获取空中实时RSSI值。
 * Copyright (C) 2025 Panchip Technology Corp. All rights reserved.
 * ************************************************************************************************
 */
#include "bsp.h"
#include "pan_rf.h"

/* g_RxCount为PAN3029/3060 接收计数变量 */
unsigned int g_RxCount = 0;

int main(void)
{
    int ret;
    uint32_t LastSystemTickMs;

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

    RF_ConfigUserParams();      /* 配置 Frequency、SF、BW、Preamble、CRC等参数 */
    RF_EnterContinousRxState(); /* 进入连续接收状态 */

    printf("Real time rssi test.\r\n");
    
    LastSystemTickMs = SysTick_GetTick();

    while (1)
    {
        /* 每200ms打印一次噪声RSSI值 */
        if (SysTick_GetTick() - LastSystemTickMs > 200)
        {
            LastSystemTickMs = SysTick_GetTick(); /* 更新上次打印时间 */

            printf("Noise rssi:%ddBm\r\n", RF_GetRealTimeRssi()); /* 从函数RF_GetRealTimeRssi()中获取空中实时RSSI */
        }
        
        if (CHECK_RF_IRQ()) /* 检测到RF中断，高电平表示有中断 */
        {
            uint8_t IRQFlag;
            IRQFlag = RF_GetIRQFlag(); /* 获取中断标志位 */
            if (IRQFlag & RF_IRQ_RX_DONE) /* 接收完成中断 */
            {
                g_RfRxPkt.Snr = RF_GetPktSnr();   /* 获取接收数据包的SNR值 */
                g_RfRxPkt.Rssi = RF_GetPktRssi(); /* 获取接收数据包的RSSI值 */
                
                /* 获取接收数据和长度 */
                g_RfRxPkt.RxLen = RF_GetRecvPayload((uint8_t *)g_RfRxPkt.RxBuf);

                printf("+Rx Len=%d, Count=%d\n", g_RfRxPkt.RxLen, ++g_RxCount);
                printf("+RxHexData:\r\n");
                print_hex((uint8_t *)g_RfRxPkt.RxBuf, g_RfRxPkt.RxLen); /* 以十六进制方式打印接收数据 */
                printf("SNR:%ddB, RSSI:%ddBm \r\n", (int)g_RfRxPkt.Snr, (int)g_RfRxPkt.Rssi);
                RF_ClrIRQFlag(RF_IRQ_RX_DONE); /* 清除接收完成中断标志位 */
                IRQFlag &= ~RF_IRQ_RX_DONE;
            }

            if (IRQFlag)
            {
                RF_ClrIRQFlag(IRQFlag); /* 清除未处理的中断标志位 */
            }
        }
    }
}
