
#ifndef __MYRADIO_GPIO_H_
#define __MYRADIO_GPIO_H_

#include <stdint.h>

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "board.h"

#ifndef SPI_SOFT_3LINE
    #define SPI_HARD
#endif
// #define SPI_SOFT_3LINE

typedef void (*RADIO_GPIO_CALLBACK)(uint8_t index);

//-------------射频模块引脚映射到转接板排针---------------
#define BOARD_PIN_SPI_CLK       GPIO_Pin_5     //SPI1_SCK(8)
#define BOARD_PORT_SPI_CLK      GPIOA          //DAC_OUT2 ADC12_IN5
#define BOARD_GPIO_SPI_CLK      BOARD_PORT_SPI_CLK, BOARD_PIN_SPI_CLK          //

#define BOARD_PIN_SPI_MISO      GPIO_Pin_6    //SPI1_MISO(8)
#define BOARD_PORT_SPI_MISO     GPIOA         // TIM8_BKIN/ADC12_IN6
                                              // TIM3_CH1(8)
#define BOARD_GPIO_SPI_MISO     BOARD_PORT_SPI_MISO, BOARD_PIN_SPI_MISO         //

#define BOARD_PIN_SPI_MOSI      GPIO_Pin_7    //SPI1_MOSI(8)/
#define BOARD_PORT_SPI_MOSI     GPIOA         // TIM8_CH1N/ADC12_IN7
                                            // TIM3_CH2(8)
#define BOARD_GPIO_SPI_MOSI     BOARD_PORT_SPI_MOSI, BOARD_PIN_SPI_MOSI         //

#define BOARD_PIN_SPI_CSN       GPIO_Pin_4     //SPI1_NSS(8)/
#define BOARD_PORT_SPI_CSN      GPIOA          // USART2_CK(8)
                                               // DAC_OUT1/ADC12_IN4
#define BOARD_GPIO_SPI_CSN      BOARD_PORT_SPI_CSN, BOARD_PIN_SPI_CSN          //
//用于射频中断响应
#define RF_PAN3029_IRQ_PIN      GPIO_Pin_0  //
#define RF_PAN3029_IRQ_PORT     GPIOB
#define RF_PAN3029_IRQ          RF_PAN3029_IRQ_PORT, RF_PAN3029_IRQ_PIN          //
//
#define RF_PAN3029_NRST_PIN     GPIO_Pin_1  //
#define RF_PAN3029_NRST_PORT    GPIOB
#define RF_PAN3029_NRST         RF_PAN3029_NRST_PORT, RF_PAN3029_NRST_PIN          //
//
#define RF_PAN3029_IO3_PIN      GPIO_Pin_5  //
#define RF_PAN3029_IO3_PORT     GPIOC
#define RF_PAN3029_IO3          RF_PAN3029_IO3_PORT, RF_PAN3029_IO3_PIN          //
//
#define RF_PAN3029_IO11_PIN     GPIO_Pin_2  //
#define RF_PAN3029_IO11_PORT    GPIOA
#define RF_PAN3029_IO11         RF_PAN3029_IO11_PORT, RF_PAN3029_IO11_PIN          //
//用于大功率模块的PA和LNA控制脚
#define RF_EXTPA_RE_PIN         GPIO_Pin_10  //
#define RF_EXTPA_RE_PORT        GPIOB
#define RF_EXTPA_RE             RF_EXTPA_RE_PORT, RF_EXTPA_RE_PIN          //
#define RF_EXTPA_TE_PIN         GPIO_Pin_11  //
#define RF_EXTPA_TE_PORT        GPIOB
#define RF_EXTPA_TE             RF_EXTPA_TE_PORT, RF_EXTPA_TE_PIN          //
//-------------射频模块引脚映射到转接板排针---------------END

uint8_t READ_RF_PAN3029_IRQ(void);
void RF_PAN3029_IRQ_H(void);
void RF_PAN3029_IRQ_L(void);

void RF_EXT_PA_RE_H(void);
void RF_EXT_PA_RE_L(void);
void RF_EXT_PA_TE_H(void);
void RF_EXT_PA_TE_L(void);

#define RF_EXT_PA_TO_TX() RF_EXT_PA_TE_H();RF_EXT_PA_RE_L()
#define RF_EXT_PA_TO_RX() RF_EXT_PA_TE_L();RF_EXT_PA_RE_H()
#define RF_EXT_PA_TO_IDLE() RF_EXT_PA_TE_L();RF_EXT_PA_RE_L()

void BOARD_SPI_NSS_H(void);
void BOARD_SPI_NSS_L(void);

//-------------将封装的API映射到射频模块硬件层---------------
void myRadio_gpio_init(RADIO_GPIO_CALLBACK cb);
uint8_t myRadioSpi_rwByte(uint8_t byteToWrite);
void myRadioSpi_wBuffer(uint8_t* pData, uint8_t len);
void myRadioSpi_rBuffer(uint8_t* pData, uint8_t len);

#define SpiReadWrite(p)         myRadioSpi_rwByte(p)
#define SpiWriteData(p1, p2)    myRadioSpi_wBuffer(p2, p1)
#define SpiReadData(p1, p2)     myRadioSpi_rBuffer(p2, p1)
//-------------将封装的API映射到射频模块硬件层---------------END
#endif
