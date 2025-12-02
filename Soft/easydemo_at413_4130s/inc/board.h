#ifndef __BOARD_H
#define __BOARD_H
#include "at32f413_board.h"
#include "at32f413_clock.h"

//--------------------------------------------

#define BOARD_PIN_H TRUE
#define BOARD_PIN_L FALSE

//-----------------------LED指示灯，低电平驱动
#define BOARD_PIN_LED1    GPIO_PINS_3   //
#define BOARD_PORT_LED1   GPIOA         
#define BOARD_GPIO_LED1   BOARD_PORT_LED1, BOARD_PIN_LED1          //

#define BOARD_PIN_LED2    GPIO_PINS_3
#define BOARD_PORT_LED2   GPIOC
#define BOARD_GPIO_LED2   BOARD_PORT_LED2, BOARD_PIN_LED2          //

#define LED_ON   BOARD_PIN_H
#define LED_OFF   BOARD_PIN_L

#define BOARD_PIN_BEEP    GPIO_PINS_12
#define BOARD_PORT_BEEP    GPIOA
#define BOARD_GPIO_BEEP   BOARD_PORT_BEEP, BOARD_PIN_BEEP          //

#define BEEP_ON   BOARD_PIN_L
#define BEEP_OFF   BOARD_PIN_H
//-----------------------------------------LED指示灯，END
//-----------------------LCD显示器驱动IO口
#define BG_LED_ON       BOARD_PIN_H
#define BG_LED_OFF      BOARD_PIN_L
#define BOARD_PIN_LCD_BG_LED    GPIO_PINS_4   //
#define BOARD_PORT_LCD_BG_LED   GPIOB
#define BOARD_GPIO_LCD_BG_LED   BOARD_PORT_LCD_BG_LED, BOARD_PIN_LCD_BG_LED          //

#define BOARD_PIN_LCD_SDA    GPIO_PINS_7   //
#define BOARD_PORT_LCD_SDA   GPIOB
#define BOARD_GPIO_LCD_SDA   BOARD_PORT_LCD_SDA, BOARD_PIN_LCD_SDA          //

#define BOARD_PIN_LCD_CLK    GPIO_PINS_6   //
#define BOARD_PORT_LCD_CLK   GPIOB
#define BOARD_GPIO_LCD_CLK   BOARD_PORT_LCD_CLK, BOARD_PIN_LCD_CLK          //

#define BOARD_PIN_LCD_RST    GPIO_PINS_5   //
#define BOARD_PORT_LCD_RST   GPIOB
#define BOARD_GPIO_LCD_RST   BOARD_PORT_LCD_RST, BOARD_PIN_LCD_RST          //

#define LCD_SDA_H()     gpio_bits_write(BOARD_GPIO_LCD_SDA, BOARD_PIN_L)
#define LCD_SDA_L()     gpio_bits_write(BOARD_GPIO_LCD_SDA, BOARD_PIN_H)
#define READ_LCD_SDA()     gpio_output_data_bit_read(BOARD_GPIO_LCD_SDA)
#define LCD_CLK_H()     gpio_bits_write(BOARD_GPIO_LCD_CLK, BOARD_PIN_L)
#define LCD_CLK_L()     gpio_bits_write(BOARD_GPIO_LCD_CLK, BOARD_PIN_H)

#define LCD_RST_ENABLE()     gpio_bits_write(BOARD_GPIO_LCD_RST, BOARD_PIN_H)
#define LCD_RST_DISABLE()     gpio_bits_write(BOARD_GPIO_LCD_RST, BOARD_PIN_L)

#define LCD_BG_LED_ON()     gpio_bits_write(BOARD_GPIO_LCD_BG_LED, BG_LED_OFF)
#define LCD_BG_LED_OFF()     gpio_bits_write(BOARD_GPIO_LCD_BG_LED, BG_LED_ON)

//-----------------------------------------LCD显示器驱动IO口，END

#define BOARD_PIN_KEY1    GPIO_PINS_0
#define BOARD_PORT_KEY1   GPIOA
#define BOARD_GPIO_KEY1   BOARD_PORT_KEY1, BOARD_PIN_KEY1          //

#define BOARD_PIN_KEY2    GPIO_PINS_14
#define BOARD_PORT_KEY2   GPIOB
#define BOARD_GPIO_KEY2   BOARD_PORT_KEY2, BOARD_PIN_KEY2          //

#define BOARD_PIN_KEY3    GPIO_PINS_15
#define BOARD_PORT_KEY3   GPIOB
#define BOARD_GPIO_KEY3   BOARD_PORT_KEY3, BOARD_PIN_KEY3          //

#define BOARD_PIN_KEY4    GPIO_PINS_6
#define BOARD_PORT_KEY4   GPIOC
#define BOARD_GPIO_KEY4   BOARD_PORT_KEY4, BOARD_PIN_KEY4          //

#define BOARD_PIN_KEY5    GPIO_PINS_7
#define BOARD_PORT_KEY5   GPIOC
#define BOARD_GPIO_KEY5   BOARD_PORT_KEY5, BOARD_PIN_KEY5          //

#define BOARD_PIN_CURRENT_AD    GPIO_PINS_1  //对应ADC1
#define BOARD_PORT_CURRENT_AD   GPIOA       
#define BOARD_GPIO_CURRENT_AD   BOARD_PORT_CURRENT_AD, BOARD_PIN_CURRENT_AD          //
#endif
