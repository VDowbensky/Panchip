#include "myRadio_gpio.h"
#include "at32f413_board.h"
#include "at32f413_clock.h"

RADIO_GPIO_CALLBACK gpioCallback;
int spiMosiMode = 0;
//---------------------------射频SPI驱动部分---------------------
void BOARD_SPI_NSS_H(void)
{
    gpio_bits_write(BOARD_GPIO_SPI_CSN, BOARD_PIN_H);
}
void BOARD_SPI_NSS_L(void)
{
    gpio_bits_write(BOARD_GPIO_SPI_CSN, BOARD_PIN_L);
}
void BOARD_SPI_SCK_H(void)
{
    gpio_bits_write(BOARD_GPIO_SPI_CLK, BOARD_PIN_H);
}
void BOARD_SPI_SCK_L(void)
{
    gpio_bits_write(BOARD_GPIO_SPI_CLK, BOARD_PIN_L);
}
void BOARD_SPI_MISO_H(void)
{
    gpio_bits_write(BOARD_GPIO_SPI_MISO, BOARD_PIN_H);
}
void BOARD_SPI_MISO_L(void)
{
    gpio_bits_write(BOARD_GPIO_SPI_MISO, BOARD_PIN_L);
}
void BOARD_SPI_MOSI_H(void)
{
    gpio_bits_write(BOARD_GPIO_SPI_MOSI, BOARD_PIN_H);
}
void BOARD_SPI_MOSI_L(void)
{
#ifdef SPI_SOFT_3LINE

#endif
    gpio_bits_write(BOARD_GPIO_SPI_MOSI, BOARD_PIN_L);
}
uint8_t READ_BOARD_SPI_MISO(void)
{
#ifndef SPI_SOFT_3LINE
    return gpio_input_data_bit_read(BOARD_GPIO_SPI_MISO);
#else

#endif
}

//---------------------------射频驱动IO部分---------------------
void RF_PAN3029_IRQ_H(void)
{
    gpio_bits_write(RF_PAN3029_IRQ, BOARD_PIN_H);
}
void RF_PAN3029_IRQ_L(void)
{
    gpio_bits_write(RF_PAN3029_IRQ, BOARD_PIN_L);
}
void RF_PAN3029_NRST_H(void)
{
    // gpio_bits_write(RF_PAN3029_NRST, BOARD_PIN_H);
}
void RF_PAN3029_NRST_L(void)
{
    // gpio_bits_write(RF_PAN3029_NRST, BOARD_PIN_L);
}
void RF_PAN3029_IO3_H(void)
{
    gpio_bits_write(RF_PAN3029_IO3, BOARD_PIN_H);
}
void RF_PAN3029_IO3_L(void)
{
    gpio_bits_write(RF_PAN3029_IO3, BOARD_PIN_L);
}
void RF_EXT_PA_RE_H(void)
{
    gpio_bits_write(RF_EXTPA_RE, BOARD_PIN_H);
}
void RF_EXT_PA_RE_L(void)
{
    gpio_bits_write(RF_EXTPA_RE, BOARD_PIN_L);
}
void RF_EXT_PA_TE_H(void)
{
    gpio_bits_write(RF_EXTPA_TE, BOARD_PIN_H);
}
void RF_EXT_PA_TE_L(void)
{
    gpio_bits_write(RF_EXTPA_TE, BOARD_PIN_L);
}
uint8_t READ_RF_PAN3029_IRQ(void)
{
    return gpio_input_data_bit_read(RF_PAN3029_IRQ);
}
void EXINT1_IRQHandler(void)
{
    if(exint_flag_get(EXINT_LINE_1) != RESET)
    {
        /* Clear the EXTI line 1 pending bit */
        exint_flag_clear(EXINT_LINE_1);
        if (READ_RF_PAN3029_IRQ())
        {
            gpioCallback(1);
        }
        
    }
}
// BOARD_GPIOB
void myRadio_gpio_irq_init()
{
    // NVIC_InitTypeDef  NVIC_InitStructure;
    // EXTI_InitTypeDef  EXTI_InitStructure;
    // gpio_init_type  gpio_init_struct;

    // gpio_init_struct.GPIO_Pin = RF_PAN3029_IRQ_PIN;         
    // gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;  
    // gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;      
    // gpio_init(RF_PAN3029_IRQ_PORT, &gpio_init_struct);

    // EXTI_ClearITPendingBit(EXTI_Line0);
    // EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    // EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    // EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    // EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    // EXTI_Init(&EXTI_InitStructure);
    // GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
    // /* Enable and set EXTI1 Interrupt */
    // NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    // NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    // NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    // NVIC_Init(&NVIC_InitStructure);

    exint_init_type exint_init_struct;
    gpio_init_type gpio_init_struct;

  
    /* set default parameter */
    gpio_default_para_init(&gpio_init_struct);
  
    /* configure button pin as input with pull-up/pull-down */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins = RF_PAN3029_IRQ_PIN;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(RF_PAN3029_IRQ_PORT, &gpio_init_struct);

    crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
    gpio_exint_line_config(GPIO_PORT_SOURCE_GPIOB, GPIO_PINS_SOURCE1);
  
    exint_default_para_init(&exint_init_struct);
    exint_init_struct.line_enable = TRUE;
    exint_init_struct.line_mode = EXINT_LINE_INTERRUPUT;
    exint_init_struct.line_select = EXINT_LINE_1;
    exint_init_struct.line_polarity = EXINT_TRIGGER_RISING_EDGE;
    exint_init(&exint_init_struct);
  
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(EXINT1_IRQn, 0, 0);
}
void myRadio_gpio_init(RADIO_GPIO_CALLBACK cb)
{
    gpio_init_type  gpio_init_struct;
    
#if defined(SPI_HARD)    
	// SPI_InitTypeDef  SPI_InitStructure;
    // //----------SPI1时钟使能
    // RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1, ENABLE );

	// gpio_init_struct.GPIO_Pin = BOARD_PIN_SPI_CLK | BOARD_PIN_SPI_MISO | BOARD_PIN_SPI_MOSI;
	// gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	// gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	// gpio_init(BOARD_PORT_SPI_CLK, &gpio_init_struct);

    // GPIO_SetBits(BOARD_GPIO_SPI_CLK | BOARD_PIN_SPI_MISO | BOARD_PIN_SPI_MOSI);
    // /*!< SPI configuration */
    // SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    // SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    // SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    // SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    // SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    // SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    // SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;

    // SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    // SPI_InitStructure.SPI_CRCPolynomial = 7;
    // SPI_Init(SPI1, &SPI_InitStructure);

    spi_init_type spi_init_struct;
    // /*!< Enable the SPI1  */
    // SPI_Cmd(SPI1, ENABLE);
    gpio_default_para_init(&gpio_init_struct);
    /* spi1 sck pin */
    gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_pull           = GPIO_PULL_DOWN;
    gpio_init_struct.gpio_mode         = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins           = BOARD_PIN_SPI_CLK;
    gpio_init(BOARD_PORT_SPI_CLK, &gpio_init_struct);

    /* spi1 miso pin */
    gpio_init_struct.gpio_pull           = GPIO_PULL_DOWN;
    gpio_init_struct.gpio_mode         = GPIO_MODE_INPUT;
    gpio_init_struct.gpio_pins           = BOARD_PIN_SPI_MISO;
    gpio_init(BOARD_PORT_SPI_MISO, &gpio_init_struct);

    /* spi1 mosi pin */
    gpio_init_struct.gpio_pull           = GPIO_PULL_DOWN;
    gpio_init_struct.gpio_mode         = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins           = BOARD_PIN_SPI_MOSI;
    gpio_init(BOARD_PORT_SPI_MOSI, &gpio_init_struct);
  

    crm_periph_clock_enable(CRM_SPI1_PERIPH_CLOCK, TRUE);
    spi_default_para_init(&spi_init_struct);
    spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
    spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
    spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_16;
    spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
    spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
    spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
    spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;
    spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
    spi_init(SPI1, &spi_init_struct);


    spi_enable(SPI1, TRUE);
#else
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_pins = BOARD_PIN_SPI_MOSI;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(BOARD_PORT_SPI_MOSI, &gpio_init_struct);
#ifndef SPI_SOFT_3LINE
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
gpio_init_struct.gpio_pins = BOARD_PIN_SPI_MISO;
gpio_init_struct.gpio_pull = GPIO_PULL_UP;
gpio_init(BOARD_PORT_SPI_MISO, &gpio_init_struct);
    // gpio_init_struct.GPIO_Pin = BOARD_PIN_SPI_MISO;           
    // gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;     
    // gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;      
    // gpio_init(BOARD_PORT_SPI_MISO, &gpio_init_struct);
#endif
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
gpio_init_struct.gpio_pins = BOARD_PIN_SPI_CLK;
gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
gpio_init(BOARD_PORT_SPI_CLK, &gpio_init_struct);
    // gpio_init_struct.GPIO_Pin = BOARD_PIN_SPI_CLK;            
    // gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;      
    // gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;      
    // gpio_init(BOARD_PORT_SPI_CLK, &gpio_init_struct);
    BOARD_SPI_SCK_L();

#endif
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
gpio_init_struct.gpio_pins = BOARD_PIN_SPI_CSN;
gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
gpio_init(BOARD_PORT_SPI_CSN, &gpio_init_struct);
    // gpio_init_struct.GPIO_Pin = BOARD_PIN_SPI_CSN;            
    // gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;      
    // gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;      
    // gpio_init(BOARD_PORT_SPI_CSN, &gpio_init_struct);
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
gpio_init_struct.gpio_pins = RF_PAN3029_IRQ_PIN;
gpio_init_struct.gpio_pull = GPIO_PULL_UP;
gpio_init(RF_PAN3029_IRQ_PORT, &gpio_init_struct);
    // gpio_init_struct.GPIO_Pin = RF_PAN3029_IRQ_PIN;            
    // gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;      
    // gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;      
    // gpio_init(RF_PAN3029_IRQ_PORT, &gpio_init_struct);
// gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
// gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
// gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
// gpio_init_struct.gpio_pins = RF_PAN3029_NRST_PIN;
// gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
// gpio_init(RF_PAN3029_NRST_PORT, &gpio_init_struct);
    // gpio_init_struct.GPIO_Pin = RF_PAN3029_NRST_PIN;            
    // gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;      
    // gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;      
    // gpio_init(RF_PAN3029_NRST_PORT, &gpio_init_struct);
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
gpio_init_struct.gpio_pins = RF_PAN3029_IO3_PIN;
gpio_init_struct.gpio_pull = GPIO_PULL_UP;
gpio_init(RF_PAN3029_IO3_PORT, &gpio_init_struct);
    // gpio_init_struct.GPIO_Pin = RF_PAN3029_IO3_PIN;            
    // gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;      
    // gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;      
    // gpio_init(RF_PAN3029_IO3_PORT, &gpio_init_struct);
gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
gpio_init_struct.gpio_pins = RF_PAN3029_IO11_PIN;
gpio_init_struct.gpio_pull = GPIO_PULL_UP;
gpio_init(RF_PAN3029_IO11_PORT, &gpio_init_struct);
    // gpio_init_struct.GPIO_Pin = RF_PAN3029_IO11_PIN;            
    // gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;      
    // gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;      
    // gpio_init(RF_PAN3029_IO11_PORT, &gpio_init_struct);

    // gpio_init_struct.GPIO_Pin = RF_EXTPA_RE_PIN;            
    // gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;      
    // gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;      
    // gpio_init(RF_EXTPA_RE_PORT, &gpio_init_struct);
    // gpio_init_struct.GPIO_Pin = RF_EXTPA_TE_PIN;            
    // gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;      
    // gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;      
    // gpio_init(RF_EXTPA_TE_PORT, &gpio_init_struct);

    BOARD_SPI_NSS_H();
    RF_PAN3029_NRST_H();
    myRadio_gpio_irq_init();
    gpioCallback = cb;
}
uint8_t myRadioSpi_rwByte(uint8_t byteToWrite)
{
    uint16_t i;
    uint8_t temp;
    temp = 0;   
#if defined(SPI_HARD)
	while (spi_i2s_flag_get(SPI1, SPI_I2S_TDBE_FLAG) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
    {
        i++;
        if(i > 2000)return 0;
    }			  
	spi_i2s_data_transmit(SPI1, byteToWrite); //通过外设SPIx发送一个数据
	i=0;

	while (spi_i2s_flag_get(SPI1, SPI_I2S_RDBF_FLAG) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位
    {
        i++;
        if(i > 2000)return 0;
    }
    /*!< Return the byte read from the SPI bus */
    temp = spi_i2s_data_receive(SPI1);
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

