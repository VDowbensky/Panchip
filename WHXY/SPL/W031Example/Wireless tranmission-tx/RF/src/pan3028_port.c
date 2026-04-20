#include "pan3028_port.h"

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
	while (RESET == SPI_GetFlagStatus(CW_SPI1, SPI_FLAG_TXE))
	{
	}

	SPI_SendData(CW_SPI1, tx_data);

	while ((SPI_GetFlagStatus(CW_SPI1,SPI_FLAG_RXNE) == RESET))
	{
	}

	return SPI_ReceiveData(CW_SPI1);
}

/**
 * @brief spi_cs_set_high
 * @param[in] <none>
 * @return none
 */
void spi_cs_set_high(void)
{
	GPIO_WritePin(CW_GPIOB, GPIO_PIN_3, GPIO_Pin_SET);
}

/**
 * @brief spi_cs_set_low
 * @param[in] <none>
 * @return none
 */
void spi_cs_set_low(void)
{
	GPIO_WritePin(CW_GPIOB, GPIO_PIN_3, GPIO_Pin_RESET);
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
 * @brief do PAN3028 TX/RX IO to initialize
 * @param[in] <none>
 * @return none
 */
void rf_antenna_init(void)
{
	PAN3028_set_gpio_output(MODULE_GPIO_RX);
	PAN3028_set_gpio_output(MODULE_GPIO_TX);

	PAN3028_set_gpio_state(MODULE_GPIO_RX, 0);
	PAN3028_set_gpio_state(MODULE_GPIO_TX, 0);    
}

/**
 * @brief do PAN3028 XTAL IO to initialize
 * @param[in] <none>
 * @return none
 */
void rf_tcxo_init(void)
{
	PAN3028_set_gpio_output(MODULE_GPIO_TCXO);
	PAN3028_set_gpio_state(MODULE_GPIO_TCXO, 1);
}

/**
 * @brief close PAN3028 XTAL IO 
 * @param[in] <none>
 * @return none
 */
void rf_tcxo_close(void)
{
	PAN3028_set_gpio_output(MODULE_GPIO_TCXO);
	PAN3028_set_gpio_state(MODULE_GPIO_TCXO, 0);
}
/**
 * @brief change PAN3028 IO to rx
 * @param[in] <none>
 * @return none
 */
void rf_antenna_rx(void)
{ 
	PAN3028_set_gpio_state(MODULE_GPIO_TX, 0);     
	PAN3028_set_gpio_state(MODULE_GPIO_RX, 1);
}

/**
 * @brief change PAN3028 IO to tx
 * @param[in] <none>
 * @return none
 */
void rf_antenna_tx(void)
{
	PAN3028_set_gpio_state(MODULE_GPIO_RX, 0);      
	PAN3028_set_gpio_state(MODULE_GPIO_TX, 1);
}

/**
 * @brief change PAN3028 IO to close
 * @param[in] <none>
 * @return none
 */
void rf_antenna_close(void)
{
	PAN3028_set_gpio_state(MODULE_GPIO_TX, 0);
	PAN3028_set_gpio_state(MODULE_GPIO_RX, 0);
}

