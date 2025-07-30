#include "myRadio_gpio.h"
#include "stm32f10x.h"
#include "stm32f10x_exti.h"

RADIO_GPIO_CALLBACK gpioCallback;
int spiMosiMode = 0;
//---------------------------射频SPI驱动部分---------------------
void BOARD_SPI_NSS_H(void)
{
    GPIO_WriteBit(BOARD_GPIO_SPI_CSN, BOARD_PIN_H);
}
void BOARD_SPI_NSS_L(void)
{
    GPIO_WriteBit(BOARD_GPIO_SPI_CSN, BOARD_PIN_L);
}
void BOARD_SPI_SCK_H(void)
{
    GPIO_WriteBit(BOARD_GPIO_SPI_CLK, BOARD_PIN_H);
}
void BOARD_SPI_SCK_L(void)
{
    GPIO_WriteBit(BOARD_GPIO_SPI_CLK, BOARD_PIN_L);
}
void BOARD_SPI_MISO_H(void)
{
    GPIO_WriteBit(BOARD_GPIO_SPI_MISO, BOARD_PIN_H);
}
void BOARD_SPI_MISO_L(void)
{
    GPIO_WriteBit(BOARD_GPIO_SPI_MISO, BOARD_PIN_L);
}
void BOARD_SPI_MOSI_H(void)
{
#ifdef SPI_SOFT_3LINE
    int ret;
    if (spiMosiMode == 0)
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = BOARD_PIN_SPI_MOSI;           
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
        GPIO_Init(BOARD_PORT_SPI_MOSI, &GPIO_InitStructure);
    }
    spiMosiMode = 1;
#endif
    GPIO_WriteBit(BOARD_GPIO_SPI_MOSI, BOARD_PIN_H);
}
void BOARD_SPI_MOSI_L(void)
{
#ifdef SPI_SOFT_3LINE
    int ret;
    if (spiMosiMode == 0)
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = BOARD_PIN_SPI_MOSI;           
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
        GPIO_Init(BOARD_PORT_SPI_MOSI, &GPIO_InitStructure);
    }
    spiMosiMode = 1;
#endif
    GPIO_WriteBit(BOARD_GPIO_SPI_MOSI, BOARD_PIN_L);
}
uint8_t READ_BOARD_SPI_MISO(void)
{
#ifndef SPI_SOFT_3LINE
    return GPIO_ReadInputDataBit(BOARD_GPIO_SPI_MISO);
#else
    int ret;
    if (spiMosiMode == 1)
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = BOARD_PIN_SPI_MOSI;           
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;     
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
        GPIO_Init(BOARD_PORT_SPI_MOSI, &GPIO_InitStructure);
    }
    spiMosiMode = 0;
    ret = GPIO_ReadInputDataBit(BOARD_GPIO_SPI_MOSI);
    return ret;
#endif
}

//---------------------------射频驱动IO部分---------------------
void RF_PAN3029_IRQ_H(void)
{
    GPIO_WriteBit(RF_PAN3029_IRQ, BOARD_PIN_H);
}
void RF_PAN3029_IRQ_L(void)
{
    GPIO_WriteBit(RF_PAN3029_IRQ, BOARD_PIN_L);
}
void RF_PAN3029_NRST_H(void)
{
    GPIO_WriteBit(RF_PAN3029_NRST, BOARD_PIN_H);
}
void RF_PAN3029_NRST_L(void)
{
    GPIO_WriteBit(RF_PAN3029_NRST, BOARD_PIN_L);
}
void RF_PAN3029_IO3_H(void)
{
    GPIO_WriteBit(RF_PAN3029_IO3, BOARD_PIN_H);
}
void RF_PAN3029_IO3_L(void)
{
    GPIO_WriteBit(RF_PAN3029_IO3, BOARD_PIN_L);
}
void RF_EXT_PA_RE_H(void)
{
    GPIO_WriteBit(RF_EXTPA_RE, BOARD_PIN_H);
}
void RF_EXT_PA_RE_L(void)
{
    GPIO_WriteBit(RF_EXTPA_RE, BOARD_PIN_L);
}
void RF_EXT_PA_TE_H(void)
{
    GPIO_WriteBit(RF_EXTPA_TE, BOARD_PIN_H);
}
void RF_EXT_PA_TE_L(void)
{
    GPIO_WriteBit(RF_EXTPA_TE, BOARD_PIN_L);
}
uint8_t READ_RF_PAN3029_IRQ(void)
{
    return GPIO_ReadInputDataBit(RF_PAN3029_IRQ);
}
void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        /* Clear the EXTI line 1 pending bit */
        EXTI_ClearITPendingBit(EXTI_Line0);
        if (READ_RF_PAN3029_IRQ())
        {
            gpioCallback(1);
        }
        
    }
}
// BOARD_GPIOB
void myRadio_gpio_irq_init()
{
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = RF_PAN3029_IRQ_PIN;         
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      
    GPIO_Init(RF_PAN3029_IRQ_PORT, &GPIO_InitStructure);

    EXTI_ClearITPendingBit(EXTI_Line0);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
    /* Enable and set EXTI1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
void myRadio_gpio_init(RADIO_GPIO_CALLBACK cb)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
    
#if defined(SPI_HARD)    
    //----------SPI1时钟使能
    RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1, ENABLE );

	GPIO_InitStructure.GPIO_Pin = BOARD_PIN_SPI_CLK | BOARD_PIN_SPI_MISO | BOARD_PIN_SPI_MOSI;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(BOARD_PORT_SPI_CLK, &GPIO_InitStructure);

    GPIO_SetBits(BOARD_GPIO_SPI_CLK | BOARD_PIN_SPI_MISO | BOARD_PIN_SPI_MOSI);
    /*!< SPI configuration */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;

    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    /*!< Enable the SPI1  */
    SPI_Cmd(SPI1, ENABLE);
#else

    GPIO_InitStructure.GPIO_Pin = BOARD_PIN_SPI_MOSI;           
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
    GPIO_Init(BOARD_PORT_SPI_MOSI, &GPIO_InitStructure);
#ifndef SPI_SOFT_3LINE
    GPIO_InitStructure.GPIO_Pin = BOARD_PIN_SPI_MISO;           
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;     
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
    GPIO_Init(BOARD_PORT_SPI_MISO, &GPIO_InitStructure);
#endif
    GPIO_InitStructure.GPIO_Pin = BOARD_PIN_SPI_CLK;            
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
    GPIO_Init(BOARD_PORT_SPI_CLK, &GPIO_InitStructure);
    BOARD_SPI_SCK_L();

#endif
    GPIO_InitStructure.GPIO_Pin = BOARD_PIN_SPI_CSN;            
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
    GPIO_Init(BOARD_PORT_SPI_CSN, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = RF_PAN3029_IRQ_PIN;            
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
    GPIO_Init(RF_PAN3029_IRQ_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = RF_PAN3029_NRST_PIN;            
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
    GPIO_Init(RF_PAN3029_NRST_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = RF_PAN3029_IO3_PIN;            
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
    GPIO_Init(RF_PAN3029_IO3_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = RF_PAN3029_IO11_PIN;            
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
    GPIO_Init(RF_PAN3029_IO11_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = RF_EXTPA_RE_PIN;            
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
    GPIO_Init(RF_EXTPA_RE_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = RF_EXTPA_TE_PIN;            
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;      
    GPIO_Init(RF_EXTPA_TE_PORT, &GPIO_InitStructure);

    BOARD_SPI_NSS_H();
    RF_PAN3029_NRST_H();
    myRadio_gpio_irq_init();
    gpioCallback = cb;
}
uint8_t myRadioSpi_rwByte(uint8_t byteToWrite)
{
    uint16_t i = 0;
    uint8_t temp;
    temp = 0;   
#if defined(SPI_HARD)
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
    {
        i++;
        if(i > 2000)return 0;
    }			  
	SPI_I2S_SendData(SPI1, byteToWrite); //通过外设SPIx发送一个数据
	i=0;

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位
    {
        i++;
        if(i > 2000)return 0;
    }
    /*!< Return the byte read from the SPI bus */
    temp = SPI_I2S_ReceiveData(SPI1);
#else
    BOARD_SPI_SCK_L();
    for(i = 0; i < 8; i ++)
    {
        if(byteToWrite & 0x80)
        {
            BOARD_SPI_MOSI_H();
        }
        else
        {
            BOARD_SPI_MOSI_L();
        } 

        byteToWrite <<= 1;

        BOARD_SPI_SCK_H();
        temp <<= 1;
        if(READ_BOARD_SPI_MISO())
        {
            temp ++; 
        }
        BOARD_SPI_SCK_L();
    }
#endif
	return temp;
}

void myRadioSpi_wBuffer(uint8_t* pData, uint8_t len)
{
    uint8_t i;
    
    for(i = 0; i < len; i++)
    {
        myRadioSpi_rwByte(*pData);
        pData ++;
    }
}

void myRadioSpi_rBuffer(uint8_t* pData, uint8_t len)
{
    uint8_t i;
    
    for(i = 0; i < len; i++)
    {
        *pData = myRadioSpi_rwByte(0xFF);
        pData ++;
    }
}

