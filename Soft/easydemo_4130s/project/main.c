#include "main.h"
#include "ReadKey.h"
#include "key.h"
#include "ReadKey.h"
#include "crc8.h"
#include "led.h"
#include "eventUnit.h"
#include "myADC.h"
#include "myInputCapture.h"
#include "myLcd.h"
#include "myDisplayUnit.h"
#include "myFlashData.h"
#include "myTim.h"
#include "myUart.h"
#include "myUart3.h"
#include "myRadio.h"

static rfRxPacket_ts rfRecvPacket;
static rfTxPacket_ts rfTxPacket;
static bool rftxflag = false;

static void rcc_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

#if defined(STM32F10X_LD_VL) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD_VL)
    /* ADCCLK = PCLK2/2 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);
#else
    /* ADCCLK = PCLK2/4 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);
#endif
}
void TIM3_CALLBACK(void)
{
    static uint8_t timeCnt_1ms = 0;

    if( ++timeCnt_1ms == 5)
    {
        timeCnt_1ms = 0;
    }
}
void rfRx_callback(uint8_t status, rfRxPacket_ts packet)
{
    switch (status)
    {
        case RX_STA_SECCESS:
        {
            rfRecvPacket = packet;
            myRadio_receiver();
        }
        break;
        case RX_STA_TIMEOUT:
        {
            myRadio_receiver();
        }
        break;
        case RX_STA_PAYLOAD_ERROR:
        {
            myRadio_receiver();
        }
        break;
        case TX_STA_SECCESS:
        {
            myRadio_receiver();
            rftxflag = false;
        }
        break;
        default:
            break;
    }
}
int main(void)
{
    rcc_init();

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); //
    myTim1_init(200, TIM3_CALLBACK);

    myRadio_init(0, rfRx_callback);
    myRadio_setFrequency(433920000);
    myRadio_setTxPower(20);
    myRadio_setRfParams(9, 7, 3);
    myRadio_receiver();
    while(1)
    {   
		if (rftxflag == false)
		{
			myRadio_delay(500);
			rftxflag = true;
			rfTxPacket.len = 5;
			strcpy(rfTxPacket.payload, "hello");
			myRadio_transmit(&rfTxPacket);
		}
        myRadio_process();
    }     
} 

