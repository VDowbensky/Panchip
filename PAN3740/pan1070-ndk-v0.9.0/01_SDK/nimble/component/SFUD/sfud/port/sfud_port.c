/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 */

#include <sfud.h>
#include <stdarg.h>
#include "PanSeries.h"
#include "soc_api.h"
#include "utility.h"

#ifdef SFUD_LOG_USE_VSNPRINT
static char log_buf[256];
#endif

void sfud_log_debug(const char *file, const long line, const char *format, ...);
void sfud_log_info(const char *format, ...);

#ifdef SFUD_USING_QSPI
#error "Current SoC doesn't support QSPI!"
#endif

static void spi_lock(sfud_spi *spi)
{
    // Lock IRQs and store the key to user_data
    spi->user_data = (void*)irq_lock();
}

static void spi_unlock(sfud_spi *spi)
{
    // Unclock IRQs using key stored in user_data
    irq_unlock((uint32_t)spi->user_data);
}

static void retry_delay_100us(void) {
    soc_busy_wait(100);
}

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf, size_t read_size)
{
    sfud_err result = SFUD_SUCCESS;
    uint8_t send_data, read_data;
    size_t tx_cnt = 0;
    size_t rx_cnt = 0;
    size_t retry_times = 1000;

    if (write_size) {
        SFUD_ASSERT(write_buf);
    }
    if (read_size) {
        SFUD_ASSERT(read_buf);
    }

    // Wait until current SPI is idle
    while (SPI_IsBusy(SPI0)) {
        SFUD_RETRY_PROCESS(retry_delay_100us, retry_times, result);
    }
    if (result != SFUD_SUCCESS) {
        goto exit;
    }

#if !SFUD_USE_HARDWARE_SPI_CS
    // Pull down CS pin
    P03 = 0;
#endif

    // Send all data and try to receive as much data as we can at same time
    while (tx_cnt < write_size + read_size) {
        if (!SPI_IsTxFifoFull(SPI0) && !SPI_IsRxFifoFull(SPI0)) {
            if (tx_cnt < write_size) {
                send_data = *write_buf++;
            } else {
                send_data = SFUD_DUMMY_DATA;
            }
            SPI_SendData(SPI0, send_data);
            tx_cnt++;
        }
        if (!SPI_IsRxFifoEmpty(SPI0)) {
            read_data = SPI_ReceiveData(SPI0);
            if (++rx_cnt > write_size) {
                *read_buf++ = read_data;
            }
        }
    }

    // Receive remained data if previouse actually received data size is not same with we've sent
    while (rx_cnt < tx_cnt) {
        if (!SPI_IsRxFifoEmpty(SPI0)) {
            read_data = SPI_ReceiveData(SPI0);
            if (++rx_cnt > write_size) {
                *read_buf++ = read_data;
            }
        }
    }

exit:

#if !SFUD_USE_HARDWARE_SPI_CS
    // Pull up CS pin
    P03 = 1;
#endif

    return result;
}

sfud_err sfud_spi_port_init(sfud_flash *flash)
{
    sfud_err result = SFUD_SUCCESS;

    SPI_InitTypeDef spiInitStruct = {
        .SPI_role = SPI_RoleMaster,
        .SPI_dataFrameSize = SPI_DataFrame_8b,
        .SPI_baudRateDiv = SPI_BAUD_RATE_DIV(2),
        .SPI_format = SPI_FormatMotorola,
        .SPI_CPOL = SPI_ClockPolarityHigh,
        .SPI_CPHA = SPI_ClockPhaseSecondEdge,
    };

    switch (flash->index) {
    case SFUD_FLASH0_DEVICE_INDEX:
        // Enable SPI0 Clock
        CLK_APB1PeriphClockCmd(CLK_APB1Periph_SPI0, ENABLE);
        // Config SPI0 Pinmux
        SYS_SET_MFP(P1, 1, SPI0_MOSI);
        SYS_SET_MFP(P0, 5, SPI0_MISO);
#if SFUD_USE_HARDWARE_SPI_CS
        SYS_SET_MFP(P0, 3, SPI0_CS);
#else
        SYS_SET_MFP(P0, 3, GPIO);
        GPIO_SetMode(P0, BIT3, GPIO_MODE_OUTPUT);
        P03 = 1;
#endif
        SYS_SET_MFP(P0, 4, SPI0_CLK);
        GPIO_EnableDigitalPath(P0, BIT5);   // SPI MISO is input pin while act as master
        // Init SPI0 module
        SPI_Init(SPI0, &spiInitStruct);
        SPI_EnableSpi(SPI0);
        // Register APIs to upper level
        flash->spi.wr = spi_write_read;
        flash->spi.lock = (void (*)(const struct __sfud_spi*))spi_lock;
        flash->spi.unlock = (void (*)(const struct __sfud_spi*))spi_unlock;
        flash->retry.delay = retry_delay_100us; // About 100 microsecond delay
        flash->retry.times = 10 * 10000; // About 10 seconds timeout */
        break;
    default:
        SFUD_INFO("Error: %s low level driver is not implemented.", flash->name);
        break;
    }

    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...)
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    SYS_PRINT("[SFUD][DBG] (%s:%ld) ", file, line);
    /* must use vprintf to print */
#ifdef SFUD_LOG_USE_VSNPRINT
    vsnprintk(log_buf, sizeof(log_buf), format, args);
    SYS_PRINT("%s\n", log_buf);
#else
    vprintk(format, args);
    SYS_PRINT("\n");
#endif
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...)
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    SYS_PRINT("[SFUD][INF] ");
#ifdef SFUD_LOG_USE_VSNPRINT
    vsnprintk(log_buf, sizeof(log_buf), format, args);
    SYS_PRINT("%s\n", log_buf);
#else
    vprintk(format, args);
    SYS_PRINT("\n");
#endif
    va_end(args);
}
