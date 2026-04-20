#include "cw32w031_gpio.h"
#include "cw32w031_uart.h"
#include "cw32w031_rcc.h"
#include "cw32w031_spi.h"
#include "cw32w031_gtim.h"
#include "cw32w031_adc.h"
#include "radio.h"
#include "delay.h"

#define BSP_KEY_ROW_NUM         (6U)

#define BSP_KEYIN0_PORT         (CW_GPIOA)
#define BSP_KEYIN0_PIN          (Pin10)

#define BSP_KEYIN1_PORT         (CW_GPIOA)
#define BSP_KEYIN1_PIN          (Pin11)

#define BSP_KEYIN2_PORT         (CW_GPIOA)
#define BSP_KEYIN2_PIN          (Pin12)

#define BSP_KEYIN3_PORT         (CW_GPIOB)
#define BSP_KEYIN3_PIN          (Pin12)

#define BSP_KEYIN4_PORT         (CW_GPIOB)
#define BSP_KEYIN4_PIN          (Pin14)

#define BSP_KEYIN5_PORT         (CW_GPIOB)
#define BSP_KEYIN5_PIN          (Pin15)

#define DCDC_LDO_SELECT         GPIO_ReadPin(CW_GPIOB, GPIO_PIN_0)
#define DCDC_SELECT             GPIO_Pin_SET
#define LDO_SELECT              GPIO_Pin_SET

#define TX_RX_SELECT            GPIO_ReadPin(CW_GPIOB, GPIO_PIN_11)
#define TX_SELECT               GPIO_Pin_SET
#define RX_SELECT               GPIO_Pin_RESET

typedef enum en_pin
{
    Pin00 = (1 <<  0),                  ///< Pin index 00 of each port group
    Pin01 = (1 <<  1),                  ///< Pin index 01 of each port group
    Pin02 = (1 <<  2),                  ///< Pin index 02 of each port group
    Pin03 = (1 <<  3),                  ///< Pin index 03 of each port group
    Pin04 = (1 <<  4),                  ///< Pin index 04 of each port group
    Pin05 = (1 <<  5),                  ///< Pin index 05 of each port group
    Pin06 = (1 <<  6),                  ///< Pin index 06 of each port group
    Pin07 = (1 <<  7),                  ///< Pin index 07 of each port group
    Pin08 = (1 <<  8),                  ///< Pin index 08 of each port group
    Pin09 = (1 <<  9),                  ///< Pin index 09 of each port group
    Pin10 = (1 << 10),                  ///< Pin index 10 of each port group
    Pin11 = (1 << 11),                  ///< Pin index 11 of each port group
    Pin12 = (1 << 12),                  ///< Pin index 12 of each port group
    Pin13 = (1 << 13),                  ///< Pin index 13 of each port group
    Pin14 = (1 << 14),                  ///< Pin index 14 of each port group
    Pin15 = (1 << 15),                  ///< Pin index 15 of each port group
    PinAll= 0xFFFF,                     ///< All pins selected
}en_pin_t;

void  GPIO_Configuration(void);
void  UART_Init(void);
void  SPI_config(void);
void  process_rf_events(void);
void  BSP_LED_Toggle(void);
void ADC_Config(void);
void Rf_Irq_Callback(void);
void GTIM1_CallBack(void);
void GTIM1_MS(uint16_t ms);
