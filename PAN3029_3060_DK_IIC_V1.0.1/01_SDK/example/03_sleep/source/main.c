/**
 *************************************************************************
 * @file    main.c
 * @brief   本示例演示了如何使用PAN3029/3060交替进行数据发送和睡眠的功能。
 * @version V1.0.1
 * @date    2025-08-18
 * @author  Panchip Team
 * @note    该示例通过以下步骤实现数据发送和深度睡眠的功能：
 *          1. 初始化系统时钟、定时器、GPIO、UART、SPI等外设。
 *          2. 初始化RF模块并配置用户参数，如频率、扩频因子、带宽等。
 *          3. 进入主循环，发送数据包并等待发送完成中断。
 *          4. 在发送完成后，关闭内部功率放大器（PA）并进入深度睡眠状态。
 *          5. 睡眠500ms后重新初始化RF模块并配置用户参数。
 *          6. 重复步骤3-5，实现交替发送和深度睡眠的功能。
 * @note    该示例需要配合01_SDK/example/01_normal_trx/rx示例一起进行功能演示，功能演示需要两块开发板：
 *          - 一块开发板烧录01_SDK/example/01_normal_trx/rx示例，进行数据包接收。
 *          - 另一块开发板烧录本示例程序，进行数据发送、进入&退出睡眠的验证。
 * @note    PAN3029/3060在进入睡眠(sleep）后，RF模块的功耗会降低1uA以下，
 *          芯片内所有的寄存器和状态会被保存，退出睡眠后，RF模块会自动恢复到之前的状态。
 * Copyright (C) 2025 Panchip Technology Corp. All rights reserved.
 *************************************************************************
 */

#include "bsp.h"
#include "pan_rf.h"

/* g_TxCount为PAN3029/3060 发送计数变量 */
static unsigned int g_TxCount = 0;

/* g_TxBuf为PAN3029/3060 发送数据缓冲区，大小16字节，内容为0~15 */
static unsigned char g_TxBuf[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

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
    
    printf("Tx with sleep test start.\r\n");

    while(1)
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
                    RF_EnterStandbyState();                 /* 发送完成后须设置为RF_STATE_STB3状态 */
                    printf("Tx Count:%d\r\n", ++g_TxCount); /* 打印发送次数 */
                    break;                                  /* 退出RF中断处理循环 */
                }

                if (IRQFlag)
                {
                    RF_ClrIRQFlag(IRQFlag); /* 清除未处理的中断标志位 */
                }
            }
        }
        printf("Enter sleep status.\r\n");
        RF_EnterSleepState(); /* 进入深度睡眠状态 */
        SysTick_Delay(500);   /* 延时500ms */
        printf("Exit sleep status.\r\n");
        RF_ExitSleepState();  /* 退出深度睡眠状态 */
    }
}
