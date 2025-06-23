/**************************************************************************//**
* @file     pan_hal_uart.c
* @version  V0.0.0
* $Revision: 1 $
* $Date:    23/09/10 $
* @brief    Panchip series UART (Universal Asynchronous Receiver-Transmitter) HAL source file.
* @note
* Copyright (C) 2023 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include "pan_hal.h"

#if (BLE_EN == 1)
#define PAN_HAL_BLE_IRQ_SPLIT_CALLBACK      1
#endif

HAL_UART_HandleTypeDef UART_Handle_Array[PAN_HAL_UART_INST_COUNT] =
{
    {
        .pUartx = UART0,
        .initObj = {0},
        .interruptObj =
        {
            .txTrigLevel = HAL_UART_TX_FIFO_HALF_FULL,
            .rxTrigLevel = HAL_UART_RX_FIFO_HALF_FULL,
            .IrqPriority = 2,
            .IRQn = UART0_IRQn,
        },
        .pTxBuffPtr = NULL,
        .txXferSize = 0,
        .txXferCount = 0,
        .pRxBuffPtr = NULL,
        .rxXferSize = 0,
        .rxXferCount = 0,
        .isTxBusy = false,
        .isRxBusy = false,
        .dmaSrc = DMAC_Peripheral_UART0_Rx,
        .dmaDst = DMAC_Peripheral_UART0_Tx,
    },
    {
        .pUartx = UART1,
        .initObj = {0},
        .interruptObj =
        {
            .txTrigLevel = HAL_UART_TX_FIFO_HALF_FULL,
            .rxTrigLevel = HAL_UART_RX_FIFO_HALF_FULL,
            .IrqPriority = 2,
            .IRQn = UART1_IRQn,
        },
        .pTxBuffPtr = NULL,
        .txXferSize = 0,
        .txXferCount = 0,
        .pRxBuffPtr = NULL,
        .rxXferSize = 0,
        .rxXferCount = 0,
        .isTxBusy = false,
        .isRxBusy = false,
        .dmaSrc = DMAC_Peripheral_UART1_Rx,
        .dmaDst = DMAC_Peripheral_UART1_Tx,
    }
};

HAL_Status HAL_UART_Init(HAL_UART_HandleTypeDef *pUart)
{
    if (pUart == NULL)
    {
        return HAL_ERROR;
    }

    uint32_t tmpreg = 0x00;
    uint32_t integerdivider = 0x00;
    uint32_t fractionaldivider = 0x00;
    uint64_t apbclock = 0x00;

    if(pUart->pUartx == UART0)
    {
        CLK_APB1PeriphClockCmd(CLK_APB1Periph_UART0, ENABLE);
    }
    else if(pUart->pUartx == UART1)
    {
        CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);
    }

    /*---------------------------- UART BRR Configuration -----------------------*/
    /* Configure the UART Baud Rate */
    apbclock = CLK_GetPeripheralFreq((void *)pUart->pUartx);
    /* Unlock to enable write & read divisor register */
    pUart->pUartx->LCR |= UART_LCR_DLAB_Msk;
    /* Determine the integer part baud_rate_divisor =  PCLK*100 / (16*required_baud_rate) */
    integerdivider = ((25 * apbclock) / (4 * (pUart->initObj.baudRate)));

    // Too high baudrate (too small divider) would cause DLL/DLH be all 0 which means UART disabled,
    // thus return false if this happens.
    if (integerdivider < 100)
        return HAL_ERROR;

    tmpreg = (integerdivider / 100);
    pUart->pUartx->RBR_THR_DLL = tmpreg & 0xFF;
    pUart->pUartx->IER_DLH = (tmpreg & 0xFF00) >> 8;

    /* Determine the fractional part */
    fractionaldivider = integerdivider - (100 * tmpreg);

    /* Implement the fractional part in the register */
    pUart->pUartx->DLF = ((((fractionaldivider * 16) + 50) / 100));
    pUart->pUartx->LCR &= ~UART_LCR_DLAB_Msk;

    /*---------------------------- UART Line Configuration -----------------------*/
    tmpreg = pUart->pUartx->LCR;
    tmpreg &= ~(UART_LCR_SP_Msk | UART_LCR_EPS_Msk | UART_LCR_PEN_Msk | UART_LCR_STOP_Msk | UART_LCR_DLS_Msk);
    tmpreg |= (pUart->initObj.format);
    pUart->pUartx->LCR = tmpreg;

    /*-------------------------------enable fifo----------------------------------*/
    UART_EnableFifo(pUart->pUartx);

    return HAL_OK;
}

HAL_Status HAL_UART_SendData(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t size, uint32_t timeout)
{
    if (pUart->isTxBusy)
    {
        return HAL_BUSY;
    }

    if ((pUart == NULL) || (pBuf == NULL) || (size == 0U))
    {
        return HAL_ERROR;
    }

    uint32_t timeout_tick = HAL_TimeConvMsToTick(timeout);
    uint32_t uptime_ref = HAL_TimeGetCurrTick();

    pUart->isTxBusy = true;
    pUart->pTxBuffPtr = pBuf;
    pUart->txXferSize = size;
    pUart->txXferCount = 0;

    while (pUart->txXferCount < pUart->txXferSize)
    {
        // Wait until Transmitter Shift Register and Tx FIFO are both empty
        while (!(UART_GetLineStatus(pUart->pUartx) & UART_LSR_TEMT_Msk))
        {
            if (timeout_tick != HAL_TIME_FOREVER)
            {
                if (HAL_TimeGetCurrTick() - uptime_ref >= timeout_tick)
                {
                    pUart->isTxBusy = false;
                    return HAL_TIMEOUT;
                }
            }
        }
        // Put a byte to Tx FIFO
        UART_SendData(pUart->pUartx, pUart->pTxBuffPtr[pUart->txXferCount++]);
    }

    pUart->isTxBusy = false;

    return HAL_OK;
}

HAL_Status HAL_UART_ReceiveData(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t size, uint32_t timeout)
{
    if (pUart->isRxBusy)
    {
        return HAL_BUSY;
    }

    if ((pUart == NULL) || (pBuf == NULL) || (size == 0U))
    {
        return HAL_ERROR;
    }

    uint32_t timeout_tick = HAL_TimeConvMsToTick(timeout);
    uint32_t uptime_ref = HAL_TimeGetCurrTick();

    pUart->isRxBusy = true;
    pUart->pRxBuffPtr = pBuf;
    pUart->rxXferSize = size;
    pUart->rxXferCount = 0;

    while (pUart->rxXferCount < pUart->rxXferSize)
    {
        // Check if there is at least one byte in Rx Data Register or Rx FIFO
        if (UART_GetLineStatus(pUart->pUartx) & UART_LSR_DR_Msk)
        {
            // Get a byte from Rx FIFO
            pUart->pRxBuffPtr[pUart->rxXferCount++] = UART_ReceiveData(pUart->pUartx);
        }
        if (timeout_tick != HAL_TIME_FOREVER)
        {
            if (HAL_TimeGetCurrTick() - uptime_ref >= timeout_tick)
            {
                pUart->isRxBusy = false;
                return HAL_TIMEOUT;
            }
        }
    }

    pUart->isRxBusy = false;

    return HAL_OK;
}

#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
#include "pan_ble_stack.h"
void ble_irq_split_callback(void)
{
    HAL_UART_HandleTypeDef *pUart;

    for (size_t i = 0; i < PAN_HAL_UART_INST_COUNT; i++)
    {
        pUart = &UART_Handle_Array[i];
        if (pUart->isRxBusy)
        {
            __disable_irq(); // To avoid swiching to the possible higher priority UART ISR
            if (pUart->interruptObj.continuousRxMode)
            {
                while (UART_GetRxFifoLevel(pUart->pUartx) > 1)
                {
                    if (pUart->rxXferCount < pUart->rxXferSize)
                    {
                        pUart->pRxBuffPtr[pUart->rxXferCount++] = UART_ReceiveData(pUart->pUartx);
                    }
                    else
                    {
                        // Set UART pending irq manually to trigger uart rx continuous buff full error callback
                        NVIC_SetPendingIRQ(pUart->interruptObj.IRQn);
                        break;
                    }
                }
            }
            else
            {
                while (!UART_IsRxFifoEmpty(pUart->pUartx))
                {
                    if (pUart->rxXferCount < pUart->rxXferSize)
                    {
                        pUart->pRxBuffPtr[pUart->rxXferCount++] = UART_ReceiveData(pUart->pUartx);
                    }
                    else
                    {
                        break;
                    }
                }

                if (pUart->rxXferCount == pUart->rxXferSize)
                {
                    // Set UART pending irq manually to trigger uart rx finish callback
                    NVIC_SetPendingIRQ(pUart->interruptObj.IRQn);
                }
            }
            __enable_irq();
        }
    }
}
#endif // PAN_HAL_BLE_IRQ_SPLIT_CALLBACK

HAL_Status HAL_UART_Init_INT(HAL_UART_HandleTypeDef *pUart)
{
    if (pUart == NULL)
    {
        return HAL_ERROR;
    }

#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
    pan_misc_register_app_irq_handler_cb(ble_irq_split_callback);
#endif // PAN_HAL_BLE_IRQ_SPLIT_CALLBACK

    // Flush UART FIFOs
    UART_ResetTxFifo(pUart->pUartx);
    UART_ResetRxFifo(pUart->pUartx);

    // Set UART IRQ trigger level
    UART_SetTxTrigger(pUart->pUartx, (UART_TxTriggerDef)pUart->interruptObj.txTrigLevel);
    UART_SetRxTrigger(pUart->pUartx, (UART_RxTriggerDef)pUart->interruptObj.rxTrigLevel);

    // Enable THRE Interrupt Mode
    UART_EnablePtime(pUart->pUartx);

    // Set UART IRQ Priority and enable UART IRQ
    NVIC_SetPriority(pUart->interruptObj.IRQn, pUart->interruptObj.IrqPriority);
    NVIC_EnableIRQ(pUart->interruptObj.IRQn);

    return HAL_OK;
}

HAL_Status HAL_UART_SendData_INT(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t size)
{
    if (pUart->isTxBusy)
    {
        return HAL_BUSY;
    }

    if ((pUart == NULL) || (pBuf == NULL) || (size == 0U))
    {
        return HAL_ERROR;
    }

    pUart->isTxBusy = true;
    pUart->pTxBuffPtr = pBuf;
    pUart->txXferSize = size;
    pUart->txXferCount = 0;

    // Enable Transmit Trigger interupt to start sending
    UART_EnableIrq(pUart->pUartx, UART_IRQ_THR_EMPTY);

    return HAL_OK;
}

HAL_Status HAL_UART_ReceiveData_INT(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t size)
{
    if (pUart->isRxBusy)
    {
        return HAL_BUSY;
    }

    if ((pUart == NULL) || (pBuf == NULL) || (size == 0U))
    {
        return HAL_ERROR;
    }

    pUart->isRxBusy = true;
    pUart->pRxBuffPtr = pBuf;
    pUart->rxXferSize = size;
    pUart->rxXferCount = 0;

    // Enable Data Available and Rx Line Status interrupt to start receiving
    UART_EnableIrq(pUart->pUartx, UART_IRQ_RECV_DATA_AVL);
    UART_EnableIrq(pUart->pUartx, UART_IRQ_LINE_STATUS);

    return HAL_OK;
}

HAL_Status HAL_UART_ReceiveDataContinuously_INT(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t bufSize)
{
    if (pUart->isRxBusy)
    {
        return HAL_BUSY;
    }

    if ((pUart == NULL) || (pBuf == NULL) || (bufSize == 0U))
    {
        return HAL_ERROR;
    }

    // The continuous Rx mode does not support Rx FIFO trigger level set to 1 char
    if ((UART_RxTriggerDef)pUart->interruptObj.rxTrigLevel == UART_RX_FIFO_ONE_CHAR)
    {
        return HAL_ERROR;
    }

    pUart->interruptObj.continuousRxMode = true;
    pUart->isRxBusy = true;
    pUart->pRxBuffPtr = pBuf;
    pUart->rxXferSize = bufSize;
    pUart->rxXferCount = 0;

    // Enable Data Available and Rx Line Status interrupt to start receiving
    UART_EnableIrq(pUart->pUartx, UART_IRQ_RECV_DATA_AVL);
    UART_EnableIrq(pUart->pUartx, UART_IRQ_LINE_STATUS);

    return HAL_OK;
}

HAL_Status HAL_UART_SendDataAbort_INT(HAL_UART_HandleTypeDef *pUart)
{
    if (pUart == NULL)
    {
        return HAL_ERROR;
    }

    // Disable Transmit Trigger interupt to abort sending
    UART_DisableIrq(pUart->pUartx, UART_IRQ_THR_EMPTY);

    // Flush Tx FIFO
    UART_ResetTxFifo(pUart->pUartx);

    pUart->isTxBusy = false;

    return HAL_OK;
}

HAL_Status HAL_UART_ReceiveDataAbort_INT(HAL_UART_HandleTypeDef *pUart)
{
    if (pUart == NULL)
    {
        return HAL_ERROR;
    }

    // Disable Data Available and Rx Line Status interrupt to abort receiving
    UART_DisableIrq(pUart->pUartx, UART_IRQ_RECV_DATA_AVL);
    UART_DisableIrq(pUart->pUartx, UART_IRQ_LINE_STATUS);

    // Flush Rx FIFO
    UART_ResetRxFifo(pUart->pUartx);

    pUart->isRxBusy = false;
    pUart->interruptObj.continuousRxMode = false;

    return HAL_OK;
}

HAL_Status HAL_UART_Init_DMA(HAL_UART_HandleTypeDef *pUart, HAL_UART_DmaDirOpt dmaDir, UART_CallbackFunc callback)
{
    uint32_t dmaCh;

    if ((pUart == NULL) || (callback == NULL))
    {
        return HAL_ERROR;
    }

    if (dmaDir == HAL_UART_DMA_TX)
    {
        // Configure UART Tx FIFO
        UART_ResetTxFifo(pUart->pUartx);
        UART_SetTxTrigger(pUart->pUartx, UART_TX_FIFO_HALF_FULL);
        UART_EnablePtime(pUart->pUartx); //Enable Programmable THRE Interrupt Mode

        /* Get free DMA channel */
        dmaCh = DMAC_AcquireChannel(DMA);
        if (dmaCh == DMA_INVLID_CHANNEL)
        {
            return HAL_ERROR;
        }

        pUart->txDmaCh = dmaCh;
        DMAC_Channel_Array[dmaCh].ConfigTmp = dma_mem2uart_config;
        DMAC_Channel_Array[dmaCh].ConfigTmp.PeripheralDst = pUart->dmaDst;
    }
    else if (dmaDir == HAL_UART_DMA_RX)
    {
        // Configure UART Rx FIFO
        UART_ResetRxFifo(pUart->pUartx);
        UART_SetRxTrigger(pUart->pUartx, UART_RX_FIFO_HALF_FULL);

        /* Get free DMA channel */
        dmaCh = DMAC_AcquireChannel(DMA);
        if (dmaCh == DMA_INVLID_CHANNEL)
        {
            return HAL_ERROR;
        }

        pUart->rxDmaCh = dmaCh;
        DMAC_Channel_Array[dmaCh].ConfigTmp = dma_uart2mem_config;
        DMAC_Channel_Array[dmaCh].ConfigTmp.PeripheralSrc = pUart->dmaSrc;
    }

    /* Enable DMA transfer interrupt */
    DMAC_ClrIntFlagMsk(DMA, dmaCh, DMAC_FLAG_INDEX_TFR);
//    DMAC_ClrIntFlagMsk(DMA, dmaCh, DMAC_FLAG_INDEX_ERR);

    DMAC_Channel_Array[dmaCh].periph = pUart;
    DMAC_Channel_Array[dmaCh].CallbackUart = callback;
    DMAC_Channel_Array[dmaCh].PeriMode = DMAC_Peri_UART;
    DMAC_SetChannelConfig(DMA, dmaCh, &DMAC_Channel_Array[dmaCh].ConfigTmp);

    return HAL_OK;
}

HAL_Status HAL_UART_SendData_DMA(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t size)
{
    if (pUart->isTxBusy)
    {
        return HAL_BUSY;
    }

    if ((pUart == NULL) || (pBuf == NULL) || (size == 0U))
    {
        return HAL_ERROR;
    }

    pUart->isTxBusy = true;
    DMAC_Channel_Array[pUart->txDmaCh].pBuffPtr = (uint32_t*)pBuf;
    DMAC_Channel_Array[pUart->txDmaCh].XferSize = size;
    DMAC_Channel_Array[pUart->txDmaCh].XferCount = 0;

    if (size < 1024)
    {
        DMAC_StartChannel(DMA, pUart->txDmaCh, pBuf, (void *)&(pUart->pUartx->RBR_THR_DLL), size);
    }
    else
    {
        DMAC_StartChannel(DMA, pUart->txDmaCh, pBuf, (void *)&(pUart->pUartx->RBR_THR_DLL), 1023);
    }

    return HAL_OK;
}

HAL_Status HAL_UART_ReceiveData_DMA(HAL_UART_HandleTypeDef *pUart, uint8_t *pBuf, uint16_t size)
{
    if (pUart->isRxBusy)
    {
        return HAL_BUSY;
    }

    if ((pUart == NULL) || (pBuf == NULL) || (size == 0U))
    {
        return HAL_ERROR;
    }

    pUart->isRxBusy = true;
    DMAC_Channel_Array[pUart->rxDmaCh].pBuffPtr = (uint32_t*)pBuf;
    DMAC_Channel_Array[pUart->rxDmaCh].XferSize = size;
    DMAC_Channel_Array[pUart->rxDmaCh].XferCount = 0;

    if (size < 1024)
    {
        DMAC_StartChannel(DMA, pUart->rxDmaCh, (void*)&(pUart->pUartx->RBR_THR_DLL), pBuf, size);
    }
    else
    {
        DMAC_StartChannel(DMA, pUart->rxDmaCh, (void*)&(pUart->pUartx->RBR_THR_DLL), pBuf, 1023);
    }

    return HAL_OK;
}

HAL_Status HAL_UART_SendDataAbort_DMA(HAL_UART_HandleTypeDef *pUart)
{
    if (pUart == NULL)
    {
        return HAL_ERROR;
    }

    // Stop current DMA channel
    DMAC_StopChannel(DMA, pUart->txDmaCh);

    // Flush Tx FIFO
    UART_ResetTxFifo(pUart->pUartx);

    pUart->isTxBusy = false;

    return HAL_OK;
}

HAL_Status HAL_UART_ReceiveDataAbort_DMA(HAL_UART_HandleTypeDef *pUart)
{
    if (pUart == NULL)
    {
        return HAL_ERROR;
    }

    // Stop current DMA channel
    DMAC_StopChannel(DMA, pUart->rxDmaCh);

    // Flush Rx FIFO
    UART_ResetRxFifo(pUart->pUartx);

    pUart->isRxBusy = false;

    return HAL_OK;
}

static void UART_HandleLineStatus(HAL_UART_HandleTypeDef *pUart)
{
    uint32_t lineStatus = UART_GetLineStatus(pUart->pUartx);

    if (pUart->isRxBusy)
    {
        // Report Rx error event
        if (pUart->interruptObj.callbackFunc != NULL)
        {
            if (lineStatus & UART_LINE_PARITY_ERR)
            {
                pUart->interruptObj.callbackFunc(pUart, HAL_UART_EVT_RX_ERR_PARITY, pUart->pRxBuffPtr, pUart->rxXferCount);
            }
            if (lineStatus & UART_LINE_FRAME_ERR)
            {
                pUart->interruptObj.callbackFunc(pUart, HAL_UART_EVT_RX_ERR_FRAME, pUart->pRxBuffPtr, pUart->rxXferCount);
            }
            if (lineStatus & UART_LINE_OVERRUN_ERR)
            {
                pUart->interruptObj.callbackFunc(pUart, HAL_UART_EVT_RX_ERR_OVERRUN, pUart->pRxBuffPtr, pUart->rxXferCount);
            }
        }
    }
}

static void UART_HandleReceivedData(HAL_UART_HandleTypeDef *pUart, UART_EventDef flag)
{
    if (pUart->isRxBusy)
    {
        if (pUart->interruptObj.continuousRxMode)
        {
            if (flag != UART_EVENT_TIMEOUT)
            {
#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
                __disable_irq(); // To avoid swiching to higher priority LL IRQ
#endif
                // Read data from UART Rx FIFO and remains 1 byte in it
                while (UART_GetRxFifoLevel(pUart->pUartx) > 1)
                {
                    if (pUart->rxXferCount < pUart->rxXferSize)
                    {
                        pUart->pRxBuffPtr[pUart->rxXferCount++] = UART_ReceiveData(pUart->pUartx);
                    }
                    else
                    {
                        break;
                    }
                }
#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
                __enable_irq();
#endif
            }
            else
            {
#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
                __disable_irq(); // To avoid swiching to higher priority LL IRQ
#endif
                // Read all data from UART Rx FIFO due to timeout occurred
                while (!UART_IsRxFifoEmpty(pUart->pUartx))
                {
                    if (pUart->rxXferCount < pUart->rxXferSize)
                    {
                        pUart->pRxBuffPtr[pUart->rxXferCount++] = UART_ReceiveData(pUart->pUartx);
                    }
                    else
                    {
                        break;
                    }
                }
#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
                __enable_irq();
#endif
                // End current rx flow and trigger Continuous Rx Timeout callback if Rx FIFO is already empty
                if (UART_IsRxFifoEmpty(pUart->pUartx))
                {
                    // Disable Data Available and Rx Line Status interrupt to abort receiving
                    UART_DisableIrq(pUart->pUartx, UART_IRQ_RECV_DATA_AVL);
                    UART_DisableIrq(pUart->pUartx, UART_IRQ_LINE_STATUS);
#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
                    __disable_irq(); // To avoid swiching to higher priority LL IRQ
#endif
                    pUart->isRxBusy = false;
                    pUart->interruptObj.continuousRxMode = false;
#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
                    __enable_irq();
#endif
                    // Trigger Continuous Rx Timeout callback
                    if (pUart->interruptObj.callbackFunc != NULL)
                    {
                        pUart->interruptObj.callbackFunc(pUart, HAL_UART_EVT_CONTI_RX_TIMEOUT, pUart->pRxBuffPtr, pUart->rxXferCount);
                    }
                }
            }
            if (pUart->rxXferCount >= pUart->rxXferSize)
            {
                // Trigger Rx Buffer Full callback
                if (pUart->interruptObj.callbackFunc != NULL)
                {
                    pUart->interruptObj.callbackFunc(pUart, HAL_UART_EVT_CONTI_RX_BUFF_FULL, pUart->pRxBuffPtr, pUart->rxXferCount);
                }
            }
        }
        else // Normal Rx Mode (Not Continuous Rx Mode)
        {
#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
            __disable_irq(); // To avoid swiching to higher priority LL IRQ
#endif
            while (!UART_IsRxFifoEmpty(pUart->pUartx))
            {
                if (pUart->rxXferCount < pUart->rxXferSize)
                {
                    pUart->pRxBuffPtr[pUart->rxXferCount++] = UART_ReceiveData(pUart->pUartx);
                }
                else
                {
                    break;
                }
            }
#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
            __enable_irq();
#endif
            if (pUart->rxXferCount == pUart->rxXferSize)
            {
                // Disable Data Available and Rx Line Status interrupt to abort receiving
                UART_DisableIrq(pUart->pUartx, UART_IRQ_RECV_DATA_AVL);
                UART_DisableIrq(pUart->pUartx, UART_IRQ_LINE_STATUS);
#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
                __disable_irq(); // To avoid swiching to higher priority LL IRQ
#endif
                pUart->isRxBusy = false;
#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
                __enable_irq();
#endif
                // Report Rx finish event
                if (pUart->interruptObj.callbackFunc != NULL)
                {
                    pUart->interruptObj.callbackFunc(pUart, HAL_UART_EVT_RX_FINISH, pUart->pRxBuffPtr, pUart->rxXferCount);
                }
            }
        }
    }
}

static void UART_HandleTransmittingData(HAL_UART_HandleTypeDef *pUart)
{
    if (pUart->isTxBusy)
    {
        while (!UART_IsTxFifoFull(pUart->pUartx))
        {
            if (pUart->txXferCount < pUart->txXferSize)
            {
                UART_SendData(pUart->pUartx, pUart->pTxBuffPtr[pUart->txXferCount++]);
            }
            else
            {
                break;
            }
        }

        if (pUart->txXferCount == pUart->txXferSize)
        {
            // Disable Transmit Trigger interupt after transmitting done
            UART_DisableIrq(pUart->pUartx, UART_IRQ_THR_EMPTY);
            pUart->isTxBusy = false;
            // Report Tx finish event
            if (pUart->interruptObj.callbackFunc != NULL)
            {
                pUart->interruptObj.callbackFunc(pUart, HAL_UART_EVT_TX_FINISH, pUart->pTxBuffPtr, pUart->txXferCount);
            }
        }
    }
}

static void UART_HandleProc(HAL_UART_HandleTypeDef *pUart)
{
    UART_EventDef event = UART_GetActiveEvent(pUart->pUartx);

    switch (event)
    {
    case UART_EVENT_LINE:
        UART_HandleLineStatus(pUart);
        break;
    case UART_EVENT_DATA:
        UART_HandleReceivedData(pUart, UART_EVENT_DATA);
        break;
    case UART_EVENT_TIMEOUT:
        UART_HandleReceivedData(pUart, UART_EVENT_TIMEOUT);
        break;
    case UART_EVENT_THR_EMPTY:
        UART_HandleTransmittingData(pUart);
        break;
    case UART_EVENT_NONE:
#if PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
        UART_HandleReceivedData(pUart, UART_EVENT_NONE);
#endif // PAN_HAL_BLE_IRQ_SPLIT_CALLBACK
        break;
    default:
        break;
    }
}

__WEAK void UART0_IRQHandlerOverlay(void)
{
    UART_HandleProc(&UART_Handle_Array[0]);
}

__WEAK void UART1_IRQHandlerOverlay(void)
{
    UART_HandleProc(&UART_Handle_Array[1]);
}

void UART0_IRQHandler(void)
{
    PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_UART0_IRQ, 1);

    UART0_IRQHandlerOverlay();

    PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_UART0_IRQ, 0);
}

void UART1_IRQHandler(void)
{
    PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_UART1_IRQ, 1);

    UART1_IRQHandlerOverlay();

    PAN_IO_TIMING_TRACK_LEVEL(CONFIG_TRACK_PIN_UART1_IRQ, 0);
}
