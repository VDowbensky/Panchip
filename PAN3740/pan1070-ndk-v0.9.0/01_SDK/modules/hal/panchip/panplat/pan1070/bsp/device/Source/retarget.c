/**************************************************************************//**
 * @file     retarget.c
 * @version  V2.00
 * @brief    PAN10xx series retarget source file
 *
 * @note
 * Copyright (C) 2016-2025 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "PanSeries.h"
#if CONFIG_RTT_LOG_ENABLE
#include "SEGGER_RTT.h"
#endif

/* Suppress the build warning #111-D: statement is unreachable */
#pragma diag_suppress 111

#if defined ( __CC_ARM   )
#if (__ARMCC_VERSION < 400000)
#else
/* Insist on keeping widthprec, to avoid X propagation by benign code in C-lib */
#pragma import _printf_widthprec
#endif
#endif

//#define DEBUG_ENABLE_UART1

#ifdef DEBUG_ENABLE_UART1
#define DEBUG_PORT         UART1
#else
#define DEBUG_PORT         UART0
#endif

//#if !(defined(__ICCARM__) && (__VER__ >= 6010000))
//struct __FILE { int handle; /* Add whatever you need here */ };
//#endif
//FILE __stdout;
//FILE __stdin;

enum { r0, r1, r2, r3, r12, lr, pc, psr};

void stackDump(uint32_t stack[])
{
    SYS_PRINT("r0  = 0x%x\r\n", stack[r0]);
    SYS_PRINT("r1  = 0x%x\r\n", stack[r1]);
    SYS_PRINT("r2  = 0x%x\r\n", stack[r2]);
    SYS_PRINT("r3  = 0x%x\r\n", stack[r3]);
    SYS_PRINT("r12 = 0x%x\r\n", stack[r12]);
    SYS_PRINT("lr  = 0x%x\r\n", stack[lr]);
    SYS_PRINT("pc  = 0x%x\r\n", stack[pc]);
    SYS_PRINT("psr = 0x%x\r\n", stack[psr]);

}

void Hard_Fault_Handler(uint32_t stack[])
{
#if CONFIG_IO_TIMING_TRACK
    PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_HARDFAULT_IRQ, 1);
#endif

    SYS_PRINT("In Hard Fault Handler\r\n");

    stackDump(stack);

    while(1){};

    // Although program won't run into here, we write a statement
    // just for symmetry.
#if CONFIG_IO_TIMING_TRACK
    PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_HARDFAULT_IRQ, 0);
#endif
}


#if defined(DEBUG_ENABLE_SEMIHOST)
/* The static buffer is used to speed up the semihost */
static char g_buf[16];
static char g_buf_len = 0;

# if defined(__ICCARM__)


/**
 * @brief    This HardFault handler is implemented to support semihost
 *
 * @param    None
 *
 * @returns  None
 *
 * @details  This function is implement to support semihost message print.
 *
 */
void HardFault_Handler(void)
{
    asm("MOV     R0, lr        \n"
        "LSLS    R0, #29       \n"        //; Check bit 2
        "BMI     SP_is_PSP     \n"        //; previous stack is PSP
        "MRS     R0, MSP       \n"        //; previous stack is MSP, read MSP
        "B       SP_Read_Ready \n"
        "SP_is_PSP:            \n"
        "MRS     R0, PSP       \n"        //; Read PSP
        "SP_Read_Ready:        \n"
        "LDR     R1, [R0, #24] \n"        //; Get previous PC
        "LDRH    R3, [R1]      \n"        //; Get instruction
        "LDR     R2, [pc, #8]  \n"        //; The special BKPT instruction
        "CMP     R3, R2        \n"        //; Test if the instruction at previous PC is BKPT
        "BNE     HardFault_Handler_Ret\n" //; Not BKPT
        "ADDS    R1, #4        \n"        //; Skip BKPT and next line
        "STR     R1, [R0, #24] \n"        //; Save previous PC
        "BX      lr            \n"        //; Return
        "DCD     0xBEAB        \n"        //; BKPT instruction code
        "HardFault_Handler_Ret:\n"
        "MOVS    r0, #4                        \n"
        "MOV     r1, LR                        \n"
        "TST     r0, r1                        \n"
        "BEQ     Stack_Use_MSP                 \n"
        "MRS     R0, PSP                       \n" //; stack use PSP
        "B       Get_LR_and_Branch             \n"
        "Stack_Use_MSP:                        \n"
        "MRS     R0, MSP                       \n" //; stack use MSP
        "Get_LR_and_Branch:                    \n"
        "MOV     R1, LR                        \n" //; LR current value
        "B Hard_Fault_Handler        \n"
       );

    while(1);
}

/**
 *
 * @brief      The function to process semihosted command
 * @param[in]  n32In_R0  : semihost register 0
 * @param[in]  n32In_R1  : semihost register 1
 * @param[out] pn32Out_R0: semihost register 0
 * @retval     0: No ICE debug
 * @retval     1: ICE debug
 *
 */
int32_t SH_DoCommand(int32_t n32In_R0, int32_t n32In_R1, int32_t *pn32Out_R0)
{
    asm("BKPT   0xAB    \n"       //; This instruction will cause ICE trap or system HardFault
        "B      SH_ICE  \n"
        "SH_HardFault:  \n"       //; Captured by HardFault
        "MOVS   R0, #0  \n"       //; Set return value to 0
        "BX     lr      \n"       //; Return
        "SH_ICE:        \n"       //; Captured by ICE
        "CMP    R2, #0  \n"
        "BEQ    SH_End  \n"
        "STR    R0, [R2]\n"       //; Save the return value to *pn32Out_R0
        "SH_End:        \n");

    return 1;                 //; Return 1 when it is trap by ICE

}


# else

/**
 * @brief    This HardFault handler is implemented to support semihost
 *
 * @param    None
 *
 * @returns  None
 *
 * @details  This function is implement to support semihost message print.
 *
 */
__asm int32_t HardFault_Handler(void)
{

    MOV     R0, LR
    LSLS    R0, #29               //; Check bit 2
    BMI     SP_is_PSP             //; previous stack is PSP
    MRS     R0, MSP               //; previous stack is MSP, read MSP
    B       SP_Read_Ready
SP_is_PSP
    MRS     R0, PSP               //; Read PSP

SP_Read_Ready
    LDR     R1, [R0, #24]         //; Get previous PC
    LDRH    R3, [R1]              //; Get instruction
    LDR     R2, =0xBEAB           //; The special BKPT instruction
                 CMP     R3, R2                //; Test if the instruction at previous PC is BKPT
                 BNE     HardFault_Handler_Ret //; Not BKPT

                 ADDS    R1, #4                //; Skip BKPT and next line
                 STR     R1, [R0, #24]         //; Save previous PC

                 BX      LR                    //; Return
HardFault_Handler_Ret

    /* TODO: Implement your own hard fault handler here. */
    MOVS    r0, #4
    MOV     r1, LR
    TST     r0, r1
    BEQ     Stack_Use_MSP
    MRS     R0, PSP ;stack use PSP
    B       Get_LR_and_Branch
Stack_Use_MSP
    MRS     R0, MSP ; stack use MSP
Get_LR_and_Branch
    MOV     R1, LR ; LR current value
    LDR     R2,=__cpp(Hard_Fault_Handler)
    BX      R2

                 B       .

                 ALIGN
}

/**
 *
 * @brief      The function to process semihosted command
 * @param[in]  n32In_R0  : semihost register 0
 * @param[in]  n32In_R1  : semihost register 1
 * @param[out] pn32Out_R0: semihost register 0
 * @retval     0: No ICE debug
 * @retval     1: ICE debug
 *
 */
__asm int32_t SH_DoCommand(int32_t n32In_R0, int32_t n32In_R1, int32_t *pn32Out_R0)
{
    BKPT   0xAB          //; Wait ICE or HardFault
    //; ICE will step over BKPT directly
    //; HardFault will step BKPT and the next line
    B      SH_ICE

SH_HardFault             //; Captured by HardFault
    MOVS   R0, #0        //; Set return value to 0
    BX     lr            //; Return

SH_ICE                   //; Captured by ICE
    //; Save return value
    CMP    R2, #0
    BEQ    SH_End
    STR    R0, [R2]      //; Save the return value to *pn32Out_R0

SH_End
    MOVS   R0, #1        //; Set return value to 1
    BX     lr            //; Return
}
#endif

#else

# if defined(__ICCARM__)

/**
 * @brief    This HardFault handler is implemented to show r0, r1, r2, r3, r12, lr, pc, psr
 *
 * @param    None
 *
 * @returns  None
 *
 * @details  This function is implement to print r0, r1, r2, r3, r12, lr, pc, psr.
 *
 */
void HardFault_Handler(void)
{
    asm("MOVS    r0, #4                        \n"
        "MOV     r1, LR                        \n"
        "TST     r0, r1                        \n"
        "BEQ     Stack_Use_MSP                 \n"
        "MRS     R0, PSP                       \n" //; stack use PSP
        "B       Get_LR_and_Branch             \n"
        "Stack_Use_MSP:                        \n"
        "MRS     R0, MSP                       \n" //; stack use MSP
        "Get_LR_and_Branch:                    \n"
        "MOV     R1, LR                        \n" //; LR current value
        "B Hard_Fault_Handler                  \n"
       );

    while(1);
}

# else

/**
 * @brief    This HardFault handler is implemented to show r0, r1, r2, r3, r12, lr, pc, psr
 *
 * @param    None
 *
 * @returns  None
 *
 * @details  This function is implement to print r0, r1, r2, r3, r12, lr, pc, psr
 *
 */
// #if !(defined(__ICCARM__) && (__VER__ >= 6100100))
 #if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6100100)
void HardFault_Handler(void)
{
    __asm("MOVS    r0, #4                        \n"
        "MOV     r1, LR                        \n"
        "TST     r0, r1                        \n"
        "BEQ     Stack_Use_MSP                 \n"
        "MRS     R0, PSP                       \n" //; stack use PSP
        "B       Get_LR_and_Branch             \n"
        "Stack_Use_MSP:                        \n"
        "MRS     R0, MSP                       \n" //; stack use MSP
        "Get_LR_and_Branch:                    \n"
        "MOV     R1, LR                        \n" //; LR current value
        "B Hard_Fault_Handler                  \n"
       );
}
#else
__asm int32_t HardFault_Handler(void)
{
    MOVS    r0, #4
    MOV     r1, LR
    TST     r0, r1
    BEQ     Stack_Use_MSP
    MRS     R0, PSP ;stack use PSP
    B       Get_LR_and_Branch
Stack_Use_MSP
    MRS     R0, MSP ; stack use MSP
Get_LR_and_Branch
    MOV     R1, LR ; LR current value
    LDR     R2,=__cpp(Hard_Fault_Handler)
    BX      R2
}
#endif

#endif

#endif


/**
  * @brief  Write a char to UART.
  * @param  ch The character sent to UART.
  * @return None
  */

void SendChar_ToUART(int ch)
{
    while(UART_IsTxFifoFull(DEBUG_PORT));
    DEBUG_PORT->RBR_THR_DLL = ch;
}


/**
  * @brief  Write a char to debug console.
  * @param  ch The character sent to debug console
  * @return None
  */

__WEAK void SendChar(int ch)
{
#if defined(DEBUG_ENABLE_SEMIHOST)
    g_buf[g_buf_len++] = ch;
    g_buf[g_buf_len] = '\0';
    if(g_buf_len + 1 >= sizeof(g_buf) || ch == '\n' || ch == '\0')
    {
        /* Send the char */

        if(SH_DoCommand(0x04, (int)g_buf, NULL) != 0)
        {
            g_buf_len = 0;
            return;
        }
        else
        {
            int i;

            for(i=0;i<g_buf_len;i++)
                SendChar_ToUART(g_buf[i]);
            g_buf_len = 0;
        }
    }
#endif

#if CONFIG_RTT_LOG_ENABLE
    SEGGER_RTT_PutCharSkipNoLock(0, ch);
#endif

#if CONFIG_UART_LOG_ENABLE
    SendChar_ToUART(ch);
#endif
}


/**
  * @brief  Read a char from debug console.
  * @param  None
  * @return Received character from debug console
  * @note   This API waits until UART debug port or semihost input a character
  */

__WEAK char GetChar(void)
{
#if defined(DEBUG_ENABLE_SEMIHOST)
#if defined ( __CC_ARM   )
    int nRet;
    while(SH_DoCommand(0x101, 0, &nRet) != 0)
    {
        if(nRet != 0)
        {
            SH_DoCommand(0x07, 0, &nRet);
            return (char)nRet;
        }
    }
#else
    int nRet;
    while(SH_DoCommand(0x7, 0, &nRet) != 0)
    {
        if(nRet != 0)
            return (char)nRet;
    }
#endif
#endif

#if CONFIG_UART_LOG_ENABLE
        while (1)
        {
            if((DEBUG_PORT->USR & UART_USR_RFNE_Msk))
            {
                return (DEBUG_PORT->RBR_THR_DLL);
            }
        }
#elif CONFIG_RTT_LOG_ENABLE
    return SEGGER_RTT_WaitKey();
#else
    return 0;
#endif
}


/**
  * @brief  Check whether UART receive FIFO is empty or not.
  * @param  None
  * @return UART Rx FIFO empty status
  * @retval 1 Indicates at least one character is available in UART Rx FIFO
  * @retval 0 UART Rx FIFO is empty
  */
int kbhit(void)
{
    return (DEBUG_PORT->USR & UART_USR_RFNE_Msk);
}

/**
  * @brief  Check whether UART transmit FIFO is empty or not.
  * @param  None
  * @return UART Tx FIFO empty status
  * @retval 1 UART Tx FIFO is empty
  * @retval 0 UART Tx FIFO is not empty
  */
int IsDebugFifoEmpty(void)
{
    return (DEBUG_PORT->USR & UART_USR_TFE_Msk) ? 1 : 0;
}

/*---------------------------------------------------------------------------------------------------------*/
/* C library retargetting                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
void _ttywrch(int ch)
{
  SendChar(ch);
  return;
}

int fputc(int ch, FILE *f)
{
  SendChar(ch);
  return ch;
}

int fgetc(FILE *f) {
   return (GetChar());
}


int ferror(FILE *f) {
  return EOF;
}

#ifdef DEBUG_ENABLE_SEMIHOST
# ifdef __ICCARM__
void __exit(int return_code) {

    /* Check if link with ICE */

    if(SH_DoCommand(0x18, 0x20026, NULL) == 0)
    {
        /* Make sure all message is print out */

        while(IsDebugFifoEmpty() == 0);
    }
label:  goto label;  /* endless loop */
}
# else
void _sys_exit(int return_code) {

    /* Check if link with ICE */
    if(SH_DoCommand(0x18, 0x20026, NULL) == 0)
    {
        /* Make sure all message is print out */
        while(IsDebugFifoEmpty() == 0);
    }
label:  goto label;  /* endless loop */
}
# endif
#endif


/*
 * Hook functions to implement the standard C library Assert APIs (assert.h)
 * - assert() is defined as abort() by default
 * - Define __ASSERT_MSG to C/C++ compiler, and then assert() would be defined
 *   as __aeabi_assert() for more message print
 */
void __aeabi_assert(const char *expr, const char *file, int line)
{
    SYS_PRINT("ASSERTION FAIL! expr: \"%s\" (%s: %d)\n", expr, file, line);

    /* Infinite loop */
    while(1);
}

void abort(void)
{
    SYS_PRINT("!!!!!!!!!!!!!!!!!! Aborted !!!!!!!!!!!!!!!!!\n");

    /* Infinite loop */
    while(1);
}
