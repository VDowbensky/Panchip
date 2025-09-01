#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

#include "hc32_ddl.h"
#include "hc32f460_gpio.h"

#if (DDL_GPIO_ENABLE == DDL_ON)

typedef struct{
    union
    {
        __IO uint16_t PIDR;
        stc_port_pidr_field_t PIDR_f;
    };
    uint8_t RESERVED1[2];
    union
    {
        __IO uint16_t PODR;
        stc_port_podr_field_t PODR_f;
    };
    union
    {
        __IO uint16_t POER;
        stc_port_poer_field_t POER_f;
    };
    union
    {
        __IO uint16_t POSR;
        stc_port_posr_field_t POSR_f;
    };
    union
    {
        __IO uint16_t PORR;
        stc_port_porr_field_t PORR_f;
    };
    union
    {
        __IO uint16_t POTR;
        stc_port_potr_field_t POTR_f;
    };
    uint8_t RESERVED6[2];
}gpioe_inout_t;

typedef struct{
  union
    {
        __IO uint16_t PCR0;
        stc_port_pcr_field_t PCR0_f;
    };
    union
    {
        __IO uint16_t PFSR0;
        stc_port_pfsr_field_t PFSR0_f;
    };
    union
    {
        __IO uint16_t PCR1;
        stc_port_pcr_field_t PCR1_f;
    };
    union
    {
        __IO uint16_t PFSR1;
        stc_port_pfsr_field_t PFSR1_f;
    };
    union
    {
        __IO uint16_t PCR2;
        stc_port_pcr_field_t PCR2_f;
    };
    union
    {
        __IO uint16_t PFSR2;
        stc_port_pfsr_field_t PFSR2_f;
    };
    union
    {
        __IO uint16_t PCR3;
        stc_port_pcr_field_t PCR3_f;
    };
    union
    {
        __IO uint16_t PFSR3;
        stc_port_pfsr_field_t PFSR3_f;
    };
    union
    {
        __IO uint16_t PCR4;
        stc_port_pcr_field_t PCR4_f;
    };
    union
    {
        __IO uint16_t PFSR4;
        stc_port_pfsr_field_t PFSR4_f;
    };
    union
    {
        __IO uint16_t PCR5;
        stc_port_pcr_field_t PCR5_f;
    };
    union
    {
        __IO uint16_t PFSR5;
        stc_port_pfsr_field_t PFSR5_f;
    };
    union
    {
        __IO uint16_t PCR6;
        stc_port_pcr_field_t PCR6_f;
    };
    union
    {
        __IO uint16_t PFSR6;
        stc_port_pfsr_field_t PFSR6_f;
    };
    union
    {
        __IO uint16_t PCR7;
        stc_port_pcr_field_t PCR7_f;
    };
    union
    {
        __IO uint16_t PFSR7;
        stc_port_pfsr_field_t PFSR7_f;
    };
    union
    {
        __IO uint16_t PCR8;
        stc_port_pcr_field_t PCR8_f;
    };
    union
    {
        __IO uint16_t PFSR8;
        stc_port_pfsr_field_t PFSR8_f;
    };
    union
    {
        __IO uint16_t PCR9;
        stc_port_pcr_field_t PCR9_f;
    };
    union
    {
        __IO uint16_t PFSR9;
        stc_port_pfsr_field_t PFSR9_f;
    };
    union
    {
        __IO uint16_t PCR10;
        stc_port_pcr_field_t PCR10_f;
    };
    union
    {
        __IO uint16_t PFSR10;
        stc_port_pfsr_field_t PFSR10_f;
    };
    union
    {
        __IO uint16_t PCR11;
        stc_port_pcr_field_t PCR11_f;
    };
    union
    {
        __IO uint16_t PFSR11;
        stc_port_pfsr_field_t PFSR11_f;
    };
    union
    {
        __IO uint16_t PCR12;
        stc_port_pcr_field_t PCR12_f;
    };
    union
    {
        __IO uint16_t PFSR12;
        stc_port_pfsr_field_t PFS12_f;
    };
    union
    {
        __IO uint16_t PCR13;
        stc_port_pcr_field_t PCR13_f;
    };
    union
    {
        __IO uint16_t PFSR13;
        stc_port_pfsr_field_t PFSR13_f;
    };
    union
    {
        __IO uint16_t PCR14;
        stc_port_pcr_field_t PCR14_f;
    };
    union
    {
        __IO uint16_t PFSR14;
        stc_port_pfsr_field_t PFSR14_f;
    };
    union
    {
        __IO uint16_t PCR15;
        stc_port_pcr_field_t PCR15_f;
    };
    union
    {
        __IO uint16_t PFSR15;
        stc_port_pfsr_field_t PFSR15_f;
    };
}gpioe_ctrl_t;

#define GPIO_BASE           (0x40053800ul)
#define GPIO_CTRL_BASE      (GPIO_BASE + 0x400ul)

#define GPIOA_BASE          (GPIO_BASE)
#define GPIOB_BASE          (GPIO_BASE + 0x10ul)
#define GPIOC_BASE          (GPIO_BASE + 0x20ul)
#define GPIOD_BASE          (GPIO_BASE + 0x30ul)
#define GPIOE_BASE          (GPIO_BASE + 0x40ul)
#define GPIOH_BASE          (GPIO_BASE + 0x50ul)

#define GPIOA               ((gpioe_inout_t *)GPIOA_BASE)
#define GPIOB               ((gpioe_inout_t *)GPIOB_BASE)
#define GPIOC               ((gpioe_inout_t *)GPIOC_BASE)
#define GPIOD               ((gpioe_inout_t *)GPIOD_BASE)
#define GPIOE               ((gpioe_inout_t *)GPIOE_BASE)
#define GPIOH               ((gpioe_inout_t *)GPIOH_BASE)

#define GPIOA_CTRL_BASE     (GPIO_CTRL_BASE)
#define GPIOB_CTRL_BASE     (GPIO_CTRL_BASE + 0x40ul)
#define GPIOC_CTRL_BASE     (GPIO_CTRL_BASE + 0x80ul)
#define GPIOD_CTRL_BASE     (GPIO_CTRL_BASE + 0xc0ul)
#define GPIOE_CTRL_BASE     (GPIO_CTRL_BASE + 0x100ul)
#define GPIOH_CTRL_BASE     (GPIO_CTRL_BASE + 0x140ul)

#define GPIOA_CTRL          ((gpioe_ctrl_t *)GPIOA_CTRL_BASE)
#define GPIOB_CTRL          ((gpioe_ctrl_t *)GPIOB_CTRL_BASE)
#define GPIOC_CTRL          ((gpioe_ctrl_t *)GPIOC_CTRL_BASE)
#define GPIOD_CTRL          ((gpioe_ctrl_t *)GPIOD_CTRL_BASE)
#define GPIOE_CTRL          ((gpioe_ctrl_t *)GPIOE_CTRL_BASE)
#define GPIOH_CTRL          ((gpioe_ctrl_t *)GPIOH_CTRL_BASE)

static inline gpioe_inout_t *gpioe_get_base(void *port)
{
    static gpioe_inout_t *gpio_base_list[] = {
        GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOH
    };
    
    return gpio_base_list[(en_port_t)((uint32_t)port)];
}

static inline gpioe_ctrl_t *gpioe_get_ctrl_base(void *port)
{
    static gpioe_ctrl_t *gpio_ctrl_base_list[] = {
        GPIOA_CTRL, GPIOB_CTRL, GPIOC_CTRL, GPIOD_CTRL, GPIOE_CTRL, GPIOH_CTRL
    };
    
    return gpio_ctrl_base_list[(en_port_t)((uint32_t)port)];
}

static inline void gpioe_write_pin(void *port, uint16_t pin, int32_t pin_value)
{
    if(pin_value){
        ((gpioe_inout_t *)port)->POSR = pin;
        
    }else{
        ((gpioe_inout_t *)port)->PORR = pin;
    }
}

static inline void gpioe_set_pin(void *port, uint16_t pin)
{
    ((gpioe_inout_t *)port)->POSR = pin;
}

static inline void gpioe_reset_pin(void *port, uint16_t pin)
{
    ((gpioe_inout_t *)port)->PORR = pin;
}

static inline int32_t gpioe_read_pin(void *port, uint16_t pin)
{
    return (int32_t)(!!(((gpioe_inout_t *)port)->PIDR & pin));
}

static inline uint8_t _get_bit_index(uint8_t pin)
{
    uint8_t index = 1;
    if(pin & 0x0f){
        if(pin & 0x03){
            if(pin & 0x01){
                index = 1;
            }else{
                index = 2;
            }
        }else{
            if(pin & 0x04){
                index = 3;
            }else{
                index = 4;
            }
        }
    }else{
        if(pin & 0x30){
            if(pin & 0x10){
                index = 5;
            }else{
                index = 6;
            }
        }else{
            if(pin & 0x40){
                index = 7;
            }else{
                index = 8;
            }
        }
    }
    
    return index;
}

static inline uint8_t get_bit_index(uint16_t pin)
{
    uint8_t pin_index = 0;

    if(pin & 0x00ff){   //���gpio pin�ڵ�8bit
        pin_index = _get_bit_index(pin) - 1;
    }else{
        pin_index = _get_bit_index(pin >> 8) - 1; 
        pin_index += 8;
    }
    
    return pin_index;
}

static inline void gpioe_set_in(void *port, uint8_t pin_index)
{
    stc_port_pcr_field_t *port_pcr;

    M4_PORT->PWPR = 0xA501u;    //PORT_Unlock();
    port_pcr = (stc_port_pcr_field_t *)((uint32_t)(&((gpioe_ctrl_t *)port)->PCR0) + pin_index * 0x04ul);
    port_pcr->POUTE = 0u;
		port_pcr->PUU = 1;
    M4_PORT->PWPR = 0xA500u;    //PORT_Lock();
}

static inline void gpioe_set_out(void *port, uint8_t pin_index)
{
    static stc_port_pcr_field_t *port_pcr;

    M4_PORT->PWPR = 0xA501u;    //PORT_Unlock();
    port_pcr = (stc_port_pcr_field_t *)((uint32_t)(&((gpioe_ctrl_t *)port)->PCR0) + pin_index * 0x04ul);
    port_pcr->POUTE = 1u;
    M4_PORT->PWPR = 0xA500u;    //PORT_Lock();
}
#endif

#endif

