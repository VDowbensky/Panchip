/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file pan3029.c
 * @brief
 *
 * @history - V0.2, 2023-08-04
*******************************************************************************/
#include "pan3029_port.h"

extern uint8_t spi_tx_rx(uint8_t tx_data);

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
	return myRadioSpi_rwByte(tx_data);
}

/**
 * @brief spi_cs_set_high
 * @param[in] <none>
 * @return none
 */
void spi_cs_set_high(void)
{
	BOARD_SPI_NSS_H();
}

/**
 * @brief spi_cs_set_low
 * @param[in] <none>
 * @return none
 */
void spi_cs_set_low(void)
{
	BOARD_SPI_NSS_L();
}
void _delay(uint32_t time_ms)
{
    uint32_t i, j;
    i = time_ms;
    while (i --)
    {
        for ( j = 0; j < 10; j++)
        {
            ;
        }
    }
}
/**
 * @brief rf_delay_ms
 * @param[in] <time> ms
 * @return none
 */
void rf_delay_ms(uint32_t time)
{
	delay1ms(time);
}

/**
 * @brief rf_delay_us
 * @param[in] <time> us
 * @return none
 */
void rf_delay_us(uint32_t time)
{
	delay10us(time/10);
}

/**
 * @brief do PAN3029 TX/RX IO to initialize
 * @param[in] <none>
 * @return none
 */
void rf_antenna_init(void)
{
	PAN3029_set_gpio_output(MODULE_GPIO_RX);
	PAN3029_set_gpio_output(MODULE_GPIO_TX);
	PAN3029_set_gpio_input(MODULE_GPIO_CAD_IRQ);

	PAN3029_set_gpio_state(MODULE_GPIO_RX, 0);
	PAN3029_set_gpio_state(MODULE_GPIO_TX, 0);
	PAN3029_set_gpio_state(MODULE_GPIO_CAD_IRQ, 0);
}

/**
 * @brief do PAN3029 XTAL IO to initialize
 * @param[in] <none>
 * @return none
 */
void rf_tcxo_init(void)
{
	PAN3029_set_gpio_output(MODULE_GPIO_TCXO);
	PAN3029_set_gpio_state(MODULE_GPIO_TCXO, 1);
}

/**
 * @brief close PAN3029 XTAL IO 
 * @param[in] <none>
 * @return none
 */
void rf_tcxo_close(void)
{
	PAN3029_set_gpio_output(MODULE_GPIO_TCXO);
	PAN3029_set_gpio_state(MODULE_GPIO_TCXO, 0);
}
/**
 * @brief change PAN3029 IO to rx
 * @param[in] <none>
 * @return none
 */
void rf_antenna_rx(void)
{ 
	PAN3029_set_gpio_state(MODULE_GPIO_TX, 0);
	PAN3029_set_gpio_state(MODULE_GPIO_RX, 1);
}

/**
 * @brief change PAN3029 IO to tx
 * @param[in] <none>
 * @return none
 */
void rf_antenna_tx(void)
{
	PAN3029_set_gpio_state(MODULE_GPIO_RX, 0);      
	PAN3029_set_gpio_state(MODULE_GPIO_TX, 1);
}

/**
 * @brief change PAN3029 IO to close
 * @param[in] <none>
 * @return none
 */
void rf_antenna_close(void)
{
	PAN3029_set_gpio_state(MODULE_GPIO_TX, 0);
	PAN3029_set_gpio_state(MODULE_GPIO_RX, 0);
}

