#ifndef __BOARD_H
#define __BOARD_H
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

//--------------------------------------------

#define BOARD_PIN_H Bit_SET
#define BOARD_PIN_L Bit_RESET

//-----------------------LED指示灯，低电平驱动
#define BOARD_PIN_LED1    GPIO_Pin_3   //
#define BOARD_PORT_LED1   GPIOA         
#define BOARD_GPIO_LED1   BOARD_PORT_LED1, BOARD_PIN_LED1          //

#define BOARD_PIN_LED2    GPIO_Pin_3
#define BOARD_PORT_LED2   GPIOC
#define BOARD_GPIO_LED2   BOARD_PORT_LED2, BOARD_PIN_LED2          //

#define LED_ON   Bit_RESET
#define LED_OFF   Bit_SET

#define BOARD_PIN_BEEP    GPIO_Pin_12
#define BOARD_PORT_BEEP    GPIOA
#define BOARD_GPIO_BEEP   BOARD_PORT_BEEP, BOARD_PIN_BEEP          //

#define BEEP_ON   Bit_SET
#define BEEP_OFF   Bit_RESET
//-----------------------------------------LED指示灯，END
//-----------------------LCD显示器驱动IO口
#define BG_LED_ON       Bit_RESET
#define BG_LED_OFF      Bit_SET
#define BOARD_PIN_LCD_BG_LED    GPIO_Pin_4   //
#define BOARD_PORT_LCD_BG_LED   GPIOB
#define BOARD_GPIO_LCD_BG_LED   BOARD_PORT_LCD_BG_LED, BOARD_PIN_LCD_BG_LED          //

#define BOARD_PIN_LCD_SDA    GPIO_Pin_7   //
#define BOARD_PORT_LCD_SDA   GPIOB
#define BOARD_GPIO_LCD_SDA   BOARD_PORT_LCD_SDA, BOARD_PIN_LCD_SDA          //

#define BOARD_PIN_LCD_CLK    GPIO_Pin_6   //
#define BOARD_PORT_LCD_CLK   GPIOB
#define BOARD_GPIO_LCD_CLK   BOARD_PORT_LCD_CLK, BOARD_PIN_LCD_CLK          //

#define BOARD_PIN_LCD_RST    GPIO_Pin_5   //
#define BOARD_PORT_LCD_RST   GPIOB
#define BOARD_GPIO_LCD_RST   BOARD_PORT_LCD_RST, BOARD_PIN_LCD_RST          //

#define LCD_SDA_H()     GPIO_WriteBit(BOARD_GPIO_LCD_SDA, Bit_SET)
#define LCD_SDA_L()     GPIO_WriteBit(BOARD_GPIO_LCD_SDA, Bit_RESET)
#define READ_LCD_SDA()     GPIO_ReadInputDataBit(BOARD_GPIO_LCD_SDA)
#define LCD_CLK_H()     GPIO_WriteBit(BOARD_GPIO_LCD_CLK, Bit_SET)
#define LCD_CLK_L()     GPIO_WriteBit(BOARD_GPIO_LCD_CLK, Bit_RESET)

#define LCD_RST_ENABLE()     GPIO_WriteBit(BOARD_GPIO_LCD_RST, Bit_RESET)
#define LCD_RST_DISABLE()     GPIO_WriteBit(BOARD_GPIO_LCD_RST, Bit_SET)

#define LCD_BG_LED_ON()     GPIO_WriteBit(BOARD_GPIO_LCD_BG_LED, BG_LED_OFF)
#define LCD_BG_LED_OFF()     GPIO_WriteBit(BOARD_GPIO_LCD_BG_LED, BG_LED_ON)

//-----------------------------------------LCD显示器驱动IO口，END

#define BOARD_PIN_KEY1    GPIO_Pin_0
#define BOARD_PORT_KEY1   GPIOA
#define BOARD_GPIO_KEY1   BOARD_PORT_KEY1, BOARD_PIN_KEY1          //

#define BOARD_PIN_KEY2    GPIO_Pin_14
#define BOARD_PORT_KEY2   GPIOB
#define BOARD_GPIO_KEY2   BOARD_PORT_KEY2, BOARD_PIN_KEY2          //

#define BOARD_PIN_KEY3    GPIO_Pin_15
#define BOARD_PORT_KEY3   GPIOB
#define BOARD_GPIO_KEY3   BOARD_PORT_KEY3, BOARD_PIN_KEY3          //

#define BOARD_PIN_KEY4    GPIO_Pin_6
#define BOARD_PORT_KEY4   GPIOC
#define BOARD_GPIO_KEY4   BOARD_PORT_KEY4, BOARD_PIN_KEY4          //

#define BOARD_PIN_KEY5    GPIO_Pin_7
#define BOARD_PORT_KEY5   GPIOC
#define BOARD_GPIO_KEY5   BOARD_PORT_KEY5, BOARD_PIN_KEY5          //

#define BOARD_PIN_CURRENT_AD    GPIO_Pin_1  //对应ADC1
#define BOARD_PORT_CURRENT_AD   GPIOA       
#define BOARD_GPIO_CURRENT_AD   BOARD_PORT_CURRENT_AD, BOARD_PIN_CURRENT_AD          //
#endif
