/**
 ****************************************************************************************
 * @file    main.c
 * @brief   本示例演示了如何使用PAN3029/3060进行Listen Before Talk (LBT)功能。
 * @version V1.0.1
 * @date    2025-08-18
 * @author  Panchip Team
 * @note    该示例展示了如何使用PAN3029/3060模块实现带有Listen Before Talk (LBT)功能的发送流程。
 *          主要功能包括：
 *          1. 初始化系统时钟、GPIO、UART、SPI等外设。
 *          2. 配置PAN3029/3060的工作参数，如频率、扩频因子(SF)、带宽(BW)、前导码长度等。
 *          3. 在发送数据前进行CAD检测，判断信道是否空闲。
 *          4. 如果信道忙，则触发随机退避机制，等待一段时间后重试。
 *          5. 如果信道空闲，则发送数据并等待发送完成中断。
 *          6. 支持最大重试次数限制，超过限制后发送失败。
 *          7. 通过串口打印调试信息，包括信道状态、退避时间、发送结果等。
 * @note    该示例仅供参考，具体实现可能需要根据实际应用场景进行调整。
 * @note    该示例需要配合01_SDK/example/07_cad/rx_with_cad示例或01_SDK/example/01_normal_trx/rx示例一起进行功能演示，
 *          该功能演示需要三块开发板：
 *          - 其中两块开发板烧录01_SDK/example/07_cad/tx_with_lbt示例，进行LBT功能验证。
 *          - 另外一块开发板烧录01_SDK/example/07_cad/rx_with_cad或01_SDK/example/01_normal_trx/rx示例，进行数据包接收。
 * Copyright (C) 2025 Panchip Technology Corp. All rights reserved.
 ********************************************************************************************
 */
#include <stdlib.h>
#include "bsp.h"
#include "pan_rf.h"

#define MAX_RETRIES         5     //!< LBT最大重试次数，根据实际应用场景可调整
#define BACKOFF_MIN_MS      1000  //!< LBT最小退避时间（1秒），根据实际应用场景可调整
#define BACKOFF_MAX_MS      3000  //!< LBT最大退避时间（3秒），根据实际应用场景可调整

/* g_TxCount为PAN3029/3060 发送计数变量 */
unsigned int g_TxCount = 0;

/* g_TxBuf为PAN3029/3060 发送数据缓冲区，大小16字节，内容为0~15 */
unsigned char g_TxBuf[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

/**
 * @brief CAD检测
 * @return 信道状态
 *        - false: 信道空闲
 *        - true: 信道忙
 */
static bool IsChannelBusy(void)
{
    uint8_t bw, sf;
    uint32_t OneSymbolTime;
    uint32_t CadTime;
    bool channelBusy = false;

    sf = RF_GetSF();
    bw = RF_GetBandWidth();

    OneSymbolTime = RF_GetOneSymbolTime(bw, sf); /* Get one symbol time, unit: us */

    RF_StartCad(RF_CAD_THRESHOLD_20, RF_CAD_03_SYMBOL); /* 进入接收状态，并开始CAD检测 */

    /**
     * @brief 这里需要注意的是，CAD检测时间是根据单个chirp时间来计算
     * @note 使用RF_CAD_01_SYMBOL时, CAD检测时间可能会大于1个符号时间，实际可采用2个符号时间
     * @note 使用RF_CAD_02_SYMBOL时, CAD检测时间可能会大于2个符号时间，实际可采用3个符号时间
     * @note 使用RF_CAD_03_SYMBOL时, CAD检测时间可能会大于3个符号时间，实际可采用4个符号时间
     * @note 使用RF_CAD_04_SYMBOL时, CAD检测时间可能会大于4个符号时间，实际可采用5个符号时间
     * @note 下面代码中的RF_CAD_03_SYMBOL + 1表示多加一个符号时间的余量来保证CAD检测的可靠性
     * @note 下面代码中360us时间对应的是PAN3029/3060的从STB3到接收状态的建立时间
     */
    CadTime = OneSymbolTime * (RF_CAD_03_SYMBOL + 1) + 360; /* unit:us */
    RF_DelayUs(CadTime);/* 延时一会等待CAD检测完成，这里采用延时方式，也可以采用中断方式来处理CAD检测完成事件 */

    /* 如果PAN3029/3060的GPIO11引脚为高电平，则表示空中有ChirpIOT信号 */
    if(CHECK_RF_CAD()) 
    {
        channelBusy = true; /* channel is busy */
    }
    else
    {
        channelBusy = false; /* channel is free */
    }
    RF_StopCad(); /* Stop CAD detection */

    return channelBusy;
}

/**
 * @brief 带LBT的发送函数
 * @param Buffer 要发送的数据缓冲区
 * @param Len 要发送的数据字节数
 * @return 发送结果
 *       - true: 发送成功
 *       - false: 发送失败
 * @note 该函数会在发送前进行CAD检测，如果信道忙则会进行退避重试
 */
static bool sendPacketWithLBT(uint8_t *Buffer, size_t Len)
{
    int Retry = 0;
    while (Retry < MAX_RETRIES)
    {
        if (!IsChannelBusy())
        {
            printf("channel is idle.\r\n");
            /* 信道空闲，发送数据 */
            RF_TxSinglePkt(Buffer, Len);
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
                        break;                  /* 退出RF中断处理循环 */
                    }

                    if (IRQFlag)
                    {
                        RF_ClrIRQFlag(IRQFlag); /* 清除未处理的中断标志位 */
                    }
                }
            }
            return true;
        }
        else
        {
            /* 信道忙，触发退避 */
            int backoffTime = rand();  /* 生成随机数 */
            backoffTime = (backoffTime % (BACKOFF_MAX_MS - BACKOFF_MIN_MS)) + BACKOFF_MIN_MS; /* 随机退避时间 */
            printf("channel is busy, wait and try...\r\n");
            printf("backoffTime: %d ms\r\n", backoffTime);
            RF_DelayMs(backoffTime);   /* 等待退避时间 */
            Retry++;
        }
    }
    printf("Exceed max retries, send failed\r\n");
    return false;
}

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
    RF_SetPreamLen(1500);  /* 设置前导码长度为1500个符号 */
    srand(0x31415);        /* 初始化随机数种子 */

    printf("Tx packet with LBT test start.\r\n");

    while(1)
    {
        memset(g_TxBuf, g_TxCount++, sizeof(g_TxBuf)); /* 发送数据缓冲区填充数据 */

        if (sendPacketWithLBT(g_TxBuf, sizeof(g_TxBuf)))
        {
            printf("Send packet success!\r\n");
        }
        else
        {
            printf("Send packet fail!\r\n");
        }

        RF_DelayMs(300 + (rand()%10)); /* 发送间隔300ms加上0-9ms之间随机时间 */
    }
}
