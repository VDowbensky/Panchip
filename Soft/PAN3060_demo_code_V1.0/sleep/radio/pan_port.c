/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file pan_port.c
 * @brief
 *
 * @history - V0.8, 2024-4
*******************************************************************************/
#include "pan_port.h"
#include "main.h"
#include "spi.h"

rf_port_t rf_port=
{
    .antenna_init = rf_antenna_init,
    .tcxo_init = rf_tcxo_init,
    .set_tx = rf_antenna_tx,
    .set_rx = rf_antenna_rx,
    .antenna_close = rf_antenna_close,
    .tcxo_close = rf_tcxo_close,
    .spi_readwrite = spi_readwritebyte,
    .spi_cs_high = spi_cs_set_high,
    .spi_cs_low = spi_cs_set_low,
    .delayms = rf_delay_ms,
    .delayus = rf_delay_us,
};



/**
 * @brief spi_readwritebyte
 * @param[in] <tx_data> spi readwritebyte value
 * @return result
 */
uint8_t spi_readwritebyte(uint8_t tx_data)
{
	uint8_t rx_data = 0;
	HAL_SPI_TransmitReceive(&hspi1, &tx_data, &rx_data, sizeof(tx_data), 0xFFFF);
	return rx_data;
}

/**
 * @brief spi_cs_set_high
 * @param[in] <none>
 * @return none
 */
void spi_cs_set_high(void)
{
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief spi_cs_set_low
 * @param[in] <none>
 * @return none
 */
void spi_cs_set_low(void)
{
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
}

/**
 * @brief rf_delay_ms
 * @param[in] <time> ms
 * @return none
 */
void rf_delay_ms(uint32_t time)
{
    HAL_Delay(time);
}

/**
 * @brief rf_delay_us
 * @param[in] <time> us
 * @return none
 */
void rf_delay_us(uint32_t time)
{
    uint32_t i;
    uint32_t delay_count = (64000000 / 1000000) * time / 4;  

    for (i = 0; i < delay_count; i++)
    {
        __NOP(); 
    }
}

/**
 * @brief do rf TX/RX IO to initialize
 * @param[in] <none>
 * @return none
 */
void rf_antenna_init(void)
{
    rf_set_gpio_output(MODULE_GPIO_RX);
    rf_set_gpio_output(MODULE_GPIO_TX);
    rf_set_gpio_input(MODULE_GPIO_CAD_IRQ);

    rf_set_gpio_state(MODULE_GPIO_RX, 0);
    rf_set_gpio_state(MODULE_GPIO_TX, 0);
    rf_set_gpio_state(MODULE_GPIO_CAD_IRQ, 0);
}

/**
 * @brief do rf XTAL IO to initialize
 * @param[in] <none>
 * @return none
 */
void rf_tcxo_init(void)
{
	rf_set_gpio_output(MODULE_GPIO_TCXO);
	rf_set_gpio_state(MODULE_GPIO_TCXO, 1);		
}

/**
 * @brief close rf XTAL IO
 * @param[in] <none>
 * @return none
 */
void rf_tcxo_close(void)
{
	rf_set_gpio_output(MODULE_GPIO_TCXO);
	rf_set_gpio_state(MODULE_GPIO_TCXO, 0);
}
/**
 * @brief change rf IO to rx
 * @param[in] <none>
 * @return none
 */
void rf_antenna_rx(void)
{
    rf_set_gpio_state(MODULE_GPIO_TX, 0);
    rf_set_gpio_state(MODULE_GPIO_RX, 1);
}

/**
 * @brief change rf IO to tx
 * @param[in] <none>
 * @return none
 */
void rf_antenna_tx(void)
{
    rf_set_gpio_state(MODULE_GPIO_RX, 0);
    rf_set_gpio_state(MODULE_GPIO_TX, 1);
}

/**
 * @brief change rf IO to close
 * @param[in] <none>
 * @return none
 */
void rf_antenna_close(void)
{
    rf_set_gpio_state(MODULE_GPIO_TX, 0);
    rf_set_gpio_state(MODULE_GPIO_RX, 0);
}

