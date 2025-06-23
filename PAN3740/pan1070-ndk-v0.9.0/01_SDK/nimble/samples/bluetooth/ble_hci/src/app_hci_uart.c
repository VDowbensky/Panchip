/**
 *******************************************************************************
 * @FileName  : app_hci_uart.c
 * @Author    : GaoQiu
 * @CreateDate: 2020-05-18
 * @Copyright : Copyright(C) GaoQiu
 *              All Rights Reserved.
 *******************************************************************************
 *
 * The information contained herein is confidential and proprietary property of
 * GaoQiu and is available under the terms of Commercial License Agreement
 * between GaoQiu and the licensee in separate contract or the terms described
 * here-in.
 *
 * This heading MUST NOT be removed from this file.
 *
 * Licensees are granted free, non-transferable use of the information in this
 * file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************
 */
#include "app.h"

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sysinit/sysinit.h>
#include <syscfg/syscfg.h>
#include "os/os_mbuf.h"
#include "os/os_mempool.h"
#include "nimble/transport.h"
#include "nimble/transport/hci_h4.h"
#include "app.h"
#include "pan_hal.h"
#include "PanSeries.h"

/*******************************************************************************
 * Macro
 ******************************************************************************/
/**@brief HCI UART Baudrate config. */
#define HCI_UART_INSTANCE	UART1_OBJ
#define HCI_UART_BAUDRATE   CONFIG_HCI_UART_BAUDRATE

/**@brief HCI Tx queue depth config. */
#define TX_Q_NUM   (MYNEWT_VAL(BLE_TRANSPORT_ACL_FROM_LL_COUNT) + \
                    MYNEWT_VAL(BLE_TRANSPORT_EVT_COUNT) + \
                    MYNEWT_VAL(BLE_TRANSPORT_EVT_DISCARDABLE_COUNT))

/*******************************************************************************
 * Variable Define & Declaration
 ******************************************************************************/

/**@brief hci uart tx structure */
struct hci_uart_tx {
    uint8_t  type;
    uint8_t  sent_type;
    uint16_t len;
    uint16_t idx;

    struct os_mbuf *om;
    uint8_t *buf;

    STAILQ_ENTRY(hci_uart_tx) tx_q_next;
};

/**@brief hci uart tx queue define. */
static STAILQ_HEAD(, hci_uart_tx) tx_q;

/**@brief hci uart tx memory pool. */
static struct os_mempool pool_tx_q;
static uint8_t pool_tx_q_buf[ OS_MEMPOOL_BYTES(TX_Q_NUM, sizeof(struct hci_uart_tx)) ];

/**@brief HCI H4 statement machine. */
struct hci_h4_sm hci_uart_h4sm;

/**@brief UART TX/RX buffer */
static uint8_t uart_tx_buf[4 + 255] = {0};
static uint8_t uart_rx_buf[4 + 255] = {0};

/*******************************************************************************
 * Function Declaration
 ******************************************************************************/
void hci_uart_start_tx(void);

/*******************************************************************************
 * Function Define
 ******************************************************************************/
int ble_transport_to_hs_evt_impl(void *buf)
{
    struct hci_uart_tx *txe;
    os_sr_t sr;

    txe = os_memblock_get(&pool_tx_q);
    if (!txe) {
        APP_LOG_WRN("ble_transport_to_hs_evt_impl: get tx buffer failed\r\n");
        return -ENOMEM;
    }

    txe->type = HCI_H4_EVT;
    txe->sent_type = 0;
    txe->len = 2 + ((uint8_t *)buf)[1];
    txe->buf = buf;
    txe->idx = 0;
    txe->om = NULL;

    /* data enqueue */
    OS_ENTER_CRITICAL(sr);
    STAILQ_INSERT_TAIL(&tx_q, txe, tx_q_next);
    OS_EXIT_CRITICAL(sr);

    /* data tx */
    hci_uart_start_tx();

    return 0;
}

int ble_transport_to_hs_acl_impl(struct os_mbuf *om)
{
    struct hci_uart_tx *txe;
    os_sr_t sr;

    txe = os_memblock_get(&pool_tx_q);
    if(!txe){
        APP_LOG_WRN("ble_transport_to_hs_acl_impl: get tx buffer failed\r\n");
        return -ENOMEM;
    }

    txe->type = HCI_H4_ACL;
    txe->sent_type = 0;
    txe->len = OS_MBUF_PKTLEN(om);
    txe->idx = 0;
    txe->buf = NULL;
    txe->om = om;

    /* data enqueue */
    OS_ENTER_CRITICAL(sr);
    STAILQ_INSERT_TAIL(&tx_q, txe, tx_q_next);
    OS_EXIT_CRITICAL(sr);

    /* data tx */
    hci_uart_start_tx();

    return 0;
}

static int hci_uart_frame_cb(uint8_t pkt_type, void *data)
{
    switch(pkt_type) {
    case HCI_H4_CMD:
        return ble_transport_to_ll_cmd(data);
    case HCI_H4_ACL:
        return ble_transport_to_ll_acl(data);
    default:
        APP_LOG_WRN("hci_uart_frame_cb: invalid hci frame\r\n");
    }
    return -1;
}

static int hci_uart_rx(uint8_t *pdata, uint32_t len)
{
	int rc = hci_h4_sm_rx(&hci_uart_h4sm, pdata, len);
    return rc;
}

static int hci_uart_dma_tx(struct hci_uart_tx *tx)
{
	if(tx == NULL || (tx->buf == NULL && tx->om == NULL)) {
		return -1;
	}

	if(tx->len == 0 || tx->len > sizeof(uart_tx_buf)-1){
		return -1;
	}

    /* set hci type */
    uart_tx_buf[0] = tx->type;

	switch(tx->type)
	{
	case HCI_H4_EVT:
	    /* copy data */
	    memcpy(uart_tx_buf+1, tx->buf, tx->len);
		break;

	case HCI_H4_ACL:
	    /* copy data */
		os_mbuf_copydata(tx->om, 0, tx->len, uart_tx_buf+1);
		break;

	default:
        APP_LOG_ERR("Unrecognized HCI Tx Type: 0x%02x\n", tx->type);
	}

	tx->sent_type = 1;

    // Start UART Tx with DMA
    HAL_Status rc = HAL_UART_SendData_DMA(&HCI_UART_INSTANCE, uart_tx_buf, tx->len+1);
    if (rc != HAL_OK){
        APP_LOG_ERR("HAL Uart Send Data with DMA Fail, error code = %d!\n", rc);
        return -2;
    }

	return 0;
}

void hci_uart_start_tx(void)
{
    struct hci_uart_tx *tx = NULL;
    os_sr_t sr;

    OS_ENTER_CRITICAL(sr);
    tx = STAILQ_FIRST(&tx_q);
    OS_EXIT_CRITICAL(sr);

    if(!tx){
        return;
    }

    /* UART Tx busy */
    if(tx->sent_type){
        return;
    }

    /* start uart tx */
    hci_uart_dma_tx(tx);
}

void app_hci_uart_tx_dma_isr(UART_Cb_Flag_Opt flag, uint8_t *pOutPtr, uint16_t size)
{
    struct hci_uart_tx *tx = NULL;
    os_sr_t sr;

    OS_ENTER_CRITICAL(sr);
    tx = STAILQ_FIRST(&tx_q);
    OS_EXIT_CRITICAL(sr);

    /* UART Tx done handle */
    if(tx->sent_type)
    {
        tx->sent_type = 0;

        switch(tx->type)
        {
        case HCI_H4_EVT:
        {
            ble_transport_free(tx->buf);
            OS_ENTER_CRITICAL(sr);
            STAILQ_REMOVE_HEAD(&tx_q, tx_q_next);
            OS_EXIT_CRITICAL(sr);
            os_memblock_put(&pool_tx_q, tx);
            break;
        }
        case HCI_H4_ACL:
            os_mbuf_free_chain(tx->om);
            OS_ENTER_CRITICAL(sr);
            STAILQ_REMOVE_HEAD(&tx_q, tx_q_next);
            OS_EXIT_CRITICAL(sr);
            os_memblock_put(&pool_tx_q, tx);
            break;

        default:
            APP_LOG_ERR("Unrecognized HCI Tx Type: 0x%02x\n", tx->type);
        }
    }

    /* restart tx */
    hci_uart_start_tx();
}


static void hci_uart_start_rx(void)
{
    /* Start UART Rx */
    HAL_Status rc = HAL_UART_ReceiveDataContinuously_INT(&HCI_UART_INSTANCE, uart_rx_buf, sizeof(uart_rx_buf));
    if (rc != HAL_OK) {
        APP_LOG_ERR("HAL uart rx start failed, rc = %d!\n", rc);
    }
}

static void app_hci_uart_rx_isr(HAL_UART_HandleTypeDef* pUart, HAL_UART_EventOpt event, uint8_t *pBuf, uint16_t xferCount)
{
    /* Handle Rx Finish Event */
    if(event == HAL_UART_EVT_RX_FINISH) {
        //APP_LOG_INFO("UART Rx done, rcvd cnt: %d\n", xferCount);

        hci_uart_rx(pBuf, xferCount);
    }

    /* Handle Continuous Rx Timeout Event */
    if(event == HAL_UART_EVT_CONTI_RX_TIMEOUT) {
        //APP_LOG_INFO("UART Continuous Rx Timeout, rcvd cnt: %d\n", xferCount);

        hci_uart_rx(pBuf, xferCount);
        hci_uart_start_rx();
    }

    /* Handle Continuous Rx Buffer Full Event */
    if(event == HAL_UART_EVT_CONTI_RX_BUFF_FULL) {
    	// close uart rx
        HAL_UART_ReceiveDataAbort_INT(pUart);

        APP_LOG_ERR("UART Continuous Rx buffer full!\n");
    }

    /* Handle Rx Error Events */
    if((event == HAL_UART_EVT_RX_ERR_PARITY) ||
       (event == HAL_UART_EVT_RX_ERR_FRAME)  ||
       (event == HAL_UART_EVT_RX_ERR_OVERRUN))
    {
        // close uart rx
        HAL_UART_ReceiveDataAbort_INT(pUart);

        if (event == HAL_UART_EVT_RX_ERR_PARITY) {
            APP_LOG_WRN("Error: UART Rx Parity Error!\n");
        }
        if (event == HAL_UART_EVT_RX_ERR_FRAME) {
            APP_LOG_WRN("Error: UART Rx Frame Error!\n");
        }
        if (event == HAL_UART_EVT_RX_ERR_OVERRUN) {
            APP_LOG_WRN("Error: UART Rx FIFO Overrun Error!\n");
        }
        APP_LOG_ERR("Actual received data size: %d\n", xferCount);
    }
}

static int hci_uart_init(void)
{
	CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);

#if 1
    SYS_SET_MFP(P1, 2, UART1_TX);
    SYS_SET_MFP(P2, 4, UART1_RX);
    GPIO_EnableDigitalPath(P2, BIT4);
#else
    //SYS_SET_MFP(P0, 1, UART1_TX);
    SYS_SET_MFP(P1, 0, UART1_TX);
    SYS_SET_MFP(P2, 4, UART1_RX);
    GPIO_EnableDigitalPath(P2, BIT4);
#endif
    
#if CONFIG_HCI_UART_FLOW_CTRL_EN
    GPIO_SetMode(P0, BIT(2), GPIO_MODE_INPUT);
    GPIO_SetMode(P1, BIT(1), GPIO_MODE_OUTPUT);
    SYS_SET_MFP(P0, 2, UART1_CTS);
    SYS_SET_MFP(P1, 1, UART1_RTS);
    
    UART_EnableAfc(HCI_UART_INSTANCE.pUartx);
    UART_EnableRts(HCI_UART_INSTANCE.pUartx);
#endif

    HCI_UART_INSTANCE.initObj.baudRate = HCI_UART_BAUDRATE;
    HCI_UART_INSTANCE.initObj.format   = HAL_UART_FMT_8_N_1;
    if (HAL_UART_Init(&HCI_UART_INSTANCE) != HAL_OK) {
        APP_LOG_ERR("HAL uart init failed...\n");
    }

    // Rx interrupt mode
    HCI_UART_INSTANCE.interruptObj.txTrigLevel = HAL_UART_TX_FIFO_HALF_FULL;    // Default config
    HCI_UART_INSTANCE.interruptObj.rxTrigLevel = HAL_UART_RX_FIFO_HALF_FULL;    // Default config
    HCI_UART_INSTANCE.interruptObj.IrqPriority = 2;                             // Default config
    HCI_UART_INSTANCE.interruptObj.callbackFunc = app_hci_uart_rx_isr;
    if(HAL_UART_Init_INT(&HCI_UART_INSTANCE) != HAL_OK) {
        APP_LOG_ERR("HAL uart interrupt init fail...\n");
    }

    // Tx DMA mode
    HAL_DMA_Init();
    if(HAL_UART_Init_DMA(&HCI_UART_INSTANCE, HAL_UART_DMA_TX, app_hci_uart_tx_dma_isr) != HAL_OK) {
    	APP_LOG_ERR("HAL uart tx DMA init failed...\n");
    }
    return 0;
}

/**
 * @brief This function is used by the ble stack and must not be used by users.
 */
void ble_transport_hs_init(void)
{
    int rc;

    /* hci uart initialization */
    rc = hci_uart_init();
    APP_ASSERT(rc == 0);

    /* hci uart tx memory pool initialization */
    rc = os_mempool_init(&pool_tx_q, TX_Q_NUM, sizeof(struct hci_uart_tx),
                         pool_tx_q_buf, "hci_uart_tx_q");
    APP_ASSERT(rc == 0);

    /* hci h4 statement machine initialization */
    hci_h4_sm_init(&hci_uart_h4sm, &hci_h4_allocs_from_hs, hci_uart_frame_cb);

    /* queue initialization. */
    STAILQ_INIT(&tx_q);

    /* start uart rx */
    hci_uart_start_rx();

    APP_LOG_INFO("ble_transport_hs_init ok\r\n");
}

void app_hci_init(void)
{
	/* ble controller initialization */
	pan_ble_controller_init();

	/* Set BD_ADDR */
#if CONFIG_USER_CHIP_MAC_ADDR
	uint8_t mac_addr[6]={0x66, 0x66, 0x66, 0x66, 0x00, 0xC2};
    pan10x_mac_addr_get(mac_addr);
    pan_misc_set_bd_addr(mac_addr);
#else
    int8_t mac_addr[6]={0x66, 0x66, 0x66, 0x66, 0x00, 0xC2};
    pan_misc_set_bd_addr(mac_addr);
#endif
}
