/**************************************************************************//**
* @file     musbfsfc.c
* @version  V1.00
* $Revision: 2 $
* $Date: 20/08/04 14:25 $
* @brief    Interrupt handler for MUSBFSFC firmware
*
* @note
* Copyright (C) 2020 Panchip Technology Corp. All rights reserved.
*****************************************************************************/
#include "musbfsfc.h"
#include "descript.h"
#include "endpoint.h"
#include "endpoint0.h"
#include "PanSeries.h"
#include "usb.h"
//#define SYS_TEST /*.....*/

//test
uint8_t bulkin_test[1024] = {0,};
uint8_t bulkout_test[1024] = {0,};
uint8_t bulkin_ep_need_data = DATA_NONEED;
volatile uint8_t ep_in_out = 0;

extern uint32_t USB_DmaTx(uint32_t ep);
extern volatile bool usb_dma_en;

void USB_PersonalEpInit(void)
{
	/* Initialise endpoint status structures with endpoint numbers */
	gepbin1.byEP = 1;
	gepbin2.byEP = 2;
	gepbin3.byEP = 3;

	gepbout1.byEP = 1;
	gepbout2.byEP = 2;
	gepbout3.byEP = 3;

	EndpointBulkIn (&gepbin1, M_EP_RESET);
	EndpointBulkIn (&gepbin2, M_EP_RESET);
	EndpointBulkIn (&gepbin3, M_EP_RESET);
	EndpointBulkOut (&gepbout1, M_EP_RESET);
	EndpointBulkOut (&gepbout2, M_EP_RESET);
	EndpointBulkOut (&gepbout3, M_EP_RESET);
}

/****************************************
  USB Reset
****************************************/
void USB_Reset(void)
{
	/* Set device into default state */
	gnDevState = DEVSTATE_DEFAULT;
	/* Initialise descriptors */
	InitialiseDesc();
	/* Reset any state machines for each endpoint */
	/* This example assumes 3 bulk IN endpoints with endpoint numbers 1,2, 3*/
	/* and 3 bulk out endpoints with endpoint numbers 1,2 */
	Endpoint0(M_EP_RESET);

	USB_PersonalEpInit();
	bulkin_ep_need_data = DATA_NEED;
}


void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset)
{
  uint16_t index = 0;

  /* Put in global buffer same values */
  for (index = 0; index < BufferLength; index++)
  {
    pBuffer[index] = index + Offset;
  }
}


static  uint8_t num = 0;
void USB_TestExample(uint32_t IntrFlag_d)
{
	uint8_t cnt = 0;
	uint8_t rec_v[64];

	switch(IntrFlag_d)
	{
		//bulk out
		case USB_INT_BULKOUT_EP1:	//OUT1
			if (usb_dma_en) {
				ep_in_out = 4;
				SYS_TEST("EP1OUT :%d\r\n",ep_in_out);
			} else {
				gepbout1.byEP = 1;
				gepbout1.pData = (void*)rec_v;
				gepbout1.nBytesRecv = 0;
				gepbout1.nBuffLen = 0;
				EndpointBulkOut(&gepbout1, M_EP_NORMAL);
				SYS_TEST("EP1 rcvlen:%d \r\n",gepbout1.nBytesRecv);
				for(cnt =0;cnt < gepbout1.nBytesRecv;cnt++){
					SYS_TEST("0x%02x,",rec_v[cnt]);
				}
				SYS_TEST("\n");
			}
			break;
		case USB_INT_BULKOUT_EP2:	//OUT2
			gepbout2.byEP = 2;
			gepbout2.pData = (void*)rec_v;
			gepbout2.nBytesRecv = 0;
			gepbout2.nBuffLen = 0;
			EndpointBulkOut(&gepbout2, M_EP_NORMAL);
			SYS_TEST("EP2 rcvlen:%d \r\n",gepbout2.nBytesRecv);
			for(cnt =0;cnt < gepbout2.nBytesRecv;cnt++){
				SYS_TEST("0x%02x,",rec_v[cnt]);
			}
			SYS_TEST("\n");
			break;
		case USB_INT_BULKOUT_EP3:	//OUT3
			gepbout3.byEP = 3;
			gepbout3.pData = (void*)rec_v;
			gepbout3.nBytesRecv = 0;
			gepbout3.nBuffLen = 0;
			EndpointBulkOut(&gepbout3, M_EP_NORMAL);
			SYS_TEST("EP3 rcvlen:%d \r\n",gepbout3.nBytesRecv);
			for(cnt =0;cnt < gepbout3.nBytesRecv;cnt++){
				SYS_TEST("0x%02x,",rec_v[cnt]);
			}
			SYS_TEST("\n");
			break;
		case USB_INT_BULKIN_EP1:	//IN1
			SYS_TEST("EP1IN \r\n");
			if (usb_dma_en) {
				USBDMA->CNTL1 = 0x00;
				ep_in_out = 1;
			} else {
				num += 1;
				if(num >= 0x8) num = 0;
				Fill_Buffer(bulkin_test,16,num);
				gepbin1.byEP = 1;						//�˿ں�
				gepbin1.nBytesLeft = 16;					//BULKINһ�δ�����ֽ���
				gepbin1.pData = (void*)bulkin_test;		//���͵Ļ���
				EndpointBulkIn (&gepbin1, M_EP_NORMAL); //����
			}
			bulkin_ep_need_data = DATA_NONEED;
			break;
		case USB_INT_BULKIN_EP2:	//IN2
			num += 1;
			if(num >= 0x8) num = 0;
			SYS_TEST("EP2IN offset:%02x\r\n",num);
			Fill_Buffer(bulkin_test,8,num);
			gepbin2.byEP = 2;
			gepbin2.nBytesLeft = 8;//BULKINһ�δ�����ֽ���
			gepbin2.pData = (void*)bulkin_test;
			EndpointBulkIn(&gepbin2, M_EP_NORMAL);
			bulkin_ep_need_data = DATA_NONEED;
			break;
		case USB_INT_BULKIN_EP3:	//IN3
			num += 1;
			if(num >= 0x8) num = 0;
			SYS_TEST("EP3IN offset:%02x\r\n",num);
			Fill_Buffer(bulkin_test,32,num);
			gepbin3.byEP = 3;
			gepbin3.nBytesLeft = 32;//BULKINһ�δ�����ֽ���
			gepbin3.pData = (void*)bulkin_test;
			EndpointBulkIn(&gepbin3, M_EP_NORMAL);
			bulkin_ep_need_data = DATA_NONEED;
			break;
		default:break;
	}
}

/****************************************
  Top level interrupt handler
****************************************/
void musbfsfc_intr(void)
{
	uint8_t     IntrUSB;
	volatile uint16_t    IntrIn;
	volatile uint16_t    IntrOut;
	volatile uint32_t 	IntrFlag;

	/* Read interrupt registers */
	/* Mote if less than 8 IN endpoints are configured then */
	/* only M_REG_INTRIN1 need be read. */
	/* Similarly if less than 8 OUT endpoints are configured then */
	/* only M_REG_INTROUT1 need be read. */
	IntrUSB = READ_REG(USB->INT_USB);
	IntrIn  = (uint16_t)READ_REG(USB->INT_IN2);
	IntrIn <<= 8;
	IntrIn  |= (uint16_t)READ_REG(USB->INT_IN1);
	IntrOut  = (uint16_t)READ_REG(USB->INT_OUT2);
	IntrOut <<= 8;
	IntrOut  |= (uint16_t)READ_REG(USB->INT_OUT1);

	IntrFlag = IntrIn;
	IntrFlag <<= 16;
	IntrFlag |= IntrOut;

	if (IntrUSB & M_INTR_PLUG) {
		if ((IntrUSB & M_INTR_PLUG_OUT) == M_INTR_PLUG_OUT) {
			SYS_TEST("Plug out\n");
		} else if (IntrUSB & M_INTR_PLUG_OUT) {
			SYS_TEST("Plug in\n");
		}
	}
	/* Check for resume from suspend mode */
	/* Add call to resume routine here */
	if (IntrUSB & M_INTR_RESUME) {
		SYS_TEST("Resume\n");
	}

	if (IntrUSB & M_INTR_SUSPEND) {
		SYS_TEST("Suspend\n");
	}

	if (IntrUSB & M_INTR_SOF) {
		SYS_TEST("Sof\n");
	}

	/* Check for system interrupts */
	if (IntrUSB & M_INTR_RESET)
	{
		USB->POWER |= 0x1;
		USB->INT_USBE |= 0x17;
		SYS_TEST("RST\r\n");
		USB_Reset();
	}

	/* Check for endpoint 0 interrupt */
	if (IntrIn & M_INTR_EP0)
	{
		//	  SYS_TEST("EP0\r\n");
		Endpoint0(M_EP_NORMAL);
	}

	/* Check for each configured endpoint interrupt */
	/* This example assumes 2 bulk IN endpoints with endpoint numbers 1,2, */
	/* and 2 bulk out endpoints with endpoint numbers 1,2 */

	if(IntrFlag&0xfffefffe)
		USB_TestExample(IntrFlag);
	/* Check for suspend mode */
	/*
	Add call to suspend routine here
	if (IntrUSB & M_INTR_RESUME) USBSuspend();
	*/

return;
}
