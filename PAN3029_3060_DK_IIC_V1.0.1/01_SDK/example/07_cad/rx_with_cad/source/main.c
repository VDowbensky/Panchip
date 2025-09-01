/**
 ************************************************************************
 * @file    main.c
 * @brief   本示例演示了如何使用PAN3029/3060进行CAD检测和接收数据包。
 * @version V1.0.1
 * @date    2025-08-18
 * @author  Panchip Team
 * @note    该示例的功能和流程如下：
 *          1. 初始化系统时钟、定时器、GPIO、UART和SPI等硬件资源。
 *          2. 初始化PAN3029/3060模块，并配置用户参数（如频率、SF、BW、Preamble、CRC等）。
 *          3. 使用IsChannelBusy函数检测信道状态：
 *             - 如果信道忙（检测到有效chirp符号），进入接收模式，接收数据包并打印接收信息。
 *             - 如果信道空闲（未检测到有效chirp符号），进入睡眠模式，降低功耗。
 *          4. 在接收模式下，处理接收完成中断和超时中断，打印接收到的数据包内容及相关信息。
 *          5. 在睡眠模式下，定时唤醒并重新检测信道状态，循环执行上述流程。
 * @note    该示例需要配合01_SDK/example/07_cad/tx_with_lbt示例一起进行功能演示，功能演示需要两块开发板：
 *          - 一块开发板烧录01_SDK/example/07_cad/tx_with_lbt示例，进行CAD检测和发送数据包的验证。
 *          - 另一块开发板烧录本示例程序，进行CAD检测和接收数据包的验证。
 * Copyright (C) 2025 Panchip Technology Corp. All rights reserved.
 ****************************************************************************
 */
#include "bsp.h"
#include "pan_rf.h"

/* g_g_RxCount为PAN3029/3060 接收计数变量 */
unsigned int g_RxCount = 0;

/**
 * @brief CAD检测
 * @return 信道状态
 *        - false: 信道空闲（没有检测到有效chirp符号）
 *        - true: 信道忙（检测到有效chirp符号）
 * @note 该函数用于检测信道是否空闲，间接指示当前空中是否有数据包
 * @note 该函数会使用RF_StartCad()函数启动CAD检测，并使用RF_DelayUs()函数等待CAD检测完成
 * @note 该函数要求RF_DelayUs()函数延时尽可能精确
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

    RF_StartCad(RF_CAD_THRESHOLD_20, RF_CAD_03_SYMBOL); /* Start CAD detection */

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

    /* 在CAD检测完成后，再打印相关信息，防止打印影响到CAD检测时间准确性 */
    /* 打印会影响CAD检测时间准确性，建议仅在调试时打印 */
    // printf("BW: %d, SF: %d\r\n", bw, sf);
    // printf("OneSymbolTime: %d us\r\n", OneSymbolTime); /* 打印单个chirp时间 */
    // printf("CAD detection time: %d us\r\n", CadTime);  /* 打印CAD检测时间 */

    return channelBusy;
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

    uint32_t g_OneSymbolTimeUs = RF_GetOneSymbolTime(RF_BW_DEFAULT, RF_SF_DEFAULT); // 每个symbol的时间(us)

    while (1)
    {
        /**
         * 用IsChannelBusy函数检测信道是否空闲
         * - 如果信道忙，表示空中有数据包，则进入接收模式
         * - 如果信道空闲，表示空中没有数据包，则进入睡眠模式
         */
        if (IsChannelBusy())
        {
            // printf("Symbol is detected, enter rx mode.\r\n");  /* 打印会影响CAD检测时间准确性，建议仅在调试时打印 */

            /* 进入接收模式，设置接收超时时间为3000ms，
               这里的3000ms是一个示例值，可以根据实际需求调整 */
            RF_EnterSingleRxWithTimeout(3000);

            while (1)
            {
                if (CHECK_RF_IRQ()) /* 检测到RF中断，高电平表示有中断 */
                {
                    uint8_t IRQFlag;
                    IRQFlag = RF_GetIRQFlag();    /* 获取中断标志位 */
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
                    if (IRQFlag & RF_IRQ_RX_TIMEOUT) /* CRC错误中断 */
                    {
                        RF_ClrIRQFlag(RF_IRQ_RX_TIMEOUT); /* 清除CRC错误中断标志位 */
                        IRQFlag &= ~RF_IRQ_RX_TIMEOUT;
                        printf(">>RF_IRQ_RX_TIMEOUT\r\n");
                    }

                    if (IRQFlag)
                    {
                        RF_ClrIRQFlag(IRQFlag); /* 清除未处理的中断标志位 */
                    }

                    break; /* 退出接收循环 */
                }
            }
        }
        else
        {
            printf("Channel is free, enter sleep mode.\r\n");
            RF_EnterSleepState(); /* 进入睡眠模式 */
            RF_DelayMs(300);      /* 休眠300ms时间，这里的300ms是一个示例值，可以根据实际需求调整 */
            RF_ExitSleepState();  /* 退出睡眠模式 */
            printf("Exit sleep mode.\r\n");
        }
    }
}
