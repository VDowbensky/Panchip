/*******************************************************************************
 * @note Copyright (C) 2011-2022， Shanghai Panchip Microelectronics Co., Ltd. 
 * @SPDX-License-Identifier: Apache-2.0
 * @history - V0.1, 2022-09-28
*******************************************************************************/
#include "spi.h"
#include "hardware.h"
#include "timer3.h"
#include <stdio.h>
#include "service_pt.h"
#include "bt.h"
#include "trng.h"
#include "dmac.h"
#include "service_net.h"
#include "ds18b20.h"
#include "hal_pwm.h"

uint8_t uart_rx_buf[RX_FIFO_SIZE];
Fifo_t uart_rx_fifo = {0, 0,  RX_FIFO_SIZE,  0, (uint8_t *)uart_rx_buf};

uint8_t uart_tx_buf[TX_FIFO_SIZE];
Fifo_t uart_tx_fifo = {0, 0,  TX_FIFO_SIZE,  0, (uint8_t *)uart_tx_buf};

uint32_t uart_baudrate = 115200;    //uart baudrate

/*
 * After the node receives the lastbyte of package, 
 * it will wait for a short period (2~3 bytes). 
 * uart_packing_interval is the short period.
 * * □□□<package>□□□<package>□□□
 */
uint32_t uart_packing_interval = 5;
uint32_t uart_idle_timer_cnt = 59536;

/* aux pin mode
 * aux_mode=0
 *   AUX=0: fifo is empty
 *   AUX=1: fifo is not empty
 *  aux_mode=1
 *   AUX=0: fifo is not full
 *   AUX=1: fifo is full
 */
int aux_mode = AUX_MODE_EMPTY;

bool uart_rx_busy_flag = FALSE;
bool uart_tx_busy_flag = FALSE;

#ifdef DDL_UART_DMA_YES
volatile bool uart_dmac_tx_flag = FALSE;
#endif

volatile aux_level_t aux_level = {LEVEL_INACTIVE, LEVEL_INACTIVE, LEVEL_INACTIVE, LEVEL_INACTIVE};

extern void nm_set_io_op(void);
extern void pt_uart_set_rx_evt(void);
extern void net_uart_set_evt(void);
extern void pt_io_set_evt(void);

bool aux_level_set(aux_type_t type, bool level)
{
#if 0
    switch ( type )
    {
        case AUX_INIT :
            aux_level.init = level;
            break;
        case AUX_JOIN :
            aux_level.join = level;
            break;
        case AUX_DATA :
            aux_level.data = level;
            break;
        case AUX_FIFO :
            aux_level.fifo = level;
            break;
        case AUX_SLEEP :
            aux_level.sleep = level;
            break;
        case AUX_ERROR :
            aux_level.error = level;
            break;
        default:
            break;
    }

    if ( level == LEVEL_ACTIVE )
    {
        GPIO_AUX_LEVEL_SET_H;
    }
    else
    {
        if ( aux_level.init == LEVEL_INACTIVE
             && aux_level.join == LEVEL_INACTIVE
             && aux_level.data == LEVEL_INACTIVE
             && aux_level.fifo == LEVEL_INACTIVE
             && aux_level.sleep == LEVEL_INACTIVE
             && aux_level.error == LEVEL_INACTIVE)
        {
            GPIO_AUX_LEVEL_SET_L;
        }
        else
        {
            GPIO_AUX_LEVEL_SET_H;
        }
    }
#endif   
    return GPIO_AUX_LEVEL_GET();

}

void hw_clk_div_init(void)
{
    //Clock division settings
    Sysctrl_SetHCLKDiv(SysctrlHclkDiv1);
    Sysctrl_SetPCLKDiv(SysctrlPclkDiv1);
}

void app_clk_init(void)
{
    stc_sysctrl_clk_cfg_t stcCfg;
    
    ///< Enable the FLASH peripheral clock
    Sysctrl_SetPeripheralGate(SysctrlPeripheralFlash, TRUE);
    
    ///< Because the clock source HCLK to be used is less than 24M: Set the FLASH read wait cycle to 0 cycles (the default value is also 0 cycles)
    Flash_WaitCycle(FlashWaitCycle0);
    
    ///< Before clock initialization, prioritize setting the clock source to be used.
    Sysctrl_SetRCHTrim(SysctrlRchFreq24MHz);
    
    ///< Select internal RCH as HCLK clock source;
    stcCfg.enClkSrc    = SysctrlClkRCH;
    ///< HCLK SYSCLK/1
    stcCfg.enHClkDiv   = SysctrlHclkDiv1;
    ///< PCLK is HCLK/1
    stcCfg.enPClkDiv   = SysctrlPclkDiv1;
    ///< System clock initialization
    Sysctrl_ClkInit(&stcCfg);
}

uint8_t hw_pll_init(void)
{
    app_clk_init();
    hw_clk_div_init();
    
    return TRUE;
}

void Rtc_IRQHandler(void)
{
    if(Rtc_GetPridItStatus() == TRUE)
    {
        Rtc_ClearPrdfItStatus();
    }
}


///< PortA interrupt service function
void PortA_IRQHandler(void)
{
    if(Gpio_GetIrqStatus(GPIO_PORT_IRQ, STK_IRQ_PIN))//A2->3029IRQ
    {
        // rf_irq_handler();  
        Gpio_ClearIrq(GPIO_PORT_IRQ, STK_IRQ_PIN);
    }

    if(Gpio_GetIrqStatus(GPIO_PORT_CAD, GPIO_PIN_CAD))//A15->CAD
    {
        Gpio_ClearIrq(GPIO_PORT_CAD, GPIO_PIN_CAD);
    }
}

///< PortB interrupt service function
void PortB_IRQHandler(void)
{
    if ( Gpio_GetIrqStatus(GPIO_PORT_KEY, GPIO_PIN_KEY) )
    {
        Gpio_ClearIrq(GPIO_PORT_KEY, GPIO_PIN_KEY);
    }

    if(Gpio_GetIrqStatus(GPIO_PORT_IRQ, GPIO_PIN_IRQ))
    {
        // rf_irq_handler();  
        Gpio_ClearIrq(GPIO_PORT_IRQ, GPIO_PIN_IRQ);
    }

    if(Gpio_GetIrqStatus(GPIO_PORT_CAD, GPIO_PIN_CAD))
    {
        extern chirplan_proc_t chirplan_proc;
        chirplan_proc.cad_tx_timeout_flag |= MAC_EVT_TX_CAD_ACTIVE;
        stimer_stop(&chirplan_proc.stimer_chirplan_cad_event);
        Gpio_ClearIrq(GPIO_PORT_CAD, GPIO_PIN_CAD);
    }
}
void hw_rtc_cfg(int on, uint8_t second)
{
    if ( on )
    {
        stc_rtc_initstruct_t RtcInitStruct;
        
        Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);
        Sysctrl_SetPeripheralGate(SysctrlPeripheralRtc, TRUE);
        
        DDL_ZERO_STRUCT(RtcInitStruct);
        
        RtcInitStruct.rtcClksrc = RtcClkRcl;
        RtcInitStruct.rtcPrdsel.rtcPrdsel = RtcPrdx; 
        RtcInitStruct.rtcPrdsel.rtcPrdx = (uint8_t)(second*2)-1; //time step=0.5s
        Rtc_Init(&RtcInitStruct);
 
        Rtc_ClearPrdfItStatus();

        Rtc_AlmIeCmd(TRUE);                                     //IRQ               
        Rtc_Cmd(TRUE);                                          //Enable
        Rtc_StartWait();                                        //Wait
        EnableNvic(RTC_IRQn, IrqLevel3, TRUE); 
    }
    else
    {
        Rtc_DeInit();
        EnableNvic(RTC_IRQn, IrqLevel3, FALSE);
        Rtc_ClearPrdfItStatus();
    }
}

void hw_gpio_cfg(void)
{
    /* PB3->SETA
       PB4->SETB
       PB5->AUX  */
    stc_gpio_cfg_t stcGpioCfg;

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);

    stcGpioCfg.enDrv = GpioDrvH;
    stcGpioCfg.enDir = GpioDirOut;
    stcGpioCfg.enPu  = GpioPuDisable;
    stcGpioCfg.enPd  = GpioPdDisable;
    stcGpioCfg.enOD  = GpioOdDisable;
    stcGpioCfg.enCtrlMode = GpioAHB;
    Gpio_Init(GPIO_PORT_AUX, GPIO_PIN_AUX, &stcGpioCfg);   //AUX

    Gpio_Init(GPIO_PORT_LED, GPIO_PIN_LED, &stcGpioCfg);
    Gpio_WriteOutputIO(GPIO_PORT_LED, GPIO_PIN_LED, 0);

    /* AUX level is high, inditate node is initing */
    aux_level_set(AUX_INIT, LEVEL_ACTIVE);

    stcGpioCfg.enDir  = GpioDirOut;
    Gpio_Init(GpioPortA, GpioPin8, &stcGpioCfg);	//18B20
    Gpio_WriteOutputIO(GpioPortA, GpioPin8, FALSE); 

    stcGpioCfg.enDir = GpioDirIn;
    stcGpioCfg.enDrv = GpioDrvL;
    stcGpioCfg.enPu = GpioPuDisable;
    stcGpioCfg.enPd = GpioPdEnable;
    stcGpioCfg.enOD = GpioOdDisable;
    stcGpioCfg.enCtrlMode = GpioAHB;
    Gpio_Init(GPIO_PORT_IRQ, GPIO_PIN_IRQ, &stcGpioCfg);//pan3029 irq
    Gpio_Init(GPIO_PORT_CAD, GPIO_PIN_CAD, &stcGpioCfg);//cad check

    Gpio_EnableIrq(GPIO_PORT_IRQ, GPIO_PIN_IRQ, GpioIrqRising);
    Gpio_EnableIrq(GPIO_PORT_CAD, GPIO_PIN_CAD, GpioIrqRising);

    stcGpioCfg.enPu = GpioPuEnable;
    stcGpioCfg.enPd = GpioPdDisable;
    Gpio_Init(GPIO_PORT_KEY, GPIO_PIN_KEY, &stcGpioCfg);
    Gpio_EnableIrq(GPIO_PORT_KEY, GPIO_PIN_KEY, GpioIrqFalling);

    EnableNvic(PORTA_IRQn, IrqLevel3, TRUE);
    EnableNvic(PORTB_IRQn, IrqLevel3, TRUE);
}

/**
 ******************************************************************************
 ** \brief  Initialize external GPIO pins
 **
 ** \return None
 ******************************************************************************/
void hw_spi_gpio_init(void)
{
    stc_gpio_cfg_t stcGpioCfg;

    //SPI0 pin configuration: host 
    stcGpioCfg.enDrv = GpioDrvH;
    stcGpioCfg.enDir = GpioDirOut;
    stcGpioCfg.enPu  = GpioPuDisable;
    stcGpioCfg.enPd  = GpioPdDisable;
    stcGpioCfg.enOD  = GpioOdDisable;
    stcGpioCfg.enCtrlMode = GpioAHB;

    Spi_SetCS(M0P_SPI0, TRUE);
    Gpio_Init(GpioPortA, GpioPin4, &stcGpioCfg);
    Gpio_SetAfMode(GpioPortA, GpioPin4, GpioAf1);  //Configure pin PA04 as SPI0_CS
    Spi_SetCS(M0P_SPI0, TRUE);

    Gpio_Init(GpioPortA, GpioPin7, &stcGpioCfg);
    Gpio_SetAfMode(GpioPortA, GpioPin7, GpioAf1);  //Configure pin PA7 as SPI0_MOSI
    
    Gpio_Init(GpioPortA, GpioPin5, &stcGpioCfg);
    Gpio_SetAfMode(GpioPortA, GpioPin5, GpioAf1);  //Configure pin PA05 as SPI0_SCK

    stcGpioCfg.enDir = GpioDirIn;
    Gpio_Init(GpioPortA, GpioPin6, &stcGpioCfg);
    Gpio_SetAfMode(GpioPortA, GpioPin6, GpioAf1);  //Configure pin PA06 as SPI0_MISO
}

/**
 ******************************************************************************
 ** \brief  Initialize SPI
 **
 ** \return None
 ******************************************************************************/
void hw_spi_cfg(void)
{
    stc_spi_cfg_t SpiInitStruct;

    Sysctrl_SetPeripheralGate(SysctrlPeripheralSpi0,TRUE);

    // SPI0模块配置：主机
    SpiInitStruct.enSpiMode = SpiMskMaster;     //Configure master mode
    SpiInitStruct.enPclkDiv = SpiClkMskDiv4;    //Baud rate: fsys/128
    SpiInitStruct.enCPHA    = SpiMskCphafirst;  //First edge sampling
    SpiInitStruct.enCPOL    = SpiMskcpollow;    //Polarity is low
    Spi_Init(M0P_SPI0, &SpiInitStruct);
    Spi_ClearStatus(M0P_SPI0);

    hw_spi_gpio_init();
}

void hw_lp_enter_gpio_cfg(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    
    Gpio_DisableIrq(GPIO_PORT_IRQ, GPIO_PIN_IRQ, GpioIrqRising);
    Gpio_DisableIrq(GPIO_PORT_CAD, GPIO_PIN_CAD, GpioIrqRising);

    stcGpioCfg.enDir = GpioDirIn;
    stcGpioCfg.enPu  = GpioPuDisable;
    stcGpioCfg.enDrv = GpioDrvL;

    /* spi tx rx gpio config */
    stcGpioCfg.enPu  = GpioPuDisable;
    stcGpioCfg.enPd  = GpioPdDisable;
    Gpio_Init(GpioPortA, GpioPin5, &stcGpioCfg);
    Gpio_Init(GpioPortA, GpioPin6, &stcGpioCfg);
    Gpio_Init(GpioPortA, GpioPin7, &stcGpioCfg);

    /* rf irq/cad gpio config */
    stcGpioCfg.enPu  = GpioPuDisable;
    stcGpioCfg.enPd  = GpioPdDisable;
    Gpio_Init(GPIO_PORT_IRQ, GPIO_PIN_IRQ, &stcGpioCfg);
    Gpio_Init(GPIO_PORT_CAD, GPIO_PIN_CAD, &stcGpioCfg);

    Gpio_Init(GpioPortB, GpioPin3, &stcGpioCfg);
    Gpio_Init(GpioPortB, GpioPin4, &stcGpioCfg);
    Gpio_Init(GpioPortB, GpioPin5, &stcGpioCfg);
}

void hw_lp_exit_gpio_cfg(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    
    stcGpioCfg.enDir = GpioDirIn;
    stcGpioCfg.enDrv = GpioDrvL;
    stcGpioCfg.enPu = GpioPuDisable;
    stcGpioCfg.enPd = GpioPdEnable;
    stcGpioCfg.enOD = GpioOdDisable;
    stcGpioCfg.enCtrlMode = GpioAHB;
    Gpio_Init(GPIO_PORT_IRQ, GPIO_PIN_IRQ, &stcGpioCfg);//pan3029 irq
    Gpio_EnableIrq(GPIO_PORT_IRQ, GPIO_PIN_IRQ, GpioIrqRising);
    
    Gpio_Init(GPIO_PORT_CAD, GPIO_PIN_CAD, &stcGpioCfg);//cad check
    Gpio_EnableIrq(GPIO_PORT_CAD, GPIO_PIN_CAD, GpioIrqRising);
    
    EnableNvic(PORTA_IRQn, IrqLevel3, TRUE);
    EnableNvic(PORTB_IRQn, IrqLevel3, TRUE);

//    stcGpioCfg.enPu = GpioPuEnable;
//    stcGpioCfg.enPd = GpioPdDisable;

//    Gpio_Init(GpioPortB, GpioPin3, &stcGpioCfg);
//    Gpio_Init(GpioPortB, GpioPin4, &stcGpioCfg);
//    Gpio_Init(GpioPortB, GpioPin5, &stcGpioCfg);

    hw_spi_gpio_init();
}


//UART1 interrupt function
void Uart0_IRQHandler(void)
{
    static uint8_t u8RxData;

    if(Uart_GetStatus(M0P_UART0, UartRC))         //UART0 rx irq
    {
        u8RxData = Uart_ReceiveData(M0P_UART0);   //get data
        Uart_ClrStatus(M0P_UART0, UartRC);        //clear rx flag

        /* uart tx rx conflict */
        if ( uart_tx_busy_flag )
        {
            uart_rx_busy_flag = FALSE;
            uart_rx_fifo_flush();
            Bt_M0_Stop(TIM1);
            Bt_ClearIntFlag(TIM1,BtUevIrq);
            return;
        }
        
        /* indicate that package is receiving */
        uart_rx_busy_flag = TRUE;
        
        if(!fifo_is_full(&uart_rx_fifo))
        {
            fifo_push(&uart_rx_fifo, u8RxData);
            /* if fifo is not empty,set aux high */
            if ( aux_mode == AUX_MODE_EMPTY )
            {
                aux_level_set(AUX_FIFO, LEVEL_ACTIVE);
            }
        }
        else
        {
            uart_rx_busy_flag = FALSE;
            
            /* set uart recv evt */
            pt_uart_set_rx_evt();
            net_uart_set_evt();

            /* if fifo is full,set aux high */
            if ( aux_mode == AUX_MODE_FULL )
            {
                aux_level_set(AUX_FIFO, LEVEL_ACTIVE);
            }
        }
        
        hw_timer1_restart();
    }
    else if(Uart_GetStatus(M0P_UART0, UartTC)) 
    {
        Uart_ClrStatus(M0P_UART0, UartTC);
    }
    else if(Uart_GetStatus(M0P_UART0, UartFE)) 
    {
        Uart_ClrStatus(M0P_UART0, UartFE);
    }
    else if(Uart_GetStatus(M0P_UART0, UartTxe)) 
    {
        Uart_ClrStatus(M0P_UART0, UartTxe);
    }
    else if(Uart_GetStatus(M0P_UART0, UartPE)) 
    {
        Uart_ClrStatus(M0P_UART0, UartPE);
    }
    else if(Uart_GetStatus(M0P_UART0, UartCtsIf)) 
    {
        Uart_ClrStatus(M0P_UART0, UartCtsIf);
    }
    else if(Uart_GetStatus(M0P_UART0, UartCts)) 
    {
        Uart_ClrStatus(M0P_UART0, UartCts);
    }
}

//Serial port pin configuration
void hw_uart_gpio_init(void)
{
    stc_gpio_cfg_t stcGpioCfg;
    DDL_ZERO_STRUCT(stcGpioCfg);
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE); //Enable GPIO module clock
    
    stcGpioCfg.enCtrlMode = GpioAHB;
    stcGpioCfg.enDrv = GpioDrvL;
    stcGpioCfg.enOD  = GpioOdDisable;
    stcGpioCfg.enPd  = GpioPdDisable;
    
    stcGpioCfg.enDir = GpioDirOut;
    stcGpioCfg.enPu  = GpioPuDisable;
    Gpio_Init(GpioPortA, GpioPin9, &stcGpioCfg);
    Gpio_SetAfMode(GpioPortA, GpioPin9, GpioAf1); //Configure the PA09 port as URART0_TX

    stcGpioCfg.enDir = GpioDirIn;
    stcGpioCfg.enPu  = GpioPuEnable;
    Gpio_Init(GpioPortA, GpioPin10, &stcGpioCfg);
    Gpio_SetAfMode(GpioPortA, GpioPin10, GpioAf1); //Configure PA10 port to URART0_RX
}

//Serial port configuration
void hw_uart_cfg(void)
{
    stc_uart_cfg_t stcCfg;
    DDL_ZERO_STRUCT(stcCfg);
    Sysctrl_SetPeripheralGate(SysctrlPeripheralUart0, TRUE);///<Enable UART0 module clock

    stcCfg.enRunMode        = UartMskMode1;          ///<Mode 1
    stcCfg.enStopBit        = UartMsk1bit;           ///<1-bit stop bit
    stcCfg.enMmdorCk        = UartMskDataOrAddr;     ///<Even parity check
    stcCfg.stcBaud.u32Baud  = uart_baudrate;         ///<Baud rate 9600
    stcCfg.stcBaud.enClkDiv = UartMsk8Or16Div;       ///<Channel sampling frequency division configuration
    stcCfg.stcBaud.u32Pclk  = Sysctrl_GetPClkFreq(); ///<Get peripheral clock (PCLK) frequency
    Uart_Init(M0P_UART0, &stcCfg);                   ///<Serial port initialization

    ///<UART interrupt enable
    Uart_ClrStatus(M0P_UART0,UartRC);                ///<Clear receive request
    Uart_ClrStatus(M0P_UART0,UartTC);                ///<Clear transmit request
    Uart_EnableIrq(M0P_UART0,UartRxIrq);             ///< Enable serial port receive interrupt

    EnableNvic(UART0_IRQn, IrqLevel0, TRUE);         ///< Enable system interrupt

}

int uart_fifo_read(u8 *buf, int bufLen)
{
    int len = 0;

    for(int i = 0; i < bufLen; i++)
    {
        if(!fifo_is_empty(&uart_rx_fifo))
        {
            buf[i] = fifo_pop(&uart_rx_fifo);
            len++;
        }
        else
        {
            break;
        }
    }

    return len;
}

void uart_rx_fifo_flush(void)
{
    fifo_flush(&uart_rx_fifo);
}

/*******************************************************************************
 * TIM0 interrupt service function
 ******************************************************************************/
void Tim0_IRQHandler(void)
{
    static uint8_t i;
    
    //Timer0 Mode 0 Overflow Interrupt
    if(TRUE == Bt_GetIntFlag(TIM0, BtUevIrq))
    {
        Bt_ClearIntFlag(TIM0,BtUevIrq); //Clear interrupt flag
    }
}

//Timer0 configuration initialization
void hw_timer0_cfg(void)
{
    uint16_t u16ArrValue;
    uint16_t u16CntValue;
    stc_bt_mode0_cfg_t stcBtBaseCfg;
    
    DDL_ZERO_STRUCT(stcBtBaseCfg);
    
    Sysctrl_SetPeripheralGate(SysctrlPeripheralBaseTim, TRUE); //Base Timer peripheral clock enable
    
    stcBtBaseCfg.enWorkMode = BtWorkMode0;                  //Timer mode
    stcBtBaseCfg.enCT       = BtTimer;                      //Timer function, count clock is internal PCLK
    stcBtBaseCfg.enPRS      = BtPCLKDiv32;                  //PCLK/32
    stcBtBaseCfg.enCntMode  = Bt32bitFreeMode;              //Auto-reload 32-bit counter/timer
    stcBtBaseCfg.bEnTog     = FALSE;
    stcBtBaseCfg.bEnGate    = FALSE;
    stcBtBaseCfg.enGateP    = BtGatePositive;
    Bt_Mode0_Init(TIM0, &stcBtBaseCfg);                     //TIM0 Mode 0 function initialization
}

/*******************************************************************************
 * TIM1 interrupt service function
 ******************************************************************************/

void Tim1_IRQHandler(void)
{
    if(TRUE == Bt_GetIntFlag(TIM1, BtUevIrq))
    {
        Bt_ClearIntFlag(TIM1,BtUevIrq);

        Bt_M0_Stop(TIM1);

        uart_rx_busy_flag = FALSE;
        
        /* set uart recv evt */
        pt_uart_set_rx_evt();
        net_uart_set_evt();
    }
}

//Timer1 configuration initialization
void hw_timer1_cfg(void)
{
    uint16_t u16Period = 48000;
    uint16_t u16ArrValue;
    uint16_t u16CntValue;
    stc_bt_mode0_cfg_t stcBtBaseCfg;

    // uart_packing_interval = ms
    uart_idle_timer_cnt = SystemCoreClock / 16000 * uart_packing_interval;
    uart_idle_timer_cnt = 0x10000 - uart_idle_timer_cnt;

    DDL_ZERO_STRUCT(stcBtBaseCfg);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralBaseTim, TRUE); // Base Timer peripheral clock enable

    stcBtBaseCfg.enWorkMode = BtWorkMode0;   // Timer mode
    stcBtBaseCfg.enCT = BtTimer;             // Timer function, count clock is internal PCLK
    stcBtBaseCfg.enPRS = BtPCLKDiv16;        // PCLK/256
    stcBtBaseCfg.enCntMode = Bt16bitArrMode; // Auto-reload 16-bit counter/timer
    stcBtBaseCfg.bEnTog = FALSE;
    stcBtBaseCfg.bEnGate = FALSE;
    stcBtBaseCfg.enGateP = BtGatePositive;
    Bt_Mode0_Init(TIM1, &stcBtBaseCfg); // Initialize TIM0 Mode 0 function

    // u16ArrValue = 0x10000 - u16Period;
    Bt_M0_ARRSet(TIM1, uart_idle_timer_cnt); // Set reload value (ARR = 0x10000 - period)

    // u16CntValue = 0x10000 - u16Period;
    Bt_M0_Cnt16Set(TIM1, uart_idle_timer_cnt); // Set the initial count value

    Bt_ClearIntFlag(TIM1, BtUevIrq);        // Clear the interrupt flag
    Bt_Mode0_EnableIrq(TIM1);               // Enable the TIM0 interrupt (only one interrupt in mode 0)
    EnableNvic(TIM1_IRQn, IrqLevel3, TRUE); // Enable the TIM0 interrupt

    // Bt_M0_Run(TIM1);
}

void hw_timer1_restart(void)
{
    Bt_M0_Stop(TIM1);
    Bt_M0_Cnt16Set(TIM1, uart_idle_timer_cnt); 
    Bt_M0_Run(TIM1);  
}

uint32_t total_ticks = 0;

uint32_t Micros_timer2(void)
{
    return total_ticks;
}

void Tim2_IRQHandler(void)
{
    total_ticks++;

    if(TRUE == Bt_GetIntFlag(TIM2, BtUevIrq))
    {
        Bt_ClearIntFlag(TIM2,BtUevIrq); //Clear interrupt flag
    }
}

//Timer2 configuration initialization
void hw_timer2_cfg(void)
{
    uint16_t u16Period = 48000;
    uint16_t                  u16ArrValue;
    uint16_t                  u16CntValue;
    stc_bt_mode0_cfg_t     stcBtBaseCfg;

    uart_idle_timer_cnt = 0x10000 - 1507;//1ms 24Mhz

    DDL_ZERO_STRUCT(stcBtBaseCfg);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralBaseTim, TRUE); //Base Timer peripheral clock enable
    
    stcBtBaseCfg.enWorkMode = BtWorkMode0;                  //Timer mode
    stcBtBaseCfg.enCT       = BtTimer;                      //Timer function, count clock is internal PCLK
    stcBtBaseCfg.enPRS      = BtPCLKDiv16;                 //PCLK/256
    stcBtBaseCfg.enCntMode  = Bt16bitArrMode;               //Auto-reload 16-bit counter/timer
    stcBtBaseCfg.bEnTog     = FALSE;
    stcBtBaseCfg.bEnGate    = FALSE;
    stcBtBaseCfg.enGateP    = BtGatePositive;
    Bt_Mode0_Init(TIM2, &stcBtBaseCfg);                     // Initialize TIM2 Mode 0 function
    
    //u16ArrValue = 0x10000 - u16Period;
    Bt_M0_ARRSet(TIM2, uart_idle_timer_cnt);                        // Set the reload value (ARR = 0x10000 - period)
    
    //u16CntValue = 0x10000 - u16Period;
    Bt_M0_Cnt16Set(TIM2, uart_idle_timer_cnt);                      // Set the initial count value
    
    Bt_ClearIntFlag(TIM2,BtUevIrq);                         // Clear the interrupt flag  
    Bt_Mode0_EnableIrq(TIM2);                               //Enable TIM2 interrupt (only one interrupt in mode 0)
    EnableNvic(TIM2_IRQn, IrqLevel0, TRUE);                 //Enable TIM2 interrupt
    Bt_M0_Run(TIM2); 
}

void hw_rng_cfg(void)
{
    Sysctrl_SetPeripheralGate(SysctrlPeripheralRng, TRUE);
    Trng_Init();
}

void mcu_enter_lp(void)
{
    EnableNvic(UART0_IRQn, IrqLevel3, FALSE);

    __disable_irq();
    hw_lp_enter_gpio_cfg();
    __enable_irq();
    
    Lpm_GotoDeepSleep(FALSE);
}

void mcu_exit_lp(void)
{
    hw_pll_init();
    
    __disable_irq();
#ifdef DDL_UART_DMA_YES
    app_uart0_dma_cfg();
#endif
    hw_lp_exit_gpio_cfg();
    hw_uart_cfg();
    hw_spi_cfg();
    Hal_Pwm_Init();
    PWM_Set_RGB(0);
    DS18B20_Init();
    hw_timer2_cfg();
    __enable_irq();
}

/**
 * @brief spi_cs_set_high
 * @param[in] <none>
 * @return none
 */
void spi_cs_high(void)
{
    Spi_SetCS(M0P_SPI0, TRUE);
}

/**
 * @brief spi_cs_set_low
 * @param[in] <none>
 * @return none
 */
void spi_cs_low(void)
{
    Spi_SetCS(M0P_SPI0, FALSE);
}

uint8_t spi_writereadbyte(uint8_t tx_data)
{
    while(Spi_GetStatus(M0P_SPI0, SpiTxe) == FALSE);
    Spi_SendData(M0P_SPI0, tx_data);

    while(Spi_GetStatus(M0P_SPI0, SpiRxne) == FALSE);
    return  Spi_ReceiveData(M0P_SPI0);
}

#ifdef DDL_UART_DMA_YES
// DMA channel configuration, ADC SQR triggers DMA transfer
void app_uart0_dma_cfg(void)
{
    stc_dma_cfg_t stcDmaCfg;
    
    DDL_ZERO_STRUCT(stcDmaCfg);   
    
    // Enable DMA clock
    Sysctrl_SetPeripheralGate(SysctrlPeripheralDma, TRUE);   
    
    stcDmaCfg.enMode =  DmaMskBlock;                             // Select burst transfer  
    stcDmaCfg.u16BlockSize = 0x01u;                              // Number of block transfers
    stcDmaCfg.u16TransferCnt = 0x03u;                            // Burst mode, the data size of a single trigger transfer is 3*3,
    stcDmaCfg.enTransferWidth = DmaMsk8Bit;                      //Transfer data width, here select word (8-bit) width    
    stcDmaCfg.enSrcAddrMode = DmaMskSrcAddrInc;                  //Source address fixed
    stcDmaCfg.enDstAddrMode = DmaMskDstAddrFix;                  //Destination address auto-increment
    stcDmaCfg.enDestAddrReloadCtl = DmaMskDstAddrReloadDisable;  //Disable reloading of transfer destination address
    stcDmaCfg.enSrcAddrReloadCtl = DmaMskSrcAddrReloadDisable;   //Disable reloading of transfer source address
    stcDmaCfg.enSrcBcTcReloadCtl = DmaMskBcTcReloadDisable;      //Disable reloading of BC/TC values
    stcDmaCfg.u32SrcAddress = (uint32_t)0x20000000;              //Specify the transfer source address
    stcDmaCfg.u32DstAddress = (uint32_t)&(M0P_UART0->SBUF);      //Specify the transfer destination address
    stcDmaCfg.enTransferMode = DmaMskOneTransfer;                //DMAC clears the CONFA:ENS bits upon transfer completion. Enables one DMA transfer.
    stcDmaCfg.enRequestNum = DmaUart0TxTrig;                     //Set to ADCSQR trigger
    stcDmaCfg.enPriority = DmaMskPriorityFix;                    //Fixed priority for each channel, CH0 priority > CH1 priority
    
    Dma_InitChannel(DmaCh0,&stcDmaCfg);                          //Initialize DMA channel 0

    Dma_EnableChannelIrq(DmaCh0);
    Dma_EnableChannelErrIrq(DmaCh0);
    EnableNvic(DMAC_IRQn, IrqLevel3, TRUE);                      ///<Enable system interrupts
}

int hw_uart0_dma_send(u8 *buf, int len)
{
    uart_tx_busy_flag = TRUE;
    uart_dmac_tx_flag = FALSE;
    
    Dma_ClrStat(DmaCh0);
    
    Dma_SetTransferCnt(DmaCh0, len);
    Dma_SetSourceAddress(DmaCh0, (uint32_t)buf);
    Dma_Enable();
    Dma_EnableChannel(DmaCh0);
    Uart_EnableFunc(M0P_UART0, UartDmaTxFunc);
    
    while(!uart_dmac_tx_flag);
    while (TRUE != M0P_UART0->ISR_f.TC);
    
    Uart_DisableFunc(M0P_UART0, UartDmaTxFunc);
    
    uart_tx_busy_flag = FALSE;
    
    return len;
}

void Dmac_IRQHandler(void)
{
    uart_dmac_tx_flag = TRUE;
    Dma_ClrStat(DmaCh0);
}
#endif
/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
 
