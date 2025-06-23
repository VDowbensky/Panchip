/**************************************************************************//**
 * @file     uart_common.c
 * @version  V1.0
 * $Date:    19/09/25 14:00 $
 * @brief    Source file for UART test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <ctype.h>
#include "PanSeries.h"
#include "uart_common.h"

uint8_t uartRxBuffer[UART_RX_BUF_SIZE];
uint8_t uartTxBuffer[UART_TX_BUF_SIZE];

volatile uint32_t uartRxBufIdx = 0;
volatile bool uartRxIntFlag = false;
uint32_t uartTxBufIdx = 0;

volatile bool uartTxDone = false;
volatile bool uartAddrRcvd = 0; // 9-bit mode address received flag

T_UART_TEST_RESULT (* const UART_TestCase[])(void) = 
{
    UART_RegisterDefaultValueCheckCase0,
    UART_DataLenSelectAndStopBitTestCase1,
    UART_ParityCheckTestCase2,
    UART_BaudrateTestCase3,
    UART_FifoAndThresholdTestCase4,
    UART_FifoAndLineStatusInterruptTestCase5,
    UART_AutoFlowControlTestCase6,
    UART_TransferWithDmaTestCase7,
    UART_9BitModeTestCase8
};

void UART_TestFunctionEnter(uint16_t TcIdx)
{
    T_UART_TEST_RESULT r = UART_TST_OK;

    if (TcIdx >= sizeof(UART_TestCase) / sizeof(void*))
    {
        SYS_TEST("Error, cannot find Testcase %d!", TcIdx);
        return;
    }

    r = (UART_TestCase[TcIdx])();
    if (r != UART_TST_OK)
    {
        SYS_TEST("UART Test Fail, Fail case: %d, Error Code: %d\n", TcIdx, r);
    }
    else
    {
        SYS_TEST("UART Test OK, Success case: %d\n", TcIdx);
    }
}

void UART_HandleLineStatus(UART_T* UARTx)
{
    char str[80];
    uint32_t lineStatus = UART_GetLineStatus(UARTx);

    // Filter line error
    if ((lineStatus & (UART_LINE_OVERRUN_ERR | UART_LINE_PARITY_ERR | UART_LINE_FRAME_ERR | UART_LINE_RX_FIFO_ERR)) != 0x0)
    {
        //Reset Rx fifo once there is a line error
        //UART_ResetRxFifo(UARTx);
        //TGT_ClrAllReadyData();
        SYS_TEST("An Uart line error occured, error type: %s\n", ConvLsrErrFlagToStr(lineStatus, str, sizeof(str)));
    }

    // Handle THRE event
    if (lineStatus & UART_LINE_THRE)
    {
        if ((UARTx->IER_DLH & UART_IER_EPTI_Msk) == 0)
            SYS_TEST("Line status, THR or TxFIFO empty!\n");
        else if ((UARTx->IIR_FCR | UART_IIR_FIFOSE_Msk) == 1)
            SYS_TEST("Line status, Tx FIFO full!\n");
        else
        {
            SYS_TEST("Not expected running to here!\n");
        }
    }

    // Handle 9-bit mode ADDR_RCVD event
    if ((lineStatus & (UART_LINE_ADDR_RCVD)) != 0x0)
    {
        //Set corresponding flag
        uartAddrRcvd = 1;
    }
}

void UART_HandleModemStatus(UART_T* UARTx)
{
    uint32_t modemStatus = UART_GetModemStatus(UARTx);

    SYS_TEST("An Uart modem event occurs, modemStatus=0x%x\n", modemStatus);
}

void UART_HandleReceivedData(UART_T* UARTx)
{
    while (!UART_IsRxFifoEmpty(UARTx))
    {
        uartRxBuffer[uartRxBufIdx++] = UART_ReceiveData(UARTx);
        if (uartRxBufIdx >= UART_RX_BUF_SIZE)   //Rx buffer full
        {
            SYS_TEST("Error, too much data received, Rx buffer full!\n");
            uartRxBufIdx = 0;   //Reset Rx buf index
            break;
        }
    }
}

void UART_HandleTransmittingData(UART_T* UARTx)
{
    static uint32_t uartTxMark = 0;
    while (!UART_IsTxFifoFull(UARTx))
    {
        if (uartTxMark < uartTxBufIdx)
        {
            UART_SendData(UARTx, uartTxBuffer[uartTxMark++]);
        }
        else
        {
            uartTxDone = true;
            uartTxMark = 0; //Reset Tx mark while transmitting done
            UART_DisableIrq(TGT_UART, UART_IRQ_THR_EMPTY);   //Disable THRE Interrupt after transmitting done
            break;
        }
    }
}

void UART_HandleProc(UART_T* UARTx)
{
    UART_EventDef event = UART_GetActiveEvent(UARTx);
    //SYS_TEST("UART (0x%x) Hdl IID: 0x%x\n", (uint32_t)UARTx, event);

    switch (event)
    {
    case UART_EVENT_LINE:
        UART_HandleLineStatus(UARTx);
        break;
    case UART_EVENT_DATA:
        SYS_TEST("Rx thres trig!\n");
        UART_HandleReceivedData(UARTx);
		uartRxIntFlag = true;
        break;
    case UART_EVENT_TIMEOUT:
        SYS_TEST("Rx tout trig!\n");
        UART_HandleReceivedData(UARTx);
		uartRxIntFlag = true;
        break;
    case UART_EVENT_THR_EMPTY:
        SYS_TEST("Tx thres trig!\n");
        UART_HandleTransmittingData(UARTx);
        break;
    case UART_EVENT_MODEM:
        UART_HandleModemStatus(UARTx);
        break;
    case UART_EVENT_NONE:
        /* Just ignore this event. */
        break;
    default:
        SYS_TEST("UART Handler Error, is not expected running to here! IID: 0x%x\n", event);
        break;
    }
}

void UART0_IRQHandler(void)
{
    UART_HandleProc(UART0);
}

void UART1_IRQHandler(void)
{
    UART_HandleProc(UART1);
}

void UART_TestModuleInit(void)
{
#if TGT_UART_ID == TEST_UART1
    // Enable Clock
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    // Config Pinmux
    SYS_SET_MFP(P1, 0, UART1_TX);
    SYS_SET_MFP(P0, 7, UART1_RX);
    GPIO_EnableDigitalPath(P0, BIT7);
    SYS_SET_MFP(P2, 7, UART1_CTS);
    SYS_SET_MFP(P2, 6, UART1_RTS);

    GPIO_EnableDigitalPath(P2, BIT7);
#else
#error "Error, Does not support this test TGT_UART_ID now."
#endif
}

// Read out all ready data from RBR or FIFO and just discard them
void TGT_ClrAllReadyData(void)
{
    volatile uint8_t c;
    while(TGT_UART->LSR & UART_LSR_DR_Msk)
    {
        c = TGT_UART->RBR_THR_DLL;
    }
}

// Get character from target test Uart
char TGT_GetChar(uint32_t* line_status)
{
    volatile uint32_t lsr;
    do
    {
        lsr = TGT_UART->LSR;
    }
    while (!(lsr & UART_LSR_DR_Msk)); // Wait until there is a new data in RBR

    if (line_status != NULL)
        *line_status = lsr;

    return (char)(TGT_UART->RBR_THR_DLL);
}

// Try to receive multi-data until getting expected number of bytes
void TGT_ReceiveMultiData(uint8_t* data, size_t expect_size)
{
    TGT_ClrAllReadyData();

    while (expect_size--)
    {
        *data++ = TGT_GetChar(NULL);
    }
}

// Send multiple data to target test Uart
void TGT_SendMultiData(const uint8_t* data, size_t size)
{
    while (size--)
    {
        TGT_UART->RBR_THR_DLL = *(data++);
        while(!(TGT_UART->LSR & UART_LSR_TEMT_Msk));    // Wait until THR is empty to avoid data lost
    }
}

/*
    Encode a Byte to Hex Character Sequence tailing with a space.
    e.g.    Byte (1B) --> Hex (5B)
            0x7E      --> '0''x''7''E'' '
*/
static void HexEncodeByte(uint8_t b, char* out)
{
    static const char hex_chars[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                     '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    unsigned char idx1 = (b & 0xF0) >> 4;
    unsigned char idx2 = b & 0x0F;
    out[0] = '0';
    out[1] = 'x';
    out[2] = hex_chars[idx1];
    out[3] = hex_chars[idx2];
    out[4] = ' ';
}

/*
    Encode a Byte Array to a Hexadecimal Displayed String.
    e.g.    Byte Array (3 Bytes) --> String (15 Bytes)
            {0x05, 0x7A, 0xFE}   --> "0x05 0x7A 0xFE"
*/
char* Hexlify(const uint8_t* bin_in, size_t n, char* hex_out)
{
    char* h = hex_out;  
    for (int i = 0; i < n; i++) {
        HexEncodeByte(bin_in[i], h);
        h += 5;
    }
    *(--h) = '\0';    //Replace the last space with '\0'
    return hex_out;
}

char* ConvLsrErrFlagToStr(uint32_t line_status, char* str_buf, size_t buf_size)
{
    static const char StrRFE[] = "Receiver FIFO Error | ";
    static const char StrFE[] = "Framing Error | ";
    static const char StrPE[] = "Parity Error | ";
    static const char StrOE[] = "Overrun Error | ";

    if (buf_size < strlen(StrRFE) + strlen(StrFE) + strlen(StrPE) + strlen(StrOE) + 2 + 1) //22+16+15+16+3=72
    {
        return NULL;
    }

    strcpy(str_buf, "| ");

    if ((line_status & (UART_LSR_RFE_Msk | UART_LSR_FE_Msk | UART_LSR_PE_Msk | UART_LSR_OE_Msk)) == 0)
    {
        strcat(str_buf, "No Error |");
    }
    else
    {
        if (line_status & UART_LSR_RFE_Msk)
        {
            strcat(str_buf, StrRFE);
        }
        if (line_status & UART_LSR_FE_Msk)
        {
            strcat(str_buf, StrFE);
        }
        if (line_status & UART_LSR_PE_Msk)
        {
            strcat(str_buf, StrPE);
        }
        if (line_status & UART_LSR_OE_Msk)
        {
            strcat(str_buf, StrOE);
        }
    }
    return str_buf;
}


T_UART_TEST_RESULT UART_RegisterDefaultValueCheckCase0(void)
{
    SYS_TEST("Target UART%d Register Default Values:\n", TGT_UART_ID);
    SYS_TEST("---------------------------\n");
    SYS_TEST("RBR_THR_DLL = 0x%08x\n", TGT_UART->RBR_THR_DLL);
    SYS_TEST("IER_DLH     = 0x%08x\n", TGT_UART->IER_DLH    );
    SYS_TEST("IIR_FCR     = 0x%08x\n", TGT_UART->IIR_FCR    );
    SYS_TEST("LCR         = 0x%08x\n", TGT_UART->LCR        );
    SYS_TEST("MCR         = 0x%08x\n", TGT_UART->MCR        );
    SYS_TEST("LSR         = 0x%08x\n", TGT_UART->LSR        );
    SYS_TEST("MSR         = 0x%08x\n", TGT_UART->MSR        );
    SYS_TEST("SCR         = 0x%08x\n", TGT_UART->SCR        );
    SYS_TEST("LPDLL       = 0x%08x\n", TGT_UART->LPDLL      );
    SYS_TEST("LPDLH       = 0x%08x\n", TGT_UART->LPDLH      );
    SYS_TEST("USR         = 0x%08x\n", TGT_UART->USR        );
    SYS_TEST("TFL         = 0x%08x\n", TGT_UART->TFL        );
    SYS_TEST("RFL         = 0x%08x\n", TGT_UART->RFL        );
    SYS_TEST("DMASA       = 0x%08x\n", TGT_UART->DMASA      );
    SYS_TEST("DLF         = 0x%08x\n", TGT_UART->DLF        );
    SYS_TEST("RAR         = 0x%08x\n", TGT_UART->RAR        );
    SYS_TEST("TAR         = 0x%08x\n", TGT_UART->TAR        );
    SYS_TEST("LCR_EXT     = 0x%08x\n", TGT_UART->LCR_EXT    );

    return UART_TST_OK;
}
