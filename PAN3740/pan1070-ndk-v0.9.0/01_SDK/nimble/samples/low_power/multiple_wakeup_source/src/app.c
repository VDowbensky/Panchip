/**
 *******************************************************************************
 * @file     app.c
 * @create   2024-12-10
 * @author   Panchip BLE GROUP
 * @note
 * Copyright (c) 2022-2024 Shanghai Panchip Microelectronics Co.,Ltd.
 *
 *******************************************************************************
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "soc_api.h"

#include "app.h"
#include "app_log.h"

/* Configure UART1 data buffer size */
#define UART_DATA_BUFF_SIZE     64

static uint8_t uart_data_buf[UART_DATA_BUFF_SIZE];

/* Configure time interval (in seconds) for switch from deepsleep mode to standby mode */
#define INTERVAL_FOR_SWITCH_LOWPOWER_MODE   30

/* Stores the handle of the task that will be notified when the transmission is complete. */
static TaskHandle_t xTaskToNotify = NULL;

/* This function overrides the reserved weak function with same name in soc_pm.c */
void sleep_timer1_handler(void)
{
    APP_LOG_INFO("\nSoc has stayed in deepsleep mode more than %ds, now try to enter standby mode 1..\n\n", INTERVAL_FOR_SWITCH_LOWPOWER_MODE);

    /* Wait until all UART0 data sending done before entering standby mode */
    while (!(UART_GetLineStatus(UART0) & UART_LINE_TXSR_EMPTY)) {
        /* Busy wait */
    }

    /* Enable P02 interrupt before entering standby mode for wakeup */
    GPIO_EnableInt(P0, 2, GPIO_INT_FALLING);

    /* Enter standby mode1 with all sram power off */
    soc_enter_standby_mode_1(STBM1_WAKEUP_SRC_GPIO, STBM1_RETENTION_SRAM_NONE);

    APP_LOG("WARNING: Failed to enter SoC standby mode 1 due to unexpected interrupt detected.\n");
    APP_LOG("         Please check if there is an unhandled interrupt during the standby mode 1\n");
    APP_LOG("         entering flow.\n");

    while (1) {
        /* Busy wait */
    }
}

/*
 * This is the application hook function running in OS IDLE task. Before use this, the
 * macro configUSE_IDLE_HOOK should be enabled in FreeRTOSConfig.h.
 *
 * Note that the function definition is a bit different with FreeRTOS original one, that
 * is, here we add a return value for flexibility.
 *
 * Return Value:
 * - false:   Continue run the following code after vApplicationIdleHook() in IDLE task,
 *            which is equivalent to the original FreeRTOS implementation.
 * - true:    Avoid run following code after vApplicationIdleHook() in IDLE task, and this
 *            could prevent SoC entering DeepSleep flow when CONFIG_PM enabled.
 */
bool vApplicationIdleHook(void)
{
    /*
     * Enter sleep mode (instead of deepsleep) if UART Rx FIFO is not empty
     * (Which means there is already data receiving from remote)
     */
    if (!UART_IsRxFifoEmpty(UART1)) {
        __disable_irq();
        __WFI();
        __enable_irq();
        /* Avoid entering DeepSleep flow here */
        return true;
    }

    /* Allow entering DeepSleep flow */
    return false;
}

/*
 * This is hook function runs right after entering SoC DeepSleep Flow.
 * Note that either the os scheduler or systick are stopped before running this
 * function, so do not run any os related objects (such as OS Timer) here.
 */
CONFIG_RAM_CODE void vSocDeepSleepEnterHook(void)
{
    /* Enable and Set timeout of SleepTimer1 */
    uint32_t timeout = soc_32k_clock_freq_get() * INTERVAL_FOR_SWITCH_LOWPOWER_MODE;
    LP_SetSleepTime(ANA, timeout, LP_SLPTMR_CH1);
    APP_LOG_INFO("SleepTimer1 started, timeout=%ds\n", INTERVAL_FOR_SWITCH_LOWPOWER_MODE);

    /* Handle UART0 (Log UART) */

    /* Wait until all UART0 data sending done before entering deepsleep mode */
    while (!(UART_GetLineStatus(UART0) & UART_LINE_TXSR_EMPTY)) {
        /* Busy wait */
    }
    /* Reset UART0 PINs to GPIO function and disable digital input path of UART0 Rx PIN to avoid possible current leakage. */
    SYS_SET_MFP(P1, 6, GPIO);
    SYS_SET_MFP(P1, 7, GPIO);
    GPIO_DisableDigitalPath(P1, BIT7);

    /* Handle UART1 (Data UART) */

    /* Wait until all UART1 data sending done before entering deepsleep mode */
    while (!(UART_GetLineStatus(UART1) & UART_LINE_TXSR_EMPTY)) {
        /* Busy wait */
    }
    /* Reset UART1 PINs to GPIO function and enable gpio interrupt for uart rx pin. */
    SYS_SET_MFP(P1, 0, GPIO);
    SYS_SET_MFP(P0, 7, GPIO);
    GPIO_EnableInt(P0, 7, GPIO_INT_FALLING);

    /* Enable P02 interrupt before entering deepsleep mode for wakeup */
    GPIO_EnableInt(P0, 2, GPIO_INT_FALLING);
}

CONFIG_RAM_CODE void vSocDeepSleepExitHook(void)
{
    /* Resume UART0 PIN Configurations to reenable UART0 function */
    SYS_SET_MFP(P1, 6, UART0_TX);
    SYS_SET_MFP(P1, 7, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT7);

    /* Resume UART1 PIN Configurations here to reenable UART1 function if is not waked up by P07 */
    if (!GPIO_GetIntFlag(P0, BIT7)) {
        SYS_SET_MFP(P1, 0, UART1_TX);
        SYS_SET_MFP(P0, 7, UART1_RX);
        GPIO_DisableInt(P0, 7);
    }

    /* Re-disable P02 interrupt after deepsleep wakeup */
    GPIO_DisableInt(P0, 2);

    /* Trigger App Task reschedule after wakeup from deepsleep mode */
    xTaskNotifyGive(xTaskToNotify);
}

CONFIG_RAM_CODE void GPIO0_IRQHandlerOverlay(void)
{
    /* Check if WKUP (P02) button pressed */
    if (GPIO_GetIntFlag(P0, BIT2)) {
        GPIO_ClrIntFlag(P0, BIT2);
        APP_LOG_INFO("WKUP key (P02) pressed..\n");
    }

    /* Check if UART1 Rx (P07) GPIO wakeup triggered */
    if (GPIO_GetIntFlag(P0, BIT7)) {
        /* Clear gpio pin irq flag */
        GPIO_ClrIntFlag(P0, BIT7);
        while (P07 == 0) {
            /* Busy wait until P07 pulled high, which means the 1st wakup
             * trigger character (0x00) send done.
             */
        }
        /* Resume UART1 PIN configs after P07 pulled high */
        SYS_SET_MFP(P1, 0, UART1_TX);
        SYS_SET_MFP(P0, 7, UART1_RX);
        GPIO_DisableInt(P0, 7);
        APP_LOG_INFO("UART1 Rx (P07) GPIO wakeup triggered..\n");
    }

    /* Try to do context switch right after current isr return */
    taskYIELD();
}

static void UART_HandleReceivedData(UART_T* UARTx)
{
    static uint8_t rx_index = 0;

    APP_LOG("Data received from UART: ");
    while (!UART_IsRxFifoEmpty(UARTx)) {
        uart_data_buf[rx_index] = UART_ReceiveData(UARTx);
        APP_LOG("0x%02x ", uart_data_buf[rx_index]);
        rx_index++;
        // Handle Rx buffer full
        if (rx_index >= UART_DATA_BUFF_SIZE) {
            APP_LOG_WRN("WARNING: Too much data received, UART Rx buffer full!\n");
            rx_index = 0;   // Reset Rx buf index
            break;
        }
    }
    APP_LOG("\n");
}

static void UART_HandleProc(UART_T *UARTx)
{
    UART_EventDef event = UART_GetActiveEvent(UARTx);

    switch (event) {
    case UART_EVENT_DATA:
    case UART_EVENT_TIMEOUT:
        /* Handle received data */
        UART_HandleReceivedData(UARTx);
        break;
    case UART_EVENT_NONE:
        /* Just ignore this event. */
        break;
    default:
        APP_LOG_WRN("WARNING: Unhandled event, IID: 0x%x\n", event);
        break;
    }
}

void UART1_IRQHandlerOverlay(void)
{
    UART_HandleProc(UART1);
}

static void uart1_comm_init(void)
{
    /* Enable UART1 Clock */
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_UART1, ENABLE);

    /* Configure UART Pinmux */
    SYS_SET_MFP(P1, 0, UART1_TX);
    SYS_SET_MFP(P0, 7, UART1_RX);
    /* Enable digital input path of UART Rx Pin */
    GPIO_EnableDigitalPath(P0, BIT7);

    /* Init UART1 */
    UART_InitTypeDef Init_Struct = {
        .UART_BaudRate = 9600,
        .UART_LineCtrl = Uart_Line_8n1,
    };
    UART_Init(UART1, &Init_Struct);
    UART_EnableFifo(UART1);

    /* Configure interrupt of UART1 */
    UART_SetRxTrigger(UART1, UART_RX_FIFO_HALF_FULL);
    UART_EnableIrq(UART1, UART_IRQ_RECV_DATA_AVL);      // Enable RDA Interrupt
    NVIC_EnableIRQ(UART1_IRQn);                         // Enable target UART INT in NVIC
}

static void parse_stbm1_gpio_wakeup_source(void)
{
    uint32_t src;

    src = soc_stbm1_gpio_wakeup_src_get();

    APP_LOG_INFO("gpio wakeup src flag = 0x%08x\n", src);

    for (size_t i = 0; i < 32; i++) {
        if (src & (1u << i)) {
            APP_LOG_INFO("SoC is waked up by GPIO P%d_%d.\n", i / 8, i % 8);
        }
    }
}

static void wakeup_gpio_key_init(void)
{
    /* Set pinmux func as GPIO */
    SYS_SET_MFP(P0, 2, GPIO);

    /* Configure debounce clock */
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_RCL, GPIO_DBCTL_DBCLKSEL_4);
    /* Enable input debounce function of specified GPIO */
    GPIO_EnableDebounce(P0, BIT2);

    /* Set GPIO to input mode */
    GPIO_SetMode(P0, BIT2, GPIO_MODE_INPUT);
    CLK_Wait3vSyncReady(); /* Necessary for P02 to do manual aon-reg sync */

    /* Enable internal pull-up resistor path */
    GPIO_EnablePullupPath(P0, BIT2);
    CLK_Wait3vSyncReady(); /* Necessary for P02 to do manual aon-reg sync */

    /* Wait for a while to ensure the internal pullup is stable before entering low power mode */
    soc_busy_wait(10000);

    /* Disable P02 interrupt at this time */
    GPIO_DisableInt(P0, 2);
}

static void wakeup_gpio_uart1_rx_pin_init(void)
{
    /* Set GPIO to input mode */
    GPIO_SetMode(P0, BIT7, GPIO_MODE_INPUT);

    /* Enable internal pull-up resistor path */
    GPIO_EnablePullupPath(P0, BIT7);

    /* Disable P07 interrupt before entering deepsleep */
    GPIO_DisableInt(P0, 7);
}

static void wakeup_gpio_init(void)
{
    /* Configure gpio wakeup key (P02) */
    wakeup_gpio_key_init();

    /* Configure gpio wakeup pin for uart rx */
    wakeup_gpio_uart1_rx_pin_init();

    /* Enable GPIO IRQs in NVIC */
    NVIC_EnableIRQ(GPIO0_IRQn);
}

/**
 *******************************************************************************
 * @brief user initialization entry
 *******************************************************************************
 */
void app_setup(void)
{
    APP_LOG_INFO("App started..\n\n");

    uint8_t rst_reason;

    /* Get the last reset reason */
    APP_LOG_INFO("Reset Reason: ");
    rst_reason = soc_reset_reason_get();
    switch (rst_reason) {
    case SOC_RST_REASON_POR_RESET:
        APP_LOG("Power On Reset.\n");
        break;
    case SOC_RST_REASON_PIN_RESET:
        APP_LOG("nRESET Pin Reset.\n");
        break;
    case SOC_RST_REASON_SYS_RESET:
        APP_LOG("NVIC System Reset.\n");
        break;
    case SOC_RST_REASON_STBM1_GPIO_WAKEUP:
        APP_LOG("Standby Mode 1 GPIO Wakeup.\n");
        parse_stbm1_gpio_wakeup_source();
        break;
    default:
        APP_LOG("Unhandled Reset Reason (%d), refer to more reason define in soc_api.h!\n", rst_reason);
    }

    /* Enable and init UART1 for data transmission */
    uart1_comm_init();

    /* Enable specific gpios wakeup for wakeup use */
    wakeup_gpio_init();
}

/**
 *******************************************************************************
 * @brief application main loop
 *******************************************************************************
 */
void app_main_loop(void)
{
    uint32_t ulNotificationValue;

    /* Store the handle of current task. */
    xTaskToNotify = xTaskGetCurrentTaskHandle();
    if (xTaskToNotify == NULL) {
        APP_LOG_ERR("Error, get current task handle failed!\n");
    }

    while (1) {
        /* Busy wait a while to simulate cpu busy status */
        APP_LOG_INFO("Busy wait a while..\n");
        soc_busy_wait(1000000);

        /* Try to take wakeup_sem */
        APP_LOG_INFO("Wait for Task Notifications..\n");
        /*
         * Wait to be notified that gpio key is pressed (gpio irq occured). Note the first parameter
         * is pdTRUE, which has the effect of clearing the task's notification value back to 0, making
         * the notification value act like a binary (rather than a counting) semaphore.
         */
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        APP_LOG_INFO("A notification received, value: %d.\n\n", ulNotificationValue);

        /* Stop the slptmr1 timer (by resetting the counter) */
        LP_SetSleepTime(ANA, 0, LP_SLPTMR_CH1);
        APP_LOG_INFO("SleepTimer1 stopped.\n");
    }
}
