/****************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
  * $Date: 16/02/ 10:37 $
 * @brief    Transmit and receive data from PC terminal through RS232 interface.
 *
 * @note
 * Copyright (C) 2016 Shanghai Panchip Microelectronics Co., Ltd.   All rights reserved.
 *
 ******************************************************************************/

#include "PanSeries.h"
#include "musbfsfc.h"
#include "descript.h"
#include "endpoint.h"
#include "endpoint0.h"
#include "usb.h"

volatile bool gTfrFlag = false;
volatile uint32_t in_out_cnt = 0;
volatile bool usb_dma_en = false;
extern volatile uint8_t ep_in_out;

uint8_t report[46]={
	0x05, 0x01,  // Usage Page (Generic Desktop)
    0x09, 0x02,  // Usage (Mouse)
    0xA1, 0x01,  // Collection (Application)
//    0x85, 0x01,  // Report Id (1)
    0x09, 0x01,  //   Usage (Pointer)
    0xA1, 0x00,  //   Collection (Physical)
    0x05, 0x09,  //     Usage Page (Buttons)
    0x19, 0x01,  //     Usage Minimum (01) - Button 1
    0x29, 0x03,  //     Usage Maximum (03) - Button 3
    0x15, 0x00,  //     Logical Minimum (0)
    0x25, 0x01,  //     Logical Maximum (1)
    0x95, 0x08,  //     Report Count (8)
	0x75, 0x01,  //     Report Size (1)
    0x81, 0x02,  //     Input (Data, Variable, Absolute) - Button states
//    0x75, 0x05,  //     Report Size (5)
//    0x95, 0x01,  //     Report Count (1)
//    0x81, 0x01,  //     Input (Constant) - Padding or Reserved bits
    0x05, 0x01,  //     Usage Page (Generic Desktop)
    0x09, 0x30,  //     Usage (X)
    0x09, 0x31,  //     Usage (Y)
    0x09, 0x38,  //     Usage (Wheel)
    0x15, 0x81,  //     Logical Minimum (-127)
    0x25, 0x7F,  //     Logical Maximum (127)
    0x75, 0x08,  //     Report Size (8)
    0x95, 0x03,  //     Report Count (3)
    0x81, 0x06,  //     Input (Data, Variable, Relative) - X & Y coordinate
    0xC0,        //   End Collection
    0xC0,        // End Collection
};


void Sys_Init(void)
{
   /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

#ifdef IP_101x
    /* Init I/O Multi-function  */
    SYS_SET_MFP(P1, 1, UART0_TX);
    SYS_SET_MFP(P1, 2, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT2);
#else
    /* Init I/O Multi-function  */
    SYS_SET_MFP(P1, 6, UART0_TX);
    SYS_SET_MFP(P1, 7, UART0_RX);
    GPIO_EnableDigitalPath(P1, BIT7);
#endif

    SYS_LockReg();
 }

void Peri_Init()
{
	UART_InitTypeDef Init_Struct;

	Init_Struct.UART_BaudRate = 921600;
	Init_Struct.UART_LineCtrl = Uart_Line_8n1;

	/* Init UART0 */
	UART_Init(UART0, &Init_Struct);
	UART_EnableFifo(UART0);
}

void Clock_Init(void)
{
	ANA->LP_FSYN_LDO |= 0X1;
    //MCU
	CLK_XthStartupConfig();
    CLK->XTH_CTRL |= CLK_XTHCTL_XTH_EN_Msk;
	CLK_WaitClockReady(CLK_SYS_SRCSEL_XTH);
    //MCU
    CLK_SYSCLKConfig(CLK_DPLL_REF_CLKSEL_XTH,CLK_DPLL_OUT_48M);
    CLK_RefClkSrcConfig(CLK_SYS_SRCSEL_DPLL);


    //APB Enable
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_All,ENABLE);
    CLK_APB1PeriphClockCmd(CLK_APB1Periph_All,ENABLE);
    CLK_APB2PeriphClockCmd(CLK_APB2Periph_All,ENABLE);

}

static int pan10xx_hw_calib_init(void)
{
	OTP_STRUCT_T otp;

    SYS_TEST("Try to load HW calibration data..");
    if (!SystemHwParamLoader(&otp)) {
        SYS_TEST("\nWARNING: Cannot find valid calib data in current chip!\n");
    } else {
        SYS_TEST(" DONE.\n");
        SYS_TEST("- Chip Info         : 0x%x\n", otp.m.chip_info);
        SYS_TEST("- Chip CP Version   : %d\n", otp.m.cp_version);
        SYS_TEST("- Chip FT Version   : %d\n", otp.m.ft_version);
        if (otp.m.ft_version >= 2) {
			SYS_TEST("- Chip MAC Address  : %02X%02X%02X%02X%02X%02X\n", otp.m_v2.mac_addr[0], otp.m_v2.mac_addr[1],
				otp.m_v2.mac_addr[2], otp.m_v2.mac_addr[3], otp.m_v2.mac_addr[4], otp.m_v2.mac_addr[5]);
		} else {
			SYS_TEST("- Chip MAC Address  : %02X%02X%02X%02X%02X%02X\n", otp.m.mac_addr[0], otp.m.mac_addr[1],
				otp.m.mac_addr[2], otp.m.mac_addr[3], otp.m.mac_addr[4], otp.m.mac_addr[5]);
		}
        SYS_TEST("- Chip UID          : %02X%02X%02X%02X%02X%02X%02X%02X%02X\n", otp.m.uid[0], otp.m.uid[1],
            otp.m.uid[2], otp.m.uid[3], otp.m.uid[4], otp.m.uid[5], otp.m.uid[6], otp.m.uid[7], otp.m.uid[8]);

#if INIT_OPTIMIZE_ACTIVE_POWER
        uint32_t tmp;
        int32_t val;

        //buck out(DCDC): ft - 2
        val = (otp.m.buck_out_trim >> 1) - 2;
        if (val < 0x0) {
            val = 0x0;
        }
        tmp = ANA->LP_BUCK_3V;
        tmp &= ~(0xFu << 2);
        tmp |= (val << 2);
        ANA->LP_BUCK_3V = tmp;

        //HPLDO(DVDD): ft - 2
        val = otp.m.hp_ldo_trim - 2;
        if (val < 0x0) {
            val = 0x0;
        }
        tmp = ANA->LP_HP_LDO;
        tmp &= ~(0xFu << 3);
        tmp |= (val << 3);
        ANA->LP_HP_LDO = tmp;
#endif
    }
    SYS_TEST("- Chip Flash UID    : ");
    for (uint32_t i = 0; i < 16; i++) {
        SYS_TEST("%02X", flash_ids.uid[i]);
    }
    SYS_TEST("\n- Chip Flash Size   : %ld KB\n", BIT(flash_ids.memory_density_id) >> 10);

	return 0;
}

void platform_init()
{
    Clock_Init();
    Sys_Init();
    Peri_Init();

    pan10xx_hw_calib_init();
}

void USB_IRQHandler(void)
{
 	musbfsfc_intr();
}

#define SYS_MFP_P03_USB_CLK_1_8      0x00080000UL

extern void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset);
extern uint8_t bulkin_test[1024];
extern uint8_t bulkout_test[1024];
extern uint8_t bulkin_ep_need_data;

void USB_DMA_IRQHandler(void)
{
	SYS_TEST("usb int occured \r\n");
	NVIC_DisableIRQ(USBDMA_IRQn);
}
const uint8_t usb_reg_default[] = {
	0x00,	//FADDR        0X00;
	0x00,	//POWER        0X01;
	0x00,	//INT_IN1      0X02;
	0x00,	//INT_IN2      0X03;
	0x00,	//INT_OUT1     0X04;
	0x00,	//INT_OUT2     0X05;
	0x00,	//INT_USB      0X06;
	0x0f,	//INT_IN1E     0X07;
	0x00,	//INT_IN2E     0X08;
	0x0e,	//INT_OUT1E    0X09;
	0x00,	//INT_OUT2E    0X0A;
	0x06,	//INT_USBE     0X0B;
	0x00,	//FRAME1       0X0C;
	0x00,	//FRAME2       0X0D;
	0x00,	//INDEX        0X0E;
	0x00,	//REV          0X0F;
	0x00,	//MAX_PKT_IN   0X10;
	0x00,	//CSR0_INCSR1  0X11;
	0x00,	//IN_CSR2      0X12;
	0x00,	//MAX_PKT_OUT  0X13;
	0x00,	//OUT_CSR1     0X14;
	0x00,	//OUT_CSR2     0X15;
	0x00,	//OUT_COUNT1   0X16;
	0x00,	//OUT_COUNT2   0X17;
	0x00,	//REV0[8]      0X18~1F;
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,	//FIFO_EP0     0X20;
	0x00,	//REV1[3]      0X21~23;
	0x00,
	0x00,
	0x00,	//FIFO_EP1     0X24;
	0x00,	//REV2[3]      0X25~27;
	0x00,
	0x00,
	0x00,	//FIFO_EP2     0X28
	0x00,	//REV3[3]      0X29~2B
	0x00,
	0x00,
	0x00	//FIFO_EP3     0X2C;
};
void USB_DefaultRegValCheck(void)
{
	uint16_t i = 0, different = 0;
	uint16_t reg_cnt = sizeof(usb_reg_default);

	SYS_TEST("start to check usb register default value \n");

	while(i<reg_cnt){
		if(((__IO uint8_t *)(&USB->FADDR))[i] != usb_reg_default[i]){
			SYS_TEST("usb addr offset:%x,expect:%x,current:%x \r\n",i,usb_reg_default[i],((__IO uint8_t *)(&USB->FADDR))[i]);
			different++;
		}
		i++;
	}
	if(!different){
	   SYS_TEST("usb default value check ok \r\n");
	}
}




void DMA_IRQHandler()
{
    if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_TFR))
    {
        if(DMAC_IntFlag(DMA, 0, DMAC_FLAG_INDEX_TFR)){
			gTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 0, DMAC_FLAG_INDEX_TFR);
        }
        if(DMAC_IntFlag(DMA, 1, DMAC_FLAG_INDEX_TFR)){
			gTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 1, DMAC_FLAG_INDEX_TFR);
        }
        if(DMAC_IntFlag(DMA, 2, DMAC_FLAG_INDEX_TFR)){
			gTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 2, DMAC_FLAG_INDEX_TFR);
        }
    }
	if(DMAC_CombinedIntStatus(DMA, DMAC_FLAG_MASK_DSTTFR))
    {
        if(DMAC_IntFlag(DMA, 0, DMAC_FLAG_INDEX_DSTTFR)){
			gTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 0, DMAC_FLAG_INDEX_DSTTFR);
        }
        if(DMAC_IntFlag(DMA, 1, DMAC_FLAG_INDEX_DSTTFR)){
			gTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 1, DMAC_FLAG_INDEX_DSTTFR);
        }
        if(DMAC_IntFlag(DMA, 2, DMAC_FLAG_INDEX_DSTTFR)){
			gTfrFlag = true;
            DMAC_ClrIntFlag(DMA, 2, DMAC_FLAG_INDEX_DSTTFR);
        }
    }
	USBDMA->CNTL1 = 0x10;
}

#define TRANS_BYTE_NUM	(65)
uint32_t USB_DmaTx(uint32_t ep)
{
	uint32_t gTxChNum;
	uint32_t trans_cnt = TRANS_BYTE_NUM;
	uint8_t dev_send_id = 0;
    DMAC_ChannelConfigTypeDef TxConfigTmp = {0,};

	WRITE_REG(USB->INDEX, ep);
	WRITE_REG (USB->IN_CSR2,M_INCSR2_MODE | M_INCSR2_DMAEnab | M_INCSR2_AUTOSET );
	WRITE_REG(USB->MAX_PKT_IN, 4);
	USBDMA->CNTL1 = 0x12;

	NVIC_EnableIRQ(DMA_IRQn);
	/*dma init*/
	DMAC_Init(DMA);

    Fill_Buffer(bulkin_test,100,in_out_cnt);
	in_out_cnt++;

    TxConfigTmp.AddrChangeDst   = DMAC_AddrChange_NoChange;
    TxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_Increment;
    TxConfigTmp.BurstLenDst     = DMAC_BurstLen_4;
    TxConfigTmp.BurstLenSrc     = DMAC_BurstLen_4;
    TxConfigTmp.DataWidthDst    = DMAC_DataWidth_8;
    TxConfigTmp.DataWidthSrc    = DMAC_DataWidth_8;
    TxConfigTmp.TransferType    = DMAC_TransferType_Mem2Per;
    TxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    TxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    TxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    TxConfigTmp.PeripheralDst   = DMAC_Peripheral_USB_Tx;
    TxConfigTmp.IntEnable       = ENABLE;


	/*get free dma channel;*/
	gTxChNum = DMAC_AcquireChannel(DMA);
	/*enable dma transfer interrupt*/
	DMAC_ClrIntFlagMsk(DMA,gTxChNum,DMAC_FLAG_INDEX_TFR);
	DMAC_SetChannelConfig(DMA,gTxChNum,&TxConfigTmp);
	/*start dma tx channel*/
	DMAC_StartChannel(DMA, gTxChNum,bulkin_test,(void*)&((uint8_t *)&USB->FIFO_EP0)[ep<<2],TRANS_BYTE_NUM);
	USBDMA->CNTL1 = 0x16;
	while(!gTfrFlag){}
	while(trans_cnt){
		if (trans_cnt > USB->MAX_PKT_IN * 8){
			trans_cnt -= USB->MAX_PKT_IN * 8;
			while(ep_in_out==0);
			ep_in_out = 0;
		} else {
			trans_cnt = 0;
			WRITE_REG(USB->CSR0_INCSR1, M_INCSR_IPR);
//			SYS_TEST("CSR0_INCSR1_1:%x\n",USB->CSR0_INCSR1);
			while(ep_in_out==0);
		}
	}
	/*release tx dma channel*/
	DMAC_ReleaseChannel(DMA,gTxChNum);
	gTfrFlag = false;

    /*disable dma*/
    DMAC_DeInit(DMA);
    return 0;
}

uint32_t USB_DmaRx(uint32_t ep)
{
	uint32_t gRxChNum;
	uint32_t nBytes, dma_trans_cnt;
	uint32_t trans_cnt = TRANS_BYTE_NUM;
	uint8_t dev_send_id = 0;
    DMAC_ChannelConfigTypeDef RxConfigTmp = {0,};

	WRITE_REG(USB->INDEX, ep);
	WRITE_REG (USB->OUT_CSR2,M_OUTCSR2_DMAMode | M_OUTCSR2_DMAEnab | M_OUTCSR2_AutoClear );
	WRITE_REG(USB->MAX_PKT_OUT, 4);
	SYS_TEST("select hardware dma mode,send 65 bytes data\n");
	SYS_TEST("a--select hardware dma mode0, every packet has a usb isr,dma isr at last \n");
	SYS_TEST("b--select hardware dma mode1, only dma isr,but last data must read by software \n");
	SYS_TEST("c--select software dma mode\n");
	char input = getchar();
	switch(input){
		case 'a':
		case 'A':
			dma_trans_cnt = trans_cnt;
			WRITE_REG (USB->OUT_CSR2, M_OUTCSR2_DMAEnab | M_OUTCSR2_AutoClear );
			USBDMA->CNTL1 = 0x14;
			break;
		case 'b':
		case 'B':
			dma_trans_cnt = trans_cnt - (trans_cnt % (USB->MAX_PKT_OUT * 8));
			USBDMA->CNTL1 = 0x14;
			break;
		case 'c':
		case 'C':
			dma_trans_cnt = trans_cnt;
			USBDMA->CNTL1 = 0x11;
			break;
		default:
			dma_trans_cnt = trans_cnt;
			WRITE_REG (USB->OUT_CSR2, M_OUTCSR2_DMAEnab | M_OUTCSR2_AutoClear );
			break;
	}

	NVIC_EnableIRQ(DMA_IRQn);
	/*dma init*/
	DMAC_Init(DMA);

    RxConfigTmp.AddrChangeDst   = DMAC_AddrChange_Increment;
    RxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_NoChange;
    RxConfigTmp.BurstLenDst     = DMAC_BurstLen_1;
    RxConfigTmp.BurstLenSrc     = DMAC_BurstLen_1;
    RxConfigTmp.DataWidthDst    = DMAC_DataWidth_8;
    RxConfigTmp.DataWidthSrc    = DMAC_DataWidth_8;
    RxConfigTmp.TransferType    = DMAC_TransferType_Per2Mem;
    RxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    RxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    RxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    RxConfigTmp.PeripheralSrc   = DMAC_Peripheral_USB_Rx;
    RxConfigTmp.IntEnable       = ENABLE;
	/*get free dma channel;*/
	gRxChNum = DMAC_AcquireChannel(DMA);
	/*enable dma transfer interrupt*/
	DMAC_ClrIntFlagMsk(DMA,gRxChNum,DMAC_FLAG_INDEX_TFR);
//	DMAC_ClrIntFlagMsk(DMA,gRxChNum,DMAC_FLAG_INDEX_DSTTFR);
	DMAC_SetChannelConfig(DMA,gRxChNum,&RxConfigTmp);
//	while((USB->OUT_CSR1 & M_OUTCSR_OPR) != M_OUTCSR_OPR);


	if ((input == 'a') || (input == 'A') || (input == 'b') || (input == 'B')){
		DMAC_StartChannel(DMA, gRxChNum,(void*)&((uint8_t *)&USB->FIFO_EP0)[ep<<2],bulkout_test,dma_trans_cnt );
		SYS_TEST("wait dma transfer\n");
		while(!gTfrFlag){}
		SYS_TEST("dma finish\n");

		if ((input == 'b') || (input == 'B')){
			uint32_t rest_data = trans_cnt - dma_trans_cnt;
			WRITE_REG (USB->OUT_CSR2, M_OUTCSR2_DMAEnab | M_OUTCSR2_AutoClear );
			while((USB->OUT_CSR1 & M_OUTCSR_OPR) != M_OUTCSR_OPR);
			USB_Read(1, rest_data, bulkout_test + dma_trans_cnt);
			ep_in_out = 4;
		}
		for(uint32_t cnt =0;cnt < TRANS_BYTE_NUM;cnt++){
			SYS_TEST("0x%02x,",bulkout_test[cnt]);
		}
		SYS_TEST("\n");
	}
	if ((input == 'c') || (input == 'C')) {
		while(trans_cnt){
			if (trans_cnt >= USB->MAX_PKT_OUT * 8){
				trans_cnt -= USB->MAX_PKT_OUT * 8;
				while((USB->OUT_CSR1 & M_OUTCSR_OPR) != M_OUTCSR_OPR);
				DMAC_StartChannel(DMA, gRxChNum,(void*)&((uint8_t *)&USB->FIFO_EP0)[ep<<2],bulkout_test,USB->MAX_PKT_OUT * 8);
				USBDMA->CNTL1 = 0x11;
				SYS_TEST("wait dma transfer\n");
				while(!gTfrFlag){}
				SYS_TEST("dma finish\n");
				for(uint32_t cnt =0;cnt < USB->MAX_PKT_OUT * 8;cnt++){
					SYS_TEST("0x%02x,",bulkout_test[cnt]);
				}
				SYS_TEST("\n");
			} else {
				WRITE_REG(USB->OUT_CSR1, M_OUTCSR_OPR);
				DMAC_StartChannel(DMA, gRxChNum,(void*)&((uint8_t *)&USB->FIFO_EP0)[ep<<2],bulkout_test,trans_cnt);
				USBDMA->CNTL1 = 0x11;
				SYS_TEST("wait last pkt transfer\n");
				while(!gTfrFlag){}
				SYS_TEST("dma last pkt finish\n");
				for(uint32_t cnt =0;cnt < trans_cnt;cnt++){
					SYS_TEST("0x%02x,",bulkout_test[cnt]);
				}
				SYS_TEST("\n");
				trans_cnt = 0;
			}
		}
	}

	/*release tx dma channel*/
	DMAC_ReleaseChannel(DMA,gRxChNum);
	gTfrFlag = false;
    /*disable dma*/
    DMAC_DeInit(DMA);
	WRITE_REG(USB->OUT_CSR1, 0);
    return 0;
}

int main(void)
{
	uint32_t i,data_len=0;
	uint32_t reg_tmp;

	platform_init();
//	USB_DefaultRegValCheck();
	ANA->ANA_MISC_3V |= ((0x1<<27)|(0x1<<28));
	ANA->ANA_MISC_3V |= ((0X1<<26));

	/*usb debounce time*/
	SYS->CTRL1 = ((SYS->CTRL1 & ~0xff) | 100);

	reg_tmp = READ_REG(USB->INT_USBE);
	reg_tmp |= 0x10;
	WRITE_REG (USB->INT_USBE,reg_tmp);

	SYS_TEST("\n cpu %d \r\n",SystemCoreClock);

	Fill_Buffer(bulkin_test,100,0);

	NVIC_EnableIRQ(USB_IRQn);
	SYS_delay_10nop(100000);

	SYS_TEST("1 --  soft bulkin/out\n");
	SYS_TEST("2 --  hardware dma in/out\n");
	char in = getchar();
	if (in == '1'){
		while(1){
			if(bulkin_ep_need_data == DATA_NEED)
			{
				SYS_delay_10nop(500000);
				bulkin_test[0] = 0xf;
				bulkin_test[1] = 0xe;
				bulkin_test[2] = 0xd;
				bulkin_test[3] = 0xc;
				bulkin_test[15] = 0x1f;

				WRITE_REG(USB->INDEX, 1);
				USB_Write((uint32_t)1, 16, (void*)bulkin_test);
				WRITE_REG(USB->CSR0_INCSR1, M_INCSR_IPR);

				bulkin_test[0] = 0x1;
				bulkin_test[1] = 0x2;
				bulkin_test[2] = 0x3;
				bulkin_test[3] = 0x4;
				bulkin_test[15] = 0x1f;
				WRITE_REG(USB->INDEX, 2);
				USB_Write((uint32_t)2, 8, (void*)bulkin_test);
				WRITE_REG(USB->CSR0_INCSR1, M_INCSR_IPR);

				bulkin_test[0] = 0x8;
				bulkin_test[1] = 0x9;
				bulkin_test[2] = 0xa;
				bulkin_test[3] = 0xb;
				bulkin_test[15] = 0x1f;
				WRITE_REG(USB->INDEX, 3);
				USB_Write((uint32_t)3, 32, (void*)bulkin_test);
				WRITE_REG(USB->CSR0_INCSR1, M_INCSR_IPR);

				SYS_delay_10nop(500000);
				bulkin_ep_need_data = DATA_NONEED;
				USB->INT_USBE |= 0x8;
				SYS_TEST("USB test\r\n");
				SYS_delay_10nop(5000);
				USB->INT_USBE &= ~0x8;
			}

		}
	}
	else if (in == '2'){
		SYS_TEST("a --  dma bulkout\n");
		SYS_TEST("b --  dma bulkin\n");
		char input = getchar();
		usb_dma_en = true;
		/*usb dma receive test	*/
		if ('a' == input) {
			SYS_TEST("bulkout_ep_need_data\n");
			while(1) {
				if(bulkin_ep_need_data == DATA_NEED){
					SYS_delay_10nop(500000);
					bulkin_ep_need_data = DATA_NONEED;
					SYS_TEST("receive first packet\n");
					USB_DmaRx(1);
				}
				if (ep_in_out == 4){
					SYS_TEST("receive next packet\n");
					ep_in_out = 0;
					WRITE_REG(USB->OUT_CSR1, M_OUTCSR_FF);
					WRITE_REG(USB->INDEX,1);
					USB_DmaRx(1);
				}
			}
		}
		else if ('b' == input) {
		/*usb dma transfer test	*/
			SYS_TEST("bulkin_ep_need_data\n");
			while(1) {
				if(bulkin_ep_need_data == DATA_NEED){
					SYS_delay_10nop(500000);
					bulkin_ep_need_data = DATA_NONEED;
					SYS_TEST("prepare first packet\n");
					USB_DmaTx(1);
					SYS_TEST("send\n");
				}
				if (ep_in_out == 1){
					SYS_TEST("prepare next packet\n");
					ep_in_out = 0;
					WRITE_REG(USB->INDEX,1);
					WRITE_REG(USB->OUT_CSR1, M_OUTCSR_CDT | M_OUTCSR_FF);
					USB_DmaTx(1);
				}
			}
		}
	}

	while(1){}
	return 0;
}
