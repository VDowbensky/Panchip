/**
 *********************************************************************************************
 * @file    main.c
 * @brief   本示例演示了如何使用PAN3029/3060进行自适应SF接收的功能
 * @version V1.0.1
 * @date    2025-08-18
 * @author  Panchip Team
 * @note    该示例功能和流程如下：
 *          1. 初始化系统时钟、SysTick定时器、定时器、GPIO、UART和SPI。
 *          2. 调用RF_Init函数初始化PAN3029/3060模块。
 *          3. 配置RF模块的用户参数，包括频率、扩频因子(SF)、带宽(BW)、前导码(Preamble)、CRC等。
 *          4. 开启自适应SF接收功能，设置需要自适应的SF集合。
 *          5. 进入连续接收模式，等待接收数据。
 *          6. 在主循环中检测RF中断信号，并处理以下中断事件：
 *             6.1 接收完成中断：获取接收数据包的SNR、RSSI、识别到的SF和数据内容，并打印。
 *             6.2 CRC错误中断：清除CRC错误标志并打印错误信息。
 * @note    该示例需要配合01_SDK/example/06_auto_sf/rx示例一起进行功能演示，功能演示需要两块开发板：
 *          1. 一块开发板作为发送端，使用01_SDK/example/06_auto_sf/tx示例代码发送不同SF的数据包。
 *          2. 另一块开发板作为接收端，使用本示例代码进行自动识别SF接收数据包功能演示。
 * @note    注意：
 *          1. 自适应SF发送功能不能与CAD功能同时使用！
 *          2. 需要自适应的SF集合越多，发送端的发送前导码长度越长，发送时间越长。
 *          3. 接收端需要自适应的SF越多，搜索时间越长，搜索时间与SF数量成正比。
 * Copyright (C) 2025 Panchip Technology Corp. All rights reserved.
 *********************************************************************************************
 */

#include "bsp.h"
#include "pan_rf.h"

/**
 * @brief 结构体定义
 * @note  用于保存需要自搜索的SF和对应的发送前导码长度
 */
typedef struct
{
    RfSpreadFactor_t Sf; //!< Spreading factor
    uint16_t TxPreamLen; //!< Tx preamble length according to SF
} AutoSfPream_t;

#define AUTO_SF_NUM  5  //!< 共需要自搜索5个SF

/* 需要自搜索的SF集合 */
static AutoSfPream_t g_AutoSfPream[AUTO_SF_NUM] = 
{
    {RF_SF5, 8},
    {RF_SF6, 8},
    {RF_SF7, 8},
    {RF_SF8, 8},
    {RF_SF9, 8},
};

/* g_RxCount为PAN3029/3060 接收计数变量 */
unsigned int g_RxCount = 0;

/**
 * @brief 根据SF集合打开对应的SF搜索
 * @param pSfPream 指向AutoSfPream_t结构体数组的指针
 * @param SF_Num 需要搜索的SF数量
 * @note 注意：自搜索功能不能与CAD功能同时使用！
 */
static void RF_EnableAutoSF(AutoSfPream_t *pSfPream, int SF_Num)
{
    uint8_t i, SF_Mask = 0;

    /* Enable auto sf（将[Page3][0x12][BIT0]置1） */
    RF_SetPageRegBits(3, 0x12, 0x01);

    /**
     * SF搜索模式提前终止使能位([Page1][0x25][BIT2])：
     *  0：发现误触发不会立即跳到下一个SF，搜索速度较慢
     *  1：发现误触发立即跳到下一个SF，搜索速度较快
     */
    RF_WritePageRegBits(1, 0x25, 1, 0x04);

    /* 使用自动识别SF功能时，必须设置CAD检测的chirp数为RF_CAD_01_SYMBOL */
    RF_SetCadChirps(RF_CAD_01_SYMBOL);

    /**
     * SF搜索使能配置：
     *  - [Page1][0x2D][BIT0]为1时，表示使能SF5自动搜索; 为0时，表示禁用SF5自动搜索;
     *  - [Page1][0x2D][BIT1]为1时，表示使能SF6自动搜索; 为0时，表示禁用SF6自动搜索;
     *  - [Page1][0x2D][BIT2]为1时，表示使能SF7自动搜索; 为0时，表示禁用SF7自动搜索;
     *  - [Page1][0x2D][BIT3]为1时，表示使能SF8自动搜索; 为0时，表示禁用SF8自动搜索;
     *  - [Page1][0x2D][BIT4]为1时，表示使能SF9自动搜索; 为0时，表示禁用SF9自动搜索;
     *  - [Page1][0x2D][BIT5]为1时，表示使能SF10自动搜索; 为0时，表示禁用SF10自动搜索;
     *  - [Page1][0x2D][BIT6]为1时，表示使能SF11自动搜索; 为0时，表示禁用SF11自动搜索;
     *  - [Page1][0x2D][BIT7]为1时，表示使能SF12自动搜索; 为0时，表示禁用SF12自动搜索;
     */
    for (i = 0; i < SF_Num; i++) 
    {
        SF_Mask |= (1 << (pSfPream[i].Sf - RF_SF5));
    }

    RF_WritePageReg(1, 0x2D, SF_Mask); /* 使能对应的SF搜索使能位 */
}

/**
 * @brief 关闭SF接收自适应功能
 */
void RF_DisableAutoSF(void)
{
    /* Disable auto sf */
    RF_ResetPageRegBits(3, 0x12, 0x01);

    /* 设置前导码长度为8 */
    RF_SetPreamLen(8);
}

/**
 * @brief 获取接收数据包的SF配置
 * @return 返回当前接收数据包的SF值
 * @note SF值在[Page1][0x7C]寄存器的高4位
 */
uint8_t RF_GetPktSF(void)
{
    uint8_t sf = RF_ReadPageReg(1, 0x7C); /* 获取当前接收的SF */
    return (sf >> 4);                     /* SF值在[Page1][0x7C]寄存器的高4位 */
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int32_t main(void)
{
    int ret;

    BSP_ClockInit();        /* 初始化系统时钟 */
    BSP_SystickInit(1000u); /* 初始化SysTick定时器，1ms */
    BSP_TimerInit();        /* 初始化定时器，1us */
    BSP_GpioInit();         /* 初始化GPIO */
    BSP_RFBusInit();          /* 初始化SPI，默认速率为6.25MHz */
    BSP_UartInit();         /* 初始化UART，波特率115200bps，用于调试打印 */

    ret = RF_Init();        /* PAN3029/3060初始化 */
    if (ret != RF_OK)
    {
        printf("RF init fail\r\n");
        while(1);
    }

    printf("Auto sf rx test.\r\n");

    RF_ConfigUserParams();                       /* 配置 Frequency、SF、BW、Preamble、CRC等参数 */
    RF_EnableAutoSF(g_AutoSfPream, AUTO_SF_NUM); /* 根据SF集合打开对应的SF搜索 */
    RF_EnterContinousRxState();                  /* 进入连续接收状态 */
    
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

                printf("+RxLen=%d, Rx Count=%d, Rx SF:%d\n", g_RfRxPkt.RxLen, ++g_RxCount, RF_GetPktSF());
                printf("+RxHexData:\r\n");
                print_hex((uint8_t *)g_RfRxPkt.RxBuf, g_RfRxPkt.RxLen); /* 以十六进制方式打印接收数据 */
                printf("SNR:%ddB, RSSI:%ddBm \r\n", (int)g_RfRxPkt.Snr, (int)g_RfRxPkt.Rssi);
                RF_ClrIRQFlag(RF_IRQ_RX_DONE); /* 清除接收完成中断标志位 */
                IRQFlag &= ~RF_IRQ_RX_DONE;
            }
            if (IRQFlag & RF_IRQ_CRC_ERR) /* CRC错误中断 */
            {
                RF_ClrIRQFlag(RF_IRQ_CRC_ERR); /* 清除CRC错误中断标志位 */
                IRQFlag &= ~RF_IRQ_CRC_ERR;
                printf(">>RF_IRQ_CRC_ERR\r\n");
            }

            if (IRQFlag)
            {
                RF_ClrIRQFlag(IRQFlag); /* 清除未处理的中断标志位 */
            }
        }
    }
}
