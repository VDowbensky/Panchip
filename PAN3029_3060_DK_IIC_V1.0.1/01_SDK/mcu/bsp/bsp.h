
#ifndef __EV_HC32F460_LQFP100_V2_H__
#define __EV_HC32F460_LQFP100_V2_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "utilities.h"
#include "hc32_common.h"
#include "hc32f460_utility.h"
#include "hc32f460_clk.h"
#include "hc32f460_efm.h"
#include "hc32f460_gpio.h"
#include "hc32f460_exint_nmi_swi.h"
#include "hc32f460_interrupts.h"
#include "hc32f460_pwc.h"
#include "hc32f460_sram.h"
#include "hc32f460_keyscan.h"
#include "hc32_ddl.h"
#include "hc32f460_usart.h"

/*******************************************************************
 * @brief PAN3029/3060 总线模式选择
 *       0: 4线SPI模式
 *       1: 3线SPI模式
 *       2: 软I2C模式
 * @note
 *       4线SPI模式：MOSI、MISO、SCK、NSS
 * - PA09连接至RF的CAD，用于指示RF的CAD信号电平，高电平有效，须设置为输入模式
 * - PB12连接至RF的RESET，用于复位芯片，低电平有效，须设置为输出模式
 ******************************************************************/
#define NSS_PORT                        (PortA)
#define NSS_PIN                         (Pin04)

#define SOFT_I2C_SCL_PORT               (PortA)
#define SOFT_I2C_SCL_PIN                (Pin03)

#define SOFT_I2C_SDA_PORT               (PortA)
#define SOFT_I2C_SDA_PIN                (Pin02)

#define GPIO_PIN_RF_IRQ                 Pin13
#define GPIO_PORT_RF_IRQ                PortB

#define GPIO_PIN_RF_CAD                 Pin09
#define GPIO_PORT_RF_CAD                PortA

#define GPIO_PIN_RF_RST                 Pin12
#define GPIO_PORT_RF_RST                PortB

/*******************************************************************
 * 收发指示灯引脚定义
 * - PC13连接至LED，用于指示RF的TX和RX状态，须设置为输出模式
 *******************************************************************/
#define GPIO_PIN_LED                    Pin13
#define GPIO_PORT_LED                   PortC

/*******************************************************************
 * USART1引脚定义
 * - PA11连接至USART1的RX
 * - PA12连接至USART1的TX，用于打印运行例程日志
 ******************************************************************/
#define USART_RX_PORT                   (PortA)
#define USART_RX_PIN                    (Pin11)
#define USART_RX_FUNC                   (Func_Usart1_Rx)

#define USART_TX_PORT                   (PortA)
#define USART_TX_PIN                    (Pin12)
#define USART_TX_FUNC                   (Func_Usart1_Tx)

/*******************************************************************
 * 收发模式选择引脚定义
 * - PB10连接至TX或RX测试模式
 ******************************************************************/
#define TX_RX_SELECT()                  PORT_GetBit(PortB, Pin10)

#define USART_RI_NUM                    (INT_USART1_RI)
#define USART_EI_NUM                    (INT_USART1_EI)
#define USART_TI_NUM                    (INT_USART1_TI)
#define USART_TCI_NUM                   (INT_USART1_TCI)

#define SPI_UNIT                        (M4_SPI1)
#define SPI_UNIT_CLOCK                  (PWC_FCG1_PERIPH_SPI1)

#define USART_CH                        (M4_USART1)
#define USART_BAUDRATE                  (115200ul)

/*******************************************************************
 * MCU的其它配置定义
 ******************************************************************/
#define TIMER4_UNIT                     (M4_TMR41)
/* Timer4 PWM */
#define TIMER4_PWM_CH                   (Timer4PwmU)    /* Timer4PwmU  Timer4PwmV  Timer4PwmW */
#define TIMER4_PWM_RT_VAL               (250u)
#define WAVE_IO_PORT                    (PortB)
#define WAVE_IO_PIN                     (Pin10)
/* Timer4 RT interrupt number */
#define TIMER4_RT_INT_NUM               (INT_TMR41_RLOU)

#define TMR_UNIT                        (M4_TMR02)
#define TMR_INI_GCMA                    (INT_TMR02_GCMA)
#define TMR_INI_GCMB                    (INT_TMR02_GCMB)

#define ENABLE_TMR0()                   (PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM02, Enable))

#define ADC1_SA_NORMAL_CHANNEL          (ADC1_CH0 | ADC1_CH10)
#define ADC1_SA_AVG_CHANNEL             (ADC1_CH12 | ADC1_CH13)
#define ADC1_SA_CHANNEL                 (ADC1_SA_NORMAL_CHANNEL | ADC1_SA_AVG_CHANNEL)
#define ADC1_SA_CHANNEL_COUNT           (4u)
#define ADC1_AVG_CHANNEL                (ADC1_SA_AVG_CHANNEL)
#define ADC1_CHANNEL                    (ADC1_SA_CHANNEL)
#define ADC1_SA_CHANNEL_SAMPLE_TIME     { 0x30,     0x80,      0x50,      0x60 }
#define ADC1_SCAN_MODE                  (AdcMode_SAOnce)
#define ADC_VREF                        (3.343f)//(3.288f)
#define ADC1_ACCURACY                   (1ul << (12u))
#define TIMEOUT_VAL                     (10u)

 typedef enum
 {
     RESET = 0,
     SET = !RESET
 } FlagStatus, ITStatus;

void BSP_ClockInit(void);
void BSP_KeyInit(void);
void BSP_GpioInit(void);
void BSP_LedToggle(void);
void BSP_UartInit(void);
void BSP_RFBusInit(void);
void BSP_AdcInit(void);
void BSP_SetGpioMode(en_port_t enPort, uint16_t u16Pin, en_pin_mode_t mode);
void BSP_TimerInit(void);
void CAD_PA09_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __EV_HC32F460_LQFP100_V2_H__ */
