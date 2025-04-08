/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct 
{	
	unsigned char rf_reach_timeout			: 1;
	unsigned char is_tx						      : 1;	
	unsigned char blink						      : 1;	
	unsigned char reach_tx						  : 1;	
	unsigned char reach_50ms						: 1;	
	unsigned char reach_10us						: 1;	
	unsigned char para_editing					: 1;
}FlagType;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
extern FlagType Flag;
extern uint8_t cnt_500ms;
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RST_Pin GPIO_PIN_3
#define RST_GPIO_Port GPIOA
#define RF_IRQ_Pin GPIO_PIN_0
#define RF_IRQ_GPIO_Port GPIOB
#define OLED_SCLK_Pin GPIO_PIN_12
#define OLED_SCLK_GPIO_Port GPIOB
#define OLED_SDIN_Pin GPIO_PIN_13
#define OLED_SDIN_GPIO_Port GPIOB
#define OLED_DC_Pin GPIO_PIN_15
#define OLED_DC_GPIO_Port GPIOB
#define SPI1_CS_Pin GPIO_PIN_8
#define SPI1_CS_GPIO_Port GPIOA
#define LED_TX_Pin GPIO_PIN_9
#define LED_TX_GPIO_Port GPIOA
#define LED_RX_Pin GPIO_PIN_10
#define LED_RX_GPIO_Port GPIOA
#define KEY_SET_Pin GPIO_PIN_5
#define KEY_SET_GPIO_Port GPIOB
#define KEY_UP_Pin GPIO_PIN_6
#define KEY_UP_GPIO_Port GPIOB
#define KEY_DOWN_Pin GPIO_PIN_7
#define KEY_DOWN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define MODE_MASTER		1
#define MODE_SLAVE		2
#define MODE_TXTEST		3
#define MODE_RXTEST		4
#define MODE_SLEEP		5

#define LED_TX_ON()		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET)
#define LED_TX_OFF()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET)

#define LED_RX_ON()		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET)
#define LED_RX_OFF()	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET)
#define LED_RX_TOG()	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10)

#define SPIn_CS_LOW()       HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET)
#define SPIn_CS_HIGH()      HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET)

#define RST_RESET()		HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET)
#define RST_SET()	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
