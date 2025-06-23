/********************************************************* * 
@file		:spi_dma_m2p.c 
@brief  	:spi dma test case,dma transfer form memory to peripheral spi with efficient dma channel config
@author 	:zhongfeng
@version	:v1.0 
@date 		:19/08/29
Copyright (C) 2019 Panchip Technology Corp. All rights reserved. 
***********************************************************/

#include "PanSeries.h"
#include "dma_common.h"

#define FLASH_READ_ADDR		(0x00010000)
uint8_t ProgramTestBuf[SECTOR_SIZE] __attribute((aligned (4)));
static void FMC_SetProgramTestBuf(uint8_t* buf, uint32_t Size)
{
	uint32_t i;
	
	for(i=0;i<Size;i++)
	{
		buf[i] = i%256;
	}
}

void WriteFlashData(void)
{
	uint32_t ret;

	FMC_EraseSector(FLCTL,FLASH_READ_ADDR);
	ret = BlankCheck(FLCTL,FLASH_READ_ADDR,SECTOR_SIZE);
	if(ret != 0)
	{
		SYS_TEST("check fail!! adr:%x\r\n",ret);
	}

	FMC_SetProgramTestBuf(ProgramTestBuf,SECTOR_SIZE);
	ProgramPage(FLCTL,FLASH_READ_ADDR,SECTOR_SIZE,ProgramTestBuf);
	ret = Verify(FLCTL,FLASH_READ_ADDR,SECTOR_SIZE,ProgramTestBuf);
	if(ret != (FLASH_READ_ADDR+ SECTOR_SIZE))
	{
		SYS_TEST("Verify fail!! adr:%x\r\n",ret);
	}
}

static void SPI_SysInit(void)
{
	SPI_InitTypeDef spiInitStruct;

	/* Init SPI0 Master*/
    spiInitStruct.SPI_format = SPI_FormatMotorola;
    spiInitStruct.SPI_baudRateDiv = 19;//SPI_BaudRateDiv_96;
    spiInitStruct.SPI_CPHA = SPI_ClockPhaseSecondEdge;
    spiInitStruct.SPI_CPOL = SPI_ClockPolarityLow;
    spiInitStruct.SPI_dataFrameSize = SPI_DataFrame_16b;
    spiInitStruct.SPI_role = SPI_RoleMaster;
    
//    SPI0->CPSR  = 2;
    SPI_Init(SPI0, &spiInitStruct);
    //SPI Interrput
    SPI_DisableIrq(SPI0, SPI_IRQ_ALL);
    SPI_EnableSpi(SPI0);

    NVIC_EnableIRQ(DMA_IRQn);
}

/*
  * @brief	   :flash dma transfer,data from flash to peripheral
  * @param in  :where TestCnt is count of transfer 
  * @retval    :none
*/
uint32_t FLASH_DmaFlash2PeripheralTranferCase30(void)
{
    uint32_t TestCntValue = TestCnt;
    DMAC_ChannelConfigTypeDef TxConfigTmp = {0,};
    
	SPI_SysInit();
    /*dma init*/
    DMAC_Init(DMA);

    WriteFlashData();

    /*set spi tx dma channel control&config register*/
    TxConfigTmp.AddrChangeDst   = DMAC_AddrChange_NoChange;
    TxConfigTmp.AddrChangeSrc   = DMAC_AddrChange_Increment;
    TxConfigTmp.BurstLenDst     = DMAC_BurstLen_4;
    TxConfigTmp.BurstLenSrc     = DMAC_BurstLen_4;
    TxConfigTmp.DataWidthDst    = DMAC_DataWidth_16;
    TxConfigTmp.DataWidthSrc    = DMAC_DataWidth_32;
    TxConfigTmp.TransferType    = DMAC_TransferType_Mem2Per;
    TxConfigTmp.FlowControl     = DMAC_FlowControl_DMA;
    TxConfigTmp.HandshakeDst    = DMAC_Handshake_Hardware;
    TxConfigTmp.HandshakeSrc    = DMAC_Handshake_Hardware;
    TxConfigTmp.PeripheralDst   = DMAC_Peripheral_SPI0_Tx;
    TxConfigTmp.IntEnable       = ENABLE;
  
    
    while(TestCntValue--){
        /*get free dma channel;*/
        gTxChNum = DMAC_AcquireChannel(DMA);
        /*enable dma transfer interrupt*/
        DMAC_ClrIntFlagMsk(DMA,gTxChNum,DMAC_FLAG_INDEX_TFR);
        DMAC_SetChannelConfig(DMA,gTxChNum,&TxConfigTmp);
        /*start dma tx channel*/
		uint32_t *p = (uint32_t *)FLASH_READ_ADDR;
		SYS_TEST("%x data is %x\n",FLASH_READ_ADDR, *p);
        DMAC_StartChannel(DMA, gTxChNum,(void*)(p),(void*)&(SPI0->DR),DATA_LENTH_TRANSFER_0);
        
        /*enable spi tx_rx dma mode */
        SPI_SetDmaRxTxEnabled(SPI0, SPI_TX_DMAC_SELECT,1); 
        while(!gTfrFlag){}
        /*disable spi tx_rx dma mode */
        SPI_SetDmaRxTxEnabled(SPI0, SPI_TX_DMAC_SELECT,0);
            
        /*release tx dma channel*/
        DMAC_ReleaseChannel(DMA,gTxChNum);
        gTfrFlag = false;
    }
    
    /*disable dma*/
    DMAC_DeInit(DMA);
    /*release tx dma channel*/
    DMAC_ReleaseChannel(DMA,gTxChNum);
    return 0;
}
