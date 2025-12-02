/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "at32f413_board.h"
#include "at32f413_clock.h"
#include "myRadio.h"

static rfRxPacket_ts rfRecvPacket;
static rfTxPacket_ts rfTxPacket;
static bool rftxflag = false;

/** @addtogroup AT32F413_periph_template
  * @{
  */

/** @addtogroup 413_LED_toggle LED_toggle
  * @{
  */
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
/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();

    /* enable the led clock */
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);
  at32_board_init();
  myRadio_init(0, rfRx_callback);
  myRadio_setFrequency(433920000);
  myRadio_setTxPower(15);
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

/**
  * @}
  */

/**
  * @}
  */
