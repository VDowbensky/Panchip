#include "cw32w031bios.h"

#define DEBUG_USART_BaudRate   115200
#define	RESET   0
#define	SET     1
uint32_t tx_mode = 0;
uint32_t tx_power = 0;
uint32_t freq_mode = 0;
uint32_t tx_pkt_max_num = 1;
uint32_t tx_pkt_cnt = 0;
uint32_t rx_pkt_cnt = 0; 
uint8_t tx_buf[256];
uint32_t tx_len = 10;
uint32_t tx_time;
uint32_t tx_ing = 0;
uint32_t keys_pressed = 0;
uint32_t key_second_pressed = 0;
uint32_t sys_tick = 0;
extern uint32_t app_tx_mode;
extern uint32_t freq_list[FREQ_LIST_NUM];
extern uint32_t cad_tx_timeout_flag;
typedef struct
{
    GPIO_TypeDef* port;
    en_pin_t pin;
    uint32_t first_state;
    uint32_t second_state;
    uint32_t last_tick;
    void (*func)(void);
    void (*func_second)(void);
}key_info_t;

key_info_t key_list[6] = 
{
[0] = {   
        .port = BSP_KEYIN0_PORT,
        .pin = BSP_KEYIN0_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = key_set_sf_routine,
        .func_second = key_set_pl_routine,
    },
    [1] = {   
        .port = BSP_KEYIN1_PORT,
        .pin = BSP_KEYIN1_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = key_set_bw_routine,
        .func_second = key_set_power_routine,
    },    
    [2] = {   
        .port = BSP_KEYIN2_PORT,
        .pin = BSP_KEYIN2_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = key_set_mode_routine,
        .func_second = key_set_cr_routine,
    },
    [3] = {   
        .port = BSP_KEYIN3_PORT,
        .pin = BSP_KEYIN3_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = key_set_start_tx_routine,
		.func_second = key_set_fq_routine,
    },
    [4] = {   
        .port = BSP_KEYIN4_PORT,
        .pin = BSP_KEYIN4_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = key_clear_cnt,
		.func_second = key_set_gain,
    },
    [5] = {   
        .port = BSP_KEYIN5_PORT,
        .pin = BSP_KEYIN5_PIN,
        .first_state = 0,
        .second_state = 0,
        .last_tick = 0,
        .func = NULL,
    },  
};

void key_set_sf_routine(void)
{
    uint32_t para;

	  rf_get_para(RF_PARA_TYPE_SF, &para);
    para ++;
    if(para > 12)
    {
        para = 7;
    }
    dis_set_sf(para);
    OLED_Refresh_Gram();
    rf_set_para(RF_PARA_TYPE_SF, para);
	if(app_tx_mode)
	{
	}
	else
	{
	  rf_port.set_rx();
	  rf_set_mode(PAN3028_MODE_RX);
	}  
}

void key_set_bw_routine(void)
{
    uint32_t para;
    
    rf_get_para(RF_PARA_TYPE_BW, &para);
    para ++;
    if(para > 9)
    {
        para = 6;
    }
    dis_set_bw(para);
    OLED_Refresh_Gram();    
    rf_set_para(RF_PARA_TYPE_BW, para);  
	if(app_tx_mode)
	{
	}
	else
	{
	  rf_port.set_rx();
	  rf_set_mode(PAN3028_MODE_RX);
	} 	
}

void key_set_gain(void)
{
	static uint8_t mode = 0;
	rf_set_mode(PAN3028_MODE_STB3);
	if(mode == 0)
	{
		rf_set_lna_gain(LNA_GAIN_LOW);
		dis_set_gain(LNA_GAIN_LOW);		
		mode = 1;
	}else{
		rf_set_lna_gain(LNA_GAIN_HIGH);
		dis_set_gain(LNA_GAIN_HIGH);	
		mode = 0;
	}

	OLED_Refresh_Gram();
	if(app_tx_mode)
	{
	}
	else
	{
	  rf_port.set_rx();
	  rf_set_mode(PAN3028_MODE_RX);
	} 	
}	

void key_set_pl_routine(void)
{
    tx_len += 10;
    if(tx_len > 240)
    {
        tx_len = 10;
    }
    dis_set_pl(tx_len);
    OLED_Refresh_Gram();
}

void key_set_mode_routine(void)
{
    tx_mode ++;
    if(tx_mode > 2)
    {
        tx_mode =0;
    }
    dis_set_mode(tx_mode);
    OLED_Refresh_Gram();
    if(tx_mode == 0)
    {
        tx_pkt_max_num = 1;
    }
    else if(tx_mode == 1)
    {
        tx_pkt_max_num = 100;
    }
    else
    {
        tx_pkt_max_num = 9999;
    }
    
}

void key_set_power_routine(void)
{
    uint32_t para;
    
    rf_get_para(RF_PARA_TYPE_TXPOWER, &para);
    
    if(para >= 30)
    {
        para = 0;
    }
    else
    {
        para ++;
    }

	rf_set_para(RF_PARA_TYPE_TXPOWER, para); 
	delay1ms(1);
	rf_get_para(RF_PARA_TYPE_TXPOWER, &para);

    dis_set_txpower(para);
    
    OLED_Refresh_Gram();    
	if(app_tx_mode)
	{
	}
	else
	{
	  rf_port.set_rx();
	  rf_set_mode(PAN3028_MODE_RX);
	} 
}

void key_set_cr_routine(void)
{
    uint32_t para;
    
    rf_get_para(RF_PARA_TYPE_CR, &para);
    para ++;
    if(para > 4)
    {
        para = 1;
    }
    dis_set_cr(para);
    OLED_Refresh_Gram();    
    rf_set_para(RF_PARA_TYPE_CR, para);  
	if(app_tx_mode)
	{
	}
	else
	{
	  rf_port.set_rx();
	  rf_set_mode(PAN3028_MODE_RX);
	} 	
}

void key_set_start_tx_routine(void)
{
    if(app_tx_mode)
    {

        tx_ing ++;
        tx_ing %= 2;
        
        if(tx_ing || (tx_pkt_max_num == 1))
        {
            
            if(tx_pkt_max_num != 1)
            {
                tx_pkt_cnt = 0;
            }            
            rf_continous_tx_send_data(tx_buf, tx_len);
        }   
    }
}

void key_set_fq_routine(void)
{
    uint32_t para;
	
    freq_mode ++;
    if(freq_mode > FREQ_LIST_NUM - 1)
    {
        freq_mode =0;
    }
	
    if(freq_mode == 0)
    {
        para = freq_list[freq_mode] * 100000;
    }
    else if(freq_mode == 1)
    {
        para = freq_list[freq_mode] * 100000;
    }
		else if(freq_mode == 2)
    {
        para = freq_list[freq_mode] * 100000;
    }
    else
    {
        para = freq_list[freq_mode] * 100000;
    }
	
    rf_set_para(RF_PARA_TYPE_FREQ, para);  
	
    rf_get_para(RF_PARA_TYPE_FREQ, &para);
	
    dis_set_fq((para/100000));

    OLED_Refresh_Gram();	
	
	if(app_tx_mode)
	{
	}
	else
	{
	  rf_port.set_rx();
	  rf_set_mode(PAN3028_MODE_RX);
	} 	
}

void key_clear_cnt(void)
{
    tx_pkt_cnt = 0;
    rx_pkt_cnt = 0;
    dis_set_cnt( 0 );
    OLED_Refresh_Gram();     
}

void key_scan(void)
{
    uint8_t i;
    uint8_t pin_state;
    uint8_t pin_state_second;
    sys_tick = GetTick();

//    if(sys_tick % 50)
//    {
//        return ;
//    }

    for(i = 0; i < 5; i++)
    {
        pin_state = GPIO_ReadPin(key_list[i].port, key_list[i].pin);  
        
        if((pin_state == RESET) && (key_list[i].first_state == SET) && (key_list[i].second_state == RESET))
        {
            keys_pressed |= (1 << i);
        }
        
        key_list[i].first_state = key_list[i].second_state;
        key_list[i].second_state = pin_state;
    }
    pin_state_second = GPIO_ReadPin(key_list[5].port, key_list[5].pin);
    if(pin_state_second == RESET)
    {
        key_second_pressed = 1;
    }
    else
    {
        key_second_pressed = 0;
    }
}

void key_event_process(void)
{
    uint32_t i = 0;
    
    for(i = 0; i < 5; i++)
    {
        if(keys_pressed & (1 << i))
        {
            if(key_second_pressed == 0)
            {
                if(key_list[i].func != NULL)
                {
                    key_list[i].func();
                }
            }
            else
            {
                if(key_list[i].func_second != NULL)
                {
                    key_list[i].func_second();
                }            
            }
  
            keys_pressed = 0;
            break;
        }
    }
}

void Rf_Irq_Callback(void)
{
	  if (CW_GPIOB->ISR_f.PIN6)
    {
			GPIOB_INTFLAG_CLR(bv6);
			rf_irq_handler();
    }
}


void  GPIO_Configuration(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	
	__RCC_GPIOA_CLK_ENABLE();
	__RCC_GPIOB_CLK_ENABLE();
	__RCC_GPIOC_CLK_ENABLE();
	__RCC_GPIOF_CLK_ENABLE();
	
	//uart的初始化
	PA08_AFx_UART1TXD();
	PA09_AFx_UART1RXD();
	
	GPIO_InitStructure.Pins = GPIO_PIN_8;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Init(CW_GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.Pins = GPIO_PIN_9;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT_PULLUP;
  GPIO_Init(CW_GPIOA, &GPIO_InitStructure); 
	
	//发送接收模式选择引脚PB11初始化   DCDC/LDO模式选择引脚PB00初始化
	GPIO_InitStructure.Pins = GPIO_PIN_11 | GPIO_PIN_0;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT_PULLUP;
  GPIO_Init(CW_GPIOB, &GPIO_InitStructure); 
	
	//按键初始化
	GPIO_InitStructure.Pins = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 ;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  GPIO_Init(CW_GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pins = GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15 ;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  GPIO_Init(CW_GPIOB, &GPIO_InitStructure);
	
  GPIO_InitStructure.Pins = GPIO_PIN_6;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Init(CW_GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pins = GPIO_PIN_10;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Init(CW_GPIOB, &GPIO_InitStructure);
	PA06_SETHIGH();
	PB10_SETHIGH();
}

void  UART_Init(void)
{
	  __RCC_UART1_CLK_ENABLE();
	  USART_InitTypeDef USART_InitStructure = {0};

    USART_InitStructure.USART_BaudRate = DEBUG_USART_BaudRate;
    USART_InitStructure.USART_Over = USART_Over_16;
    USART_InitStructure.USART_Source = USART_Source_PCLK;
    USART_InitStructure.USART_UclkFreq = 48000000;
    USART_InitStructure.USART_StartBit = USART_StartBit_FE;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(CW_UART1, &USART_InitStructure);
	
	
}

void  SPI_config(void)
{
	 GPIO_InitTypeDef GPIO_InitStructure = {0};
	 
	 PB05_AFx_SPI1MOSI();
	 PB04_AFx_SPI1MISO();
	 PB13_AFx_SPI1SCK();
	 
	 GPIO_InitStructure.Pins = GPIO_PIN_5 | GPIO_PIN_3 | GPIO_PIN_13;
     GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
     GPIO_Init(CW_GPIOB, &GPIO_InitStructure);
	 
	 GPIO_InitStructure.Pins = GPIO_PIN_4;
     GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
     GPIO_Init(CW_GPIOB, &GPIO_InitStructure); 
	 
	 //pan3028中断引脚配置
	 GPIO_InitStructure.IT=GPIO_IT_RISING;
	 GPIO_InitStructure.Pins = GPIO_PIN_6;
     GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
     GPIO_Init(CW_GPIOB, &GPIO_InitStructure);
	 
	 PB03_SETHIGH();
	 
	 __RCC_SPI1_CLK_ENABLE();
	 SPI_InitTypeDef SPI_InitStructure;

     SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;    //双向全双工
     SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                         // 主机模式
     SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                     // 帧数据长度为8bit
     SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                            // 时钟空闲电平为低
     SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                          // 第一个边沿采样
     SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                             // 片选信号由SSI寄存器控制
     SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;    // 波特率为PCLK的8分频 6MHz
     SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                    // 最高有效位 MSB 收发在前
     SPI_InitStructure.SPI_Speed = SPI_Speed_High;                          // 高速SPI
  
     SPI_Init(CW_SPI1, &SPI_InitStructure);

     SPI_Cmd(CW_SPI1, ENABLE); 

	
}

void GTIM_Config(void)
{
    GTIM_InitTypeDef GTIM_InitStruct = {0};

    __RCC_GTIM1_CLK_ENABLE();

    GTIM_InitStruct.Mode = GTIM_MODE_TIME;
    GTIM_InitStruct.OneShotMode = GTIM_COUNT_CONTINUE;
    GTIM_InitStruct.Prescaler = 47;    // 计数时钟TCLKD = TCLK/(PSC+1)
    GTIM_InitStruct.ReloadValue = 999;
    GTIM_InitStruct.ToggleOutState = DISABLE;

    GTIM_TimeBaseInit(CW_GTIM1, &GTIM_InitStruct);
    GTIM_ITConfig(CW_GTIM1, GTIM_IT_OV, ENABLE);

}
uint16_t time_ms = 0;
void GTIM1_MS(uint16_t ms)
{
    time_ms = ms;
	GTIM_Cmd(CW_GTIM1,ENABLE);
}

uint16_t gtim1_ms = 0;
void GTIM1_CallBack(void)
{
	gtim1_ms++;

	if(gtim1_ms > time_ms)
	{
		gtim1_ms = 0;
		cad_tx_timeout_flag |= MAC_EVT_TX_CAD_TIMEOUT;
		GTIM_Cmd(CW_GTIM1,DISABLE);;
	}
}

//oled屏幕显示
void dis_init(void)
{
    uint32_t para;
    uint32_t i;
    for(i = 0; i < 256; i++)
    {
        tx_buf[i] = i;
    }

    dis_set_cnt(0);
    dis_set_mode(0);
	
    rf_get_para(RF_PARA_TYPE_FREQ, &para);
	  dis_set_fq((para/100000));
	
    rf_get_para(RF_PARA_TYPE_CR, &para);
    dis_set_cr(para);  
	
    rf_get_para(RF_PARA_TYPE_SF, &para);
    dis_set_sf(para);    

    rf_get_para(RF_PARA_TYPE_BW, &para);
    dis_set_bw(para);    
    
    dis_set_pl(tx_len);
    rf_get_para(RF_PARA_TYPE_TXPOWER, &para);

    dis_set_txpower(para);  
		dis_set_gain(LNA_GAIN_HIGH);
    OLED_Refresh_Gram();
}


void ADC_Config(void)
{
      //打开ADC时钟
      REGBITS_SET( CW_SYSCTRL->APBEN2, SYSCTRL_APBEN2_ADC_Msk );
      //set PA00 as AIN0 INPUT
      PB02_ANALOG_ENABLE();

      ADC_InitTypeDef ADC_InitStructure;
	  ADC_WdtTypeDef ADC_WdtStructure;
	  ADC_SingleChTypeDef ADC_SingleChStructure;
	  //ADC默认值初始化
	  ADC_StructInit(&ADC_InitStructure);
	  //ADC模拟看门狗通道初始化
	  ADC_WdtInit(&ADC_WdtStructure);

	  ADC_InitStructure.ADC_OpMode=ADC_SingleChOneMode;
	  ADC_InitStructure.ADC_ClkDiv = ADC_Clk_Div16;    //ADCCLK:500KHz.
	  ADC_InitStructure.ADC_InBufEn = ADC_BufEnable;
	  ADC_InitStructure.ADC_SampleTime = ADC_SampTime10Clk;

	  //配置单通道转换模式
	  ADC_SingleChStructure.ADC_DiscardEn = ADC_DiscardNull;
	  ADC_SingleChStructure.ADC_Chmux = ADC_ExInputCH10; //选择ADC转换通道
	  ADC_SingleChStructure.ADC_InitStruct = ADC_InitStructure;
	  ADC_SingleChStructure.ADC_WdtStruct = ADC_WdtStructure;

	  ADC_SingleChOneModeCfg(&ADC_SingleChStructure);
	  REGBITS_SET( CW_ADC->START_f.START, ADC_START_START_Msk );
	  ADC_ITConfig(ADC_IT_EOC, ENABLE);
      ADC_EnableIrq(ADC_INT_PRIORITY);
	  ADC_ClearITPendingAll();
	  //ADC_ClearITPendingAll();
 
	  //ADC使能
	  ADC_Enable();
}




//PROCESS
void rf_rx_done_routine(void)
{
    rx_pkt_cnt ++;
	  printf("RF RX:%d\r\n",rx_pkt_cnt);
    dis_set_cnt( rx_pkt_cnt );
    OLED_Refresh_Gram();    
}

void BSP_LED_Toggle(void)
{
	  PB10_TOG();
}


void rf_tx_done_routine(void)
{
	  static uint32_t tx_time;
    tx_pkt_cnt ++;
	  printf("RF TX:%d\r\n",tx_pkt_cnt);
    dis_set_cnt( tx_pkt_cnt );
    OLED_Refresh_Gram();
    if((tx_pkt_cnt < tx_pkt_max_num) && tx_ing)
    {
		    PAN3028_sleep();
		    PAN3028_sleep_wakeup();
            delay1ms(5);
		    rf_single_tx_data(tx_buf, tx_len,&tx_time);
    }
    else
    {
        tx_ing = 0;
        rf_set_mode(PAN3028_MODE_STB3);
    }
}


extern struct RxDoneMsg RxDoneParams;
void process_rf_events(void)
{
    uint32_t flag = 0;
    
    flag = rf_get_transmit_flag();
    if(flag == RADIO_FLAG_TXDONE)
    {
		    BSP_LED_Toggle();
        rf_tx_done_routine();
        rf_set_transmit_flag(RADIO_FLAG_IDLE);
    }
    
    flag = rf_get_recv_flag();
    if(flag == RADIO_FLAG_RXDONE)
    {
		    uint8_t i;  	
		    BSP_LED_Toggle();
            rf_rx_done_routine();
            rf_set_recv_flag(RADIO_FLAG_IDLE);
		
		    printf("Rx : SNR: %f ,RSSI: %f \r\n", RxDoneParams.Snr, RxDoneParams.Rssi);
		    for(i = 0; i < RxDoneParams.Size; i++)
		    {
			      printf("0x%02x ", RxDoneParams.Payload[i]);
		    }
		    printf("\r\n");
    }     
}

