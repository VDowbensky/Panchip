/*******************************************************************************
 * @note Copyright (C) 2023 Shanghai Panchip Microelectronics Co., Ltd. All rights reserved.
 *
 * @file pan3029.c
 * @brief
 *
 * @history - V0.7, 2024-3
*******************************************************************************/
#include "pan3029_port.h"

uint8_t RadioRxPayload[255];
uint8_t plhd_buf[16];
bool pan3029_irq_trigged_flag = false;
uint8_t reg_agc_value[40] = {0x06,0x00,0xf8,0x06,0x06,0x00,\
                             0xf8,0x06,0x06,0x00,0xf8,0x06,0x06,0x00,0xf8,0x06,0x14,0xc0,0xf9,0x14,0x22,0xd4,\
                             0xf9,0x22,0x30,0xd8,0xf9,0x30,0x3e,0xde,0xf9,0x3e,0x0e,0xff,0x80,0x4f,0x12,0x80,\
                             0x38,0x01
                            };

uint8_t reg_agc_freq400[40] = {0x06,0x00,0xf8,0x06,0x06,0x00,\
                               0xf8,0x06,0x06,0x00,0xf8,0x06,0x06,0x00,0xf8,0x06,0x14,0xc0,0xf9,0x14,0x22,0xd4,\
                               0xf9,0x22,0x30,0xd8,0xf9,0x30,0x3e,0xde,0xf9,0x3e,0x0e,0xff,0x80,0x4f,0x12,0x80,\
                               0x38,0x01
                              };
uint8_t reg_agc_freq800[40] = {0x09,0x80,0xf3,0x09,0x09,0x80,\
                               0xf3,0x09,0x09,0x80,0xf3,0x09,0x09,0x80,0xf3,0x09,0x14,0x06,0xf0,0x14,0x22,0xc6,\
                               0xf1,0x22,0x31,0x73,0xf0,0x31,0x3f,0xde,0xf1,0x3f,0x0e,0xff,0xe0,0x32,0x29,0x80,\
                               0x38,0x01
                              };

const power_ramp_t power_ramp[PAN3029_MAX_RAMP][4]=
{
    {{0x01, 0x0b, 0x01, 0xff}, {0x01, 0x06, 0x00, 0xff}, {0x01, 0x07, 0x01, 0xff}, {0x01, 0x06, 0x01, 0xff}},
    {{0x01, 0x0b, 0x00, 0xff}, {0x01, 0x06, 0x00, 0x00}, {0x01, 0x0b, 0x00, 0xff}, {0x01, 0x0b, 0x01, 0xff}},
    {{0x03, 0x02, 0x01, 0xff}, {0x03, 0x04, 0x01, 0x00}, {0x03, 0x00, 0x01, 0xff}, {0x01, 0x0b, 0x00, 0xff}},
    {{0x03, 0x08, 0x01, 0xff}, {0x03, 0x00, 0x01, 0x01}, {0x03, 0x06, 0x01, 0xff}, {0x03, 0x01, 0x01, 0xff}},
    {{0x03, 0x0b, 0x00, 0xff}, {0x03, 0x00, 0x01, 0xff}, {0x03, 0x0b, 0x00, 0xff}, {0x03, 0x07, 0x01, 0xff}},
    {{0x05, 0x00, 0x01, 0xff}, {0x03, 0x0b, 0x01, 0xff}, {0x05, 0x00, 0x01, 0xff}, {0x03, 0x0b, 0x00, 0xff}},
    {{0x05, 0x03, 0x01, 0xff}, {0x05, 0x01, 0x01, 0x00}, {0x05, 0x03, 0x01, 0xff}, {0x05, 0x00, 0x01, 0xff}},
    {{0x05, 0x08, 0x01, 0xff}, {0x07, 0x00, 0x01, 0x00}, {0x05, 0x0b, 0x01, 0xff}, {0x05, 0x05, 0x01, 0xff}},
    {{0x05, 0x09, 0x00, 0xff}, {0x07, 0x02, 0x01, 0x00}, {0x07, 0x00, 0x01, 0xff}, {0x07, 0x00, 0x01, 0xff}},
    {{0x07, 0x02, 0x01, 0xff}, {0x0d, 0x01, 0x01, 0x00}, {0x07, 0x02, 0x01, 0xff}, {0x07, 0x04, 0x01, 0xff}},
    {{0x07, 0x05, 0x01, 0xff}, {0x0d, 0x02, 0x01, 0x00}, {0x09, 0x00, 0x01, 0xff}, {0x07, 0x0b, 0x01, 0xff}},
    {{0x07, 0x0b, 0x01, 0xff}, {0x0d, 0x04, 0x01, 0x00}, {0x09, 0x02, 0x01, 0xff}, {0x09, 0x03, 0x01, 0xff}},
    {{0x07, 0x0b, 0x00, 0xff}, {0x0d, 0x06, 0x01, 0x00}, {0x09, 0x05, 0x01, 0xff}, {0x0b, 0x02, 0x01, 0xff}},
    {{0x0b, 0x03, 0x01, 0xff}, {0x0d, 0x09, 0x01, 0x00}, {0x0b, 0x03, 0x01, 0xff}, {0x0b, 0x04, 0x01, 0xff}},
    {{0x0b, 0x05, 0x01, 0xff}, {0x15, 0x04, 0x01, 0x01}, {0x0b, 0x06, 0x01, 0xff}, {0x0d, 0x04, 0x01, 0xff}},
    {{0x15, 0x04, 0x01, 0xff}, {0x15, 0x05, 0x01, 0x01}, {0x0b, 0x0b, 0x01, 0xff}, {0x0d, 0x07, 0x01, 0xff}},
    {{0x15, 0x05, 0x01, 0xff}, {0x15, 0x07, 0x01, 0x01}, {0x0d, 0x08, 0x01, 0xff}, {0x15, 0x05, 0x01, 0xff}},
    {{0x15, 0x07, 0x01, 0xff}, {0x15, 0x08, 0x01, 0x01}, {0x15, 0x06, 0x01, 0xff}, {0x15, 0x07, 0x01, 0xff}},
    {{0x15, 0x01, 0x00, 0xff}, {0x15, 0x01, 0x00, 0x02}, {0x15, 0x08, 0x01, 0xff}, {0x15, 0x09, 0x01, 0xff}},
    {{0x15, 0x03, 0x00, 0xff}, {0x15, 0x03, 0x00, 0x02}, {0x15, 0x02, 0x00, 0xff}, {0x15, 0x03, 0x00, 0xff}},
    {{0x15, 0x05, 0x00, 0xff}, {0x15, 0x05, 0x00, 0xff}, {0x15, 0x05, 0x00, 0xff}, {0x15, 0x05, 0x00, 0xff}},
    {{0x15, 0x06, 0x00, 0xff}, {0x15, 0x06, 0x00, 0xff}, {0x15, 0x06, 0x00, 0xff}, {0x15, 0x06, 0x00, 0xff}},
};


const power_ramp_cfg_t power_ramp_cfg[PAN3029_MAX_RAMP+1]=
{
    /*ramp, trim+ldo, bandsel+duty+PAbias(0/1)*/
    {0x01, 0x01, 0x00},
    {0x03, 0x01, 0x01},
    {0x03, 0xf0, 0x30},
    {0x05, 0x01, 0x81},
    {0x05, 0xa1, 0x81},
    {0x05, 0xf0, 0x81},
    {0x07, 0x01, 0x81},
    {0x05, 0x01, 0x80},
    {0x05, 0x31, 0x80},
    {0x07, 0x01, 0x80},
    {0x07, 0x21, 0x80},
    {0x0b, 0x11, 0x80},
    {0x0b, 0x21, 0x80},
    {0x0b, 0x41, 0x80},
    {0x0b, 0x61, 0x80},
    {0x0b, 0x91, 0x80},
    {0x0b, 0xb1, 0x80},
    {0x0d, 0xb1, 0x80},
    {0x0f, 0xb1, 0x80},
    {0x11, 0x50, 0x80},
    {0x15, 0x30, 0x20},
    {0x15, 0x50, 0x70},
    {0x15, 0x60, 0x70},
};


/**
 * @brief read one byte from register in current page
 * @param[in] <addr> register address to write
 * @return value read from register
 */
uint8_t PAN3029_read_reg(uint8_t addr)
{
    uint8_t temreg = 0x00;

    rf_port.spi_cs_low();
    rf_port.spi_readwrite(0x00 | (addr<<1));
    temreg=rf_port.spi_readwrite(0x00);
    rf_port.spi_cs_high();
    return temreg;
}

/**
 * @brief write global register in current page and chick
 * @param[in] <addr> register address to write
 * @param[in] <value> address value to write to rgister
 * @return result
 */
uint8_t PAN3029_write_reg(uint8_t addr,uint8_t value)
{
    uint8_t tmpreg = 0;
    uint8_t addr_w = (0x01 | (addr << 1));

    rf_port.spi_cs_low();
    rf_port.spi_readwrite(addr_w);
    rf_port.spi_readwrite(value);
    rf_port.spi_cs_high();
    if(SPI_WRITE_CHECK)
    {
        tmpreg = PAN3029_read_reg(addr);
        if(tmpreg == value)
        {
            return OK;
        }
        else
        {
            return FAIL;
        }
    } else
    {
        return OK;
    }

}

/**
 * @brief rf send data fifo,send bytes register
 * @param[in] <addr> register address to write
 * @param[in] <buffer> send data buffer
 * @param[in] <size> send data size
 * @return none
 */
void PAN3029_write_fifo(uint8_t addr,uint8_t *buffer,int size)
{
    int i;
    uint8_t addr_w = (0x01 | (addr << 1));

    rf_port.spi_cs_low();
    rf_port.spi_readwrite(addr_w);
    for(i =0; i<size; i++)
    {
        rf_port.spi_readwrite(buffer[i]);
    }
    rf_port.spi_cs_high();
}

/**
 * @brief rf receive data fifo,read bytes from register
 * @param[in] <addr> register address to write
 * @param[in] <buffer> receive data buffer
 * @param[in] <size> receive data size
 * @return none
 */
void PAN3029_read_fifo(uint8_t addr,uint8_t *buffer,int size)
{
    int i;
    uint8_t addr_w = (0x00 | (addr<<1));

    rf_port.spi_cs_low();
    rf_port.spi_readwrite(addr_w);
    for(i =0; i<size; i++)
    {
        buffer[i] = rf_port.spi_readwrite(0x00);
    }
    rf_port.spi_cs_high();
}

/**
 * @brief switch page
 * @param[in] <page> page to switch
 * @return result
 */
uint32_t PAN3029_switch_page(enum PAGE_SEL page)
{
    uint8_t page_sel = 0x00;
    uint8_t tmpreg = 0x00;

    tmpreg = PAN3029_read_reg(REG_SYS_CTL);
    page_sel  = (tmpreg & 0xfc )| page;
    PAN3029_write_reg(REG_SYS_CTL,page_sel);
    if((PAN3029_read_reg(REG_SYS_CTL) &0x03) == page)
    {
        return OK;
    } else
    {
        return FAIL;
    }
}

/**
 * @brief This function write a value to register in specific page
 * @param[in] <page> the page of register
 * @param[in] <addr> register address
 * @param[in] <value> value to write
 * @return result
 */
uint32_t PAN3029_write_spec_page_reg(enum PAGE_SEL page,uint8_t addr,uint8_t value)
{
    if(PAN3029_switch_page(page) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_reg(addr, value) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief read a value to register in specific page
 * @param[in] <page> the page of register
 * @param[in] <addr> register address
 * @return success(register value) or failure
 */
uint8_t PAN3029_read_spec_page_reg(enum PAGE_SEL page,uint8_t addr)
{
    if(PAN3029_switch_page(page) != OK)
    {
        return FAIL;
    }
    return PAN3029_read_reg(addr);
}

/**
 * @brief write continue register valuies(buffer) in specific addr page
 * @param[in] <page> the page of register
 * @param[in] <addr> register start address
 * @param[in] <buffer> values to write
 * @param[in] <len> buffer len
 * @return result
 */
uint32_t PAN3029_write_read_continue_regs(enum PAGE_SEL page,uint8_t addr,uint8_t *buffer,uint8_t len)
{
    uint8_t i,temreg[256];
    uint16_t addr_w;
    if(PAN3029_switch_page(page) != OK)
    {
        return FAIL;
    }

    addr_w = (0x01 | (addr << 1));
    rf_port.spi_cs_low();
    rf_port.spi_readwrite(addr_w);
    for(i=0; i<len; i++)
    {
        rf_port.spi_readwrite(buffer[i]);
    }
    rf_port.spi_cs_high();

    rf_port.spi_cs_low();
    rf_port.spi_readwrite(0x00 | (addr<<1));
    for(i=0; i<len; i++)
    {
        temreg[i] =rf_port.spi_readwrite(0x00);
    }
    rf_port.spi_cs_high();

    for(i=0; i<len; i++)
    {
        if(temreg[i] != buffer[i])
        {
            return FAIL;
        }
    }
    return OK;
}

/**
 * @brief PAN3029 clear all irq
 * @param[in] <none>
 * @return result
 */
uint8_t PAN3029_clr_irq(void)
{
    uint8_t clr_cnt = 0, reg_value;
    uint16_t a = 0, b = 0;
    while(clr_cnt < 3)
    {
        PAN3029_write_spec_page_reg(PAGE0_SEL, 0x6C, 0x3f);//clr irq
        reg_value = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x6C);
        if((reg_value & 0x7f)==0)
        {
            return OK;
        } else {
            clr_cnt++;
            for(a=0; a<1200; a++)
                for(b=0; b<100; b++);
            continue;
        }
    }
    return FAIL;
}

/**
 * @brief get irq status
 * @param[in] <none>
 * @return ira status
 */
uint8_t PAN3029_get_irq(void)
{
    uint8_t tmpreg;

    tmpreg = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x6C);

    return (tmpreg & 0x7f);
}

/**
 * @brief RF 1.2V register refresh,Will not change register values
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_refresh(void)
{
    uint8_t tmpreg = 0;

    tmpreg = PAN3029_read_reg(REG_SYS_CTL);
    tmpreg |= 0x80;
    PAN3029_write_reg(REG_SYS_CTL,tmpreg);

    tmpreg = PAN3029_read_reg(REG_SYS_CTL);
    tmpreg &= 0x7F;

    PAN3029_write_reg(REG_SYS_CTL,tmpreg);
    PAN3029_read_reg(REG_SYS_CTL);
    return OK;
}

/**
 * @brief read packet count register
 * @param[in] <none>
 * @return packet count
 */
uint16_t PAN3029_read_pkt_cnt(void)
{
    uint8_t reg_low, reg_high;
    uint32_t pkt_cnt = 0x00;

    reg_low = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x6c);
    reg_high = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x6d);

    pkt_cnt = (reg_high << 8) | reg_low;
    return pkt_cnt;
}

/**
 * @brief clear packet count register
 * @param[in] <none>
 * @return none
 */
void PAN3029_clr_pkt_cnt(void)
{
    uint8_t tmpreg;

    tmpreg = PAN3029_read_reg(REG_SYS_CTL);
    tmpreg = (tmpreg & 0xbf) | 0x40 ;
    PAN3029_write_reg(REG_SYS_CTL,tmpreg);

    tmpreg = PAN3029_read_reg(REG_SYS_CTL);
    tmpreg = (tmpreg & 0xbf);
    PAN3029_write_reg(REG_SYS_CTL,tmpreg);
}

/**
 * @brief enable AGC function
 * @param[in] <state>
 *			  AGC_OFF/AGC_ON
 * @return result
 */
uint32_t PAN3029_agc_enable(uint32_t state)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE2_SEL, 0x06);

    if(state == AGC_OFF)
    {
        temp_val_2 = (temp_val_1 & 0xfe) | 0x01;
    }
    else
    {
        temp_val_2 = (temp_val_1 & 0xfe) | 0x00;
    }

    if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x06, temp_val_2)  != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief configure AGC function
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_agc_config(void)
{
    if(PAN3029_write_read_continue_regs(PAGE2_SEL, 0x0a, reg_agc_value, 40)  != OK)
    {
        return FAIL;
    }
    if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x34, 0xef)  != OK)
    {
        return FAIL;
    }
    return OK;
}

/**
 * @brief do basic configuration to initialize
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_ft_calibr(void)
{
    uint8_t i,tmpreg,cal[0x26]= {0};

    PAN3029_efuse_on();

    for(i = 17; i<20; i++)
    {
        cal[0x0d+i] = PAN3029_efuse_read_encry_byte(0x3b,0x5aa5,0x0d+i);
    }

    if(PAN3029_efuse_read_encry_byte(0x3b,0x5aa5,0x1c) == 0x5a)
    {
        PAN3029_write_spec_page_reg(PAGE2_SEL,0x3d,0xfd);
        if(cal[0x0d+19]!=0)
            PAN3029_write_spec_page_reg(PAGE0_SEL, 0x45, cal[0x0d+19]);

        if(PAN3029_efuse_read_encry_byte(0x3b,0x5aa5,0x0d) == MODEM_MPA)
        {
            tmpreg = PAN3029_read_spec_page_reg(PAGE3_SEL,0x1c);
            tmpreg &= 0xe0;
            tmpreg |= (cal[0x1e]&0x1f);
            PAN3029_write_spec_page_reg(PAGE3_SEL, 0x1c, tmpreg);
        } else if(PAN3029_efuse_read_encry_byte(0x3b,0x5aa5,0x0d) == MODEM_MPB)
        {
            tmpreg = (0xc0 | (cal[0x1e]&0x1f));
            PAN3029_write_spec_page_reg(PAGE3_SEL, 0x1c, tmpreg);
        }

        PAN3029_write_spec_page_reg(PAGE3_SEL, 0x1d, cal[0x1f]);
    }
    PAN3029_efuse_off();

    return OK;
}

/**
 * @brief do basic configuration to initialize
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_init(void)
{
    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x03, 0x1b)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x04, 0x76)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x06, 0x01)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x15, 0x21)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x31, 0xd0)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x36, 0x66)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x37, 0x6b)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x38, 0xcc)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x39, 0x09)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x3c, 0xb4)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x3e, 0x42)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x6a)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x41, 0x06)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x42, 0xaa)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x48, 0x77)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x49, 0x77)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x4a, 0x77)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x4b, 0x05)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x4f, 0x04)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x50, 0xd2)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x5e, 0x80)  != OK)
    {

        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x03, 0x1b)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x04, 0x76)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x0b, 0x08)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x0f, 0x0a)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x19, 0x00)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x2f, 0xd0)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x43, 0xda)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x03, 0x1b)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x04, 0x76)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x2c, 0xc0)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x2d, 0x27)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x2e, 0x09)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x2f, 0x00)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x30, 0x10)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x03, 0x1b)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x04, 0x76)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x0a, 0x0e)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x0b, 0xcf) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x0c, 0x19)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x0d, 0x98)  != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x12, 0x16) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x13, 0x14) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x16, 0xf4) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x17, 0x01) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x1A, 0x83) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x1f, 0xd9) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x26, 0x20) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief change PAN3029 mode from deep sleep to standby3(STB3)
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_deepsleep_wakeup(void)
{
    uint8_t tmpreg;
    uint8_t rstreg = 0,porreg = 0;

    porreg = PAN3029_read_reg(0x04);
    porreg |= 0x10;
    PAN3029_write_reg(0x04, porreg);
    rf_port.delayus(10);
    porreg &= 0xEF;
    PAN3029_write_reg(0x04, porreg);
    rstreg = PAN3029_read_reg(REG_SYS_CTL);
    rstreg &= 0x7F;
    PAN3029_write_reg(REG_SYS_CTL, rstreg);
    rf_port.delayus(10);
    rstreg |= 0x80;
    PAN3029_write_reg(REG_SYS_CTL, rstreg);
    rf_port.delayus(10);
    rstreg &= 0x7F;
    PAN3029_write_reg(REG_SYS_CTL, rstreg);
    rf_port.delayus(10);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_DEEP_SLEEP) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_SLEEP) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    tmpreg = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x06);
    tmpreg |= (1<<5);
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x06, tmpreg)  != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB1) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x26, 0x2f)  != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_reg(0x04, 0x36) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    rf_port.tcxo_init();

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB2) != OK)
    {
        return FAIL;
    }
    rf_port.delayms(2);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB3) != OK)
    {
        return FAIL;
    }
    else
    {
        rf_port.delayus(10);
        return OK;
    }
}


/**
 * @brief change PAN3029 mode from sleep to standby3(STB3)
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_sleep_wakeup(void)
{
    uint8_t tmpreg;

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_SLEEP) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    tmpreg = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x06);
    tmpreg |= (1<<5);
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x06, tmpreg)  != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB1) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x26, 0x2f)  != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_reg(0x04, 0x36) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    rf_port.tcxo_init();

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB2) != OK)
    {
        return FAIL;
    }
    rf_port.delayms(2);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB3) != OK)
    {
        return FAIL;
    }
    else
    {
        rf_port.delayus(10);
        return OK;
    }
}

/**
 * @brief change PAN3029 mode from standby3(STB3) to deep sleep, PAN3029 should set DCDC_OFF before enter deepsleep
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_deepsleep(void)
{
    uint8_t tmpreg;

    rf_port.delayus(10);
    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB3) != OK)
    {
        return FAIL;
    }
    rf_port.delayms(2);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB2) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB1) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    rf_port.tcxo_close();

    if(PAN3029_write_reg(0x04, 0x06) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_SLEEP) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    tmpreg = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x06);
    tmpreg &= (~(1<<5));
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x06, tmpreg)  != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x26, 0x0f)  != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_DEEP_SLEEP) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}


/**
 * @brief change PAN3029 mode from standby3(STB3) to sleep, PAN3029 should set DCDC_OFF before enter sleep
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_sleep(void)
{
    uint8_t tmpreg;

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB3) != OK)
    {
        return FAIL;
    }
    rf_port.delayms(2);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB2) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB1) != OK)
    {
        return FAIL;
    }

    rf_port.delayus(10);
    rf_port.tcxo_close();

    if(PAN3029_write_reg(0x04, 0x16) != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_SLEEP) != OK)
    {
        return FAIL;
    }

    tmpreg = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x06);
    tmpreg &= (~(1<<5));
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x06, tmpreg)  != OK)
    {
        return FAIL;
    }
    rf_port.delayus(10);

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x26, 0x0f)  != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief set LO frequency
 * @param[in] <lo> LO frequency
 *			  LO_400M / LO_800M
 * @return result
 */
uint32_t PAN3029_set_lo_freq(uint32_t lo)
{
    uint32_t reg_val = 0;
    reg_val = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x3d);
    reg_val &= ~(0x70);

    if(lo == LO_400M)
    {
        reg_val |= 0x10;
    }
    else if(lo == LO_800M)
    {
        reg_val |= 0x00;
    }

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x3d, reg_val)  != OK)
    {
        return FAIL;
    }
    return OK;
}

/**
 * @brief set frequence
 * @param[in] <freq>  RF frequency(in Hz) to set
 * @return result
 */
uint32_t PAN3029_set_freq(uint32_t freq)
{
    uint8_t reg_freq;
    float tmp_var = 0.0;
    int integer_part = 0;
    float fractional_part = 0.0;
    uint8_t lowband_sel = 0;
    int fb,fc;

    if(freq < 800000000)
    {
        PAN3029_write_read_continue_regs(PAGE2_SEL, 0x0a, reg_agc_freq400, 40);
    }
    else
    {
        PAN3029_write_read_continue_regs(PAGE2_SEL, 0x0a, reg_agc_freq800, 40);
    }

    if ( (freq >= freq_405000000) && (freq <= freq_415000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x1a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 4 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_400M);
    }
    else if ( (freq > freq_415000000) && (freq <= freq_430000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x2a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 4 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_400M);
    }
    else if ( (freq > freq_430000000) && (freq <= freq_445000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x3a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 4 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_400M);
    }
    else if ( (freq > freq_445000000) && (freq <= freq_465000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x4a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 4 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_400M);
    }
    else if ( (freq > freq_465000000) && (freq <= freq_485000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x5a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 4 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_400M);
    }
    else if ( (freq > freq_485000000) && (freq <= freq_505000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x6a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 4 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_400M);
    }
    else if ( (freq > freq_505000000) && (freq <= freq_530000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x7a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 4 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_400M);
    }
    else if ( (freq > freq_530000000) && (freq <= freq_565000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x7a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 4 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_400M);
    }
    /****************************800M****************************/
    else if ( (freq >= freq_810000000) && (freq <= freq_830000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x1a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 2 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_800M);
    }
    else if ( (freq > freq_830000000) && (freq <= freq_860000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x2a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 2 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_800M);
    }
    else if ( (freq > freq_860000000) && (freq <= freq_890000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x3a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 2 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_800M);
    }
    else if ( (freq > freq_890000000) && (freq <= freq_930000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x4a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 2 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_800M);
    }
    else if ( (freq > freq_930000000) && (freq <= freq_970000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x5a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 2 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_800M);
    }
    else if ( (freq > freq_970000000) && (freq <= freq_1010000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x6a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 2 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_800M);
    }
    else if ( (freq > freq_1010000000) && (freq <= freq_1060000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x7a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 2 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_800M);
    }
    else if ( (freq > freq_1060000000) && (freq <= freq_1080000000))
    {
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x40, 0x7a)  != OK)
        {
            return FAIL;
        }
        lowband_sel = 0;
        tmp_var = freq * 2 * 1.0 / 32000000;
        PAN3029_set_lo_freq(LO_800M);
    }
    else
    {
        return FAIL;
    }

    integer_part = (int)tmp_var;
    fb = integer_part - 20;
    fractional_part = tmp_var - integer_part;
    fc = (int)(fractional_part * 1600 / (2 * (1 + lowband_sel)));

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x15, (fb & 0xff)) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x16, (fc & 0xff)) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x17, ((fc >> 8) & 0x0f)) != OK)
    {
        return FAIL;
    }

    reg_freq = freq & 0xff;
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x09, reg_freq) != OK)
    {
        return FAIL;
    }

    reg_freq = (freq >> 8) & 0xff;
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x0a, reg_freq) != OK)
    {
        return FAIL;
    }

    reg_freq = (freq >> 16) & 0xff;
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x0b, reg_freq) != OK)
    {
        return FAIL;
    }

    reg_freq = (freq >> 24) & 0xff;
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x0c, reg_freq) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief read frequency(in Hz)
 * @param[in] <none>
 * @return frequency(in Hz)
 */
uint32_t PAN3029_read_freq(void)
{
    uint8_t reg1, reg2, reg3, reg4;
    uint32_t freq = 0x00;

    reg1 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x09);
    reg2 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x0a);
    reg3 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x0b);
    reg4 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x0c);
    freq = (reg4 << 24) | (reg3 << 16) | (reg2 << 8) | reg1;
    return freq;
}

/**
 * @brief calculate tx time
 * @param[in] <size> tx len
 * @return tx time(us)
 */
uint32_t PAN3029_calculate_tx_time(uint8_t size)
{
    uint8_t sf = PAN3029_get_sf();
    uint8_t cr = PAN3029_get_code_rate();
    uint8_t bw = PAN3029_get_bw();
    uint32_t preamble = PAN3029_get_preamble();
    uint32_t ldr = PAN3029_get_ldr();

    const float bw_table[10] = {0,0,0,0,0,0,62.5,125,250,500};

    if(bw < 6 ||bw > 9)
    {
        return 0;
    }

    float symbol_len = (float)(1<<sf)/bw_table[bw]; //symbol length
    float preamble_time;                //preamble time:ms
    float payload_time = 0;             //payload time:ms
    float total_time;                   //total time:ms

    if (sf < 7)
    {
        preamble_time = (preamble+6.25f)*symbol_len;
        payload_time = ceil((float)(size*8-sf*4+36)/((sf-ldr*2)*4));
    }
    else
    {
        preamble_time = (preamble+4.25f)*symbol_len;
        payload_time = ceil((float)(size*8-sf*4+44)/((sf-ldr*2)*4));
    }

    payload_time = payload_time*(cr+4);
    payload_time = payload_time+8;
    payload_time = payload_time*symbol_len;
    total_time = (preamble_time+payload_time)*1000;

    return (int)total_time;
}

/**
 * @brief set bandwidth
 * @param[in] <bw_val> value relate to bandwidth
 *			  BW_62_5K / BW_125K / BW_250K / BW_500K
 * @return result
 */
uint32_t PAN3029_set_bw(uint32_t bw_val)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;
    uint8_t sf_val, ldr_val;

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x0d);
    temp_val_2 = ((temp_val_1 & 0x0F) | (bw_val << 4)) ;
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x0d, temp_val_2) != OK)
    {
        return FAIL;
    }
    sf_val = PAN3029_get_sf();
    if(sf_val == SF_11)
    {
        if(bw_val == BW_62_5K || bw_val == BW_125K)
        {
            ldr_val = LDR_ON;
        }
        else
        {
            ldr_val = LDR_OFF;
        }
    }
    else if(sf_val == SF_12)
    {
        if(bw_val == BW_62_5K || bw_val == BW_125K || bw_val == BW_250K)
        {
            ldr_val = LDR_ON;
        }
        else
        {
            ldr_val = LDR_OFF;
        }
    }
    else
    {
        ldr_val = LDR_OFF;
    }
    rf_set_ldr(ldr_val);

    if(bw_val == BW_62_5K || bw_val == BW_125K || bw_val == BW_250K)
    {
        temp_val_1 = PAN3029_read_spec_page_reg(PAGE2_SEL, 0x3f);
        temp_val_1 =  temp_val_1 | 0x02;
        if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x3f, temp_val_1) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }
    } else
    {
        temp_val_1 = PAN3029_read_spec_page_reg(PAGE2_SEL, 0x3f);
        temp_val_1 =  temp_val_1 & 0xfd;
        if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x3f, temp_val_1) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }
    }
}

/**
 * @brief read bandwidth
 * @param[in] <none>
 * @return bandwidth
 */
uint8_t PAN3029_get_bw(void)
{
    uint8_t tmpreg;

    tmpreg = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x0d);

    return (tmpreg & 0xff) >> 4;
}

/**
 * @brief set spread factor
 * @param[in] <sf> spread factor to set
 *			 SF_5 / SF_6 /SF_7 / SF_8 / SF_9 / SF_10 / SF_11 / SF_12
 * @return result
 */
uint32_t PAN3029_set_sf(uint32_t sf_val)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;
    uint8_t bw_val, ldr_val;

    if(sf_val < 5 || sf_val > 12)
    {
        return FAIL;
    }
    else
    {
        temp_val_1 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x0e);
        temp_val_2 = ((temp_val_1 & 0x0F) | (sf_val << 4)) ;
        if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x0e, temp_val_2) != OK)
        {
            return FAIL;
        }
        else
        {
            bw_val = PAN3029_get_bw();
            if(sf_val == SF_11)
            {
                if(bw_val == BW_62_5K || bw_val == BW_125K)
                {
                    ldr_val = LDR_ON;
                }
                else
                {
                    ldr_val = LDR_OFF;
                }
            }
            else if(sf_val == SF_12)
            {
                if(bw_val == BW_62_5K || bw_val == BW_125K || bw_val == BW_250K)
                {
                    ldr_val = LDR_ON;
                }
                else
                {
                    ldr_val = LDR_OFF;
                }
            }
            else
            {
                ldr_val = LDR_OFF;
            }
            rf_set_ldr(ldr_val);

            return OK;
        }
    }
}

/**
 * @brief read Spreading Factor
 * @param[in] <none>
 * @return Spreading Factor
 */
uint8_t PAN3029_get_sf(void)
{
    uint8_t tmpreg;

    tmpreg = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x0e);

    return (tmpreg & 0xff) >> 4;
}

/**
 * @brief set payload CRC
 * @param[in] <crc_val> CRC to set
 *			  CRC_ON / CRC_OFF
 * @return result
 */
uint32_t PAN3029_set_crc(uint32_t crc_val)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x0e);
    temp_val_2 = ((temp_val_1 & 0xF7) | (crc_val << 3)) ;
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x0e, temp_val_2) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief read payload CRC
 * @param[in] <none>
 * @return CRC status
 */
uint8_t PAN3029_get_crc(void)
{
    uint8_t tmpreg;

    tmpreg = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x0e);

    return (tmpreg & 0x08) >> 3;
}

/**
 * @brief set code rate
 * @param[in] <code_rate> code rate to set
 *			  CODE_RATE_45 / CODE_RATE_46 / CODE_RATE_47 / CODE_RATE_48
 * @return result
 */
uint32_t PAN3029_set_code_rate(uint8_t code_rate)
{
    uint8_t tmpreg = 0;

    tmpreg = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x0d);
    tmpreg &= ~(0x7 << 1);
    tmpreg |= (code_rate << 1);
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x0d, tmpreg) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief get code rate
 * @param[in] <none>
 * @return code rate
 */
uint8_t PAN3029_get_code_rate(void)
{
    uint8_t code_rate = 0;
    uint8_t tmpreg = 0;

    tmpreg = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x0d);
    code_rate = ((tmpreg & 0x0e) >> 1);

    return code_rate;
}

/**
 * @brief set rf mode
 * @param[in] <mode>
 *			  PAN3029_MODE_DEEP_SLEEP / PAN3029_MODE_SLEEP
 *			PAN3029_MODE_STB1 / PAN3029_MODE_STB2
 *			PAN3029_MODE_STB3 / PAN3029_MODE_TX / PAN3029_MODE_RX
 * @return result
 */
uint32_t PAN3029_set_mode(uint8_t mode)
{
    if(PAN3029_write_reg(REG_OP_MODE,mode) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief get rf mode
 * @param[in] <none>
 * @return mode
 *		   PAN3029_MODE_DEEP_SLEEP / PAN3029_MODE_SLEEP
 *		 PAN3029_MODE_STB1 / PAN3029_MODE_STB2
 *		 PAN3029_MODE_STB3 / PAN3029_MODE_TX / PAN3029_MODE_RX
 */
uint8_t PAN3029_get_mode(void)
{
    return PAN3029_read_reg(REG_OP_MODE);
}

/**
 * @brief set rf Tx mode
 * @param[in] <mode>
 *			  PAN3029_TX_SINGLE/PAN3029_TX_CONTINOUS
 * @return result
 */
uint32_t PAN3029_set_tx_mode(uint8_t mode)
{
    uint8_t tmp;
    tmp = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x06);
    tmp = tmp & (~(1 << 2));
    tmp = tmp | (mode << 2);

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x06, tmp) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief set rf Rx mode
 * @param[in] <mode>
 *			  PAN3029_RX_SINGLE/PAN3029_RX_SINGLE_TIMEOUT/PAN3029_RX_CONTINOUS
 * @return result
 */
uint32_t PAN3029_set_rx_mode(uint8_t mode)
{
    uint8_t tmp;
    tmp = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x06);
    tmp = tmp & (~(3 << 0));
    tmp = tmp | (mode << 0);

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x06, tmp) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

uint32_t PAN3029_set_modem_mode(uint8_t mode)
{
    if(mode == MODEM_MODE_NORMAL)
    {
        if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x0b, 0x08) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }
    }
    else if(mode == MODEM_MODE_MULTI_SECTOR)
    {
        if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x0b, 0x18) != OK)
        {
            return FAIL;
        }
        if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x2f, 0x54) != OK)
        {
            return FAIL;
        }
        if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x30, 0x40) != OK)
        {
            return FAIL;
        }
        return OK;
    }
    return FAIL;
}

/**
 * @brief set timeout for Rx. It is useful in PAN3029_RX_SINGLE_TIMEOUT mode
 * @param[in] <timeout> rx single timeout time(in ms)
 * @return result
 */
uint32_t PAN3029_set_timeout(uint32_t timeout)
{
    uint8_t timeout_lsb = 0;
    uint8_t timeout_msb = 0;

    if(timeout > 0xffff)
    {
        timeout = 0xffff;
    }

    timeout_lsb = timeout & 0xff;
    timeout_msb = (timeout >> 8) & 0xff;

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x07, timeout_lsb) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x08, timeout_msb) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief get snr value
 * @param[in] <none>
 * @return snr
 */
float PAN3029_get_snr(void)
{
    float snr_val=0.0;
    uint8_t sig_pow_l, sig_pow_m, sig_pow_h;
    uint8_t noise_pow_l, noise_pow_m, noise_pow_h;
    uint32_t sig_pow_val;
    uint32_t noise_pow_val;
    uint32_t sf_val;

    sig_pow_l = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x74);
    sig_pow_m = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x75);
    sig_pow_h = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x76);
    sig_pow_val = ((sig_pow_h << 16) | (sig_pow_m << 8) | sig_pow_l );

    noise_pow_l = PAN3029_read_spec_page_reg(PAGE2_SEL, 0x71);
    noise_pow_m = PAN3029_read_spec_page_reg(PAGE2_SEL, 0x72);
    noise_pow_h = PAN3029_read_spec_page_reg(PAGE2_SEL, 0x73);
    noise_pow_val = ((noise_pow_h << 16) | (noise_pow_m << 8) | noise_pow_l );

    sf_val = (PAN3029_read_spec_page_reg(PAGE1_SEL, 0x7c) & 0xf0) >> 4;

    if(noise_pow_val == 0)
    {
        noise_pow_val = 1;
    }
    snr_val = (float)(10 * log10((sig_pow_val / pow(2,sf_val)) / noise_pow_val));

    return snr_val;
}

/**
 * @brief get rssi value
 * @param[in] <none>
 * @return rssi
 */
int8_t PAN3029_get_rssi(void)
{
    int8_t rssi_val = 0;

    rssi_val = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x7F);

    return rssi_val;
}


/**
 * @brief current channel energy detection
 * @param[in] <none>
 * @return rssi
 */
int8_t PAN3029_get_channel_rssi(void)
{
    int8_t rssi_pre_read = 0;
    int8_t rssi_energy = 0;

    rssi_pre_read = PAN3029_read_spec_page_reg(PAGE2_SEL, 0x06);
    rssi_pre_read &= ~(1<<2);
    PAN3029_write_spec_page_reg(PAGE2_SEL, 0x06, rssi_pre_read);
    rssi_pre_read = PAN3029_read_spec_page_reg(PAGE2_SEL, 0x06);
    rssi_pre_read |= (1<<2);
    PAN3029_write_spec_page_reg(PAGE2_SEL, 0x06, rssi_pre_read);
    rssi_energy = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x7e);

    return rssi_energy;
}

/**
 * @brief set tx_power
 * @param[in] <tx_power> open gears (range in 1--23（405MHz-565MHz）1-22(868/915MHz))
 * @return result
 */
uint32_t PAN3029_set_tx_power(uint8_t tx_power)
{
    uint8_t tmp_value1, tmp_value2,pa_bias;
    uint32_t freq, pwr_table;

    if(tx_power < PAN3029_MIN_RAMP)
    {
        tx_power = PAN3029_MIN_RAMP;
    }

    freq = PAN3029_read_freq();

    if ( (freq >= freq_405000000) && (freq <= freq_565000000))
    {
        if(tx_power > PAN3029_MAX_RAMP+1)
        {
            tx_power = PAN3029_MAX_RAMP;
        }
        tmp_value1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x1e);
        tmp_value2 = (tmp_value1 & 0xc0) | power_ramp_cfg[tx_power-1].ramp;
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x1e, tmp_value2)  != OK)//modulate wave ramp mode
        {
            return FAIL;
        }

        tmp_value1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x4b);
        tmp_value2 = (tmp_value1 & 0xf0) | (power_ramp_cfg[tx_power-1].pa_ldo >> 4);
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x4b, tmp_value2)  != OK)
        {
            return FAIL;
        }

        tmp_value1 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x22);
        tmp_value2 = (tmp_value1 & 0xfe) | (power_ramp_cfg[tx_power-1].pa_ldo & 0x01);
        if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x22, tmp_value2)  != OK)
        {
            return FAIL;
        }

        if(power_ramp_cfg[tx_power-1].pa_duty != 0x70)
        {
            tmp_value1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x46);
            tmp_value2 = tmp_value1 | 0x04;
            if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x46, tmp_value2)  != OK)
            {
                return FAIL;
            }
        } else {
            tmp_value1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x46);
            tmp_value2 = tmp_value1 & 0xfb;
            if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x46, tmp_value2)  != OK)
            {
                return FAIL;
            }
        }
        PAN3029_efuse_on();
        pa_bias = PAN3029_efuse_read_encry_byte(0x3b,0x5aa5,0x0d+19);
        PAN3029_efuse_off();
        if(pa_bias == 0)
        {
            pa_bias = 8;
        }
        tmp_value1 = pa_bias - (power_ramp_cfg[tx_power-1].pa_duty & 0x0f);
        tmp_value2 = (power_ramp_cfg[tx_power-1].pa_duty & 0xf0) | tmp_value1;
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x45, tmp_value2)  != OK)
        {
            return FAIL;
        }

        return OK;
    } else if ( (freq >= freq_810000000) && (freq <= freq_890000000))
    {
        pwr_table = 2;
    } else if ( (freq >= freq_890000000) && (freq <= freq_1080000000))
    {
        pwr_table = 3;
    } else
    {
        return FAIL;
    }

    if(tx_power > PAN3029_MAX_RAMP)
    {
        tx_power = PAN3029_MAX_RAMP;
    }

    tmp_value1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x1e);
    tmp_value2 = (tmp_value1 & 0xc0) | power_ramp[tx_power-1][pwr_table].ramp;
    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x1e, tmp_value2)  != OK)//modulate wave ramp mode
    {
        return FAIL;
    }

    tmp_value1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x4b);
    tmp_value2 = (tmp_value1 & 0xf0) | power_ramp[tx_power-1][pwr_table].pa_trim;

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x4b, tmp_value2)  != OK)
    {
        return FAIL;
    }

    tmp_value1 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x22);
    tmp_value2 = (tmp_value1 & 0xfe) | power_ramp[tx_power-1][pwr_table].pa_ldo;
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x22, tmp_value2)  != OK)
    {
        return FAIL;
    }

    if(power_ramp[tx_power-1][pwr_table].pa_duty != 0xff)
    {
        tmp_value1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x46);
        tmp_value2 = tmp_value1 | 0x04;
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x46, tmp_value2)  != OK)
        {
            return FAIL;
        }

        tmp_value1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x45);
        tmp_value2 = (tmp_value1 & 0x8f) | (power_ramp[tx_power-1][pwr_table].pa_duty << 4);
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x45, tmp_value2)  != OK)
        {
            return FAIL;
        }
    } else {
        tmp_value1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x46);
        tmp_value2 = tmp_value1 & 0xfb;
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x46, tmp_value2)  != OK)
        {
            return FAIL;
        }
    }
    return OK;
}

/**
 * @brief get tx_power
 * @param[in] <none>
 * @return tx_power if return value is 0,means get tx_power fail
 */
uint32_t PAN3029_get_tx_power(void)
{
    uint8_t open_ramp, pa_trim, pa_ldo, pa_duty, pa_duty_en;
    uint8_t i,pa_bias;
    uint32_t freq, pwr_table;
    open_ramp = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x1e) & 0x3f;
    pa_trim = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x4b) & 0x0f;
    pa_ldo = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x22) & 0x01;
    pa_duty = ((PAN3029_read_spec_page_reg(PAGE0_SEL, 0x45) & 0x70) >> 4);
    pa_duty_en = ((PAN3029_read_spec_page_reg(PAGE0_SEL, 0x46) & 0x04) >> 2);

    freq = PAN3029_read_freq();

    if ( (freq >= freq_405000000) && (freq <= freq_565000000))
    {
        PAN3029_efuse_on();
        pa_bias = PAN3029_efuse_read_encry_byte(0x3b,0x5aa5,0x0d+19);
        PAN3029_efuse_off();
        if(pa_bias == 0)
        {
            pa_bias = 8;
        }
        pa_duty = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x45);
        for(i=0; i<PAN3029_MAX_RAMP+1; i++)
        {
            if(open_ramp == power_ramp_cfg[i].ramp)
            {
                if(((pa_trim << 4) | pa_ldo) == power_ramp_cfg[i].pa_ldo)
                {
                    if((pa_duty_en == true)&&((pa_duty + (power_ramp_cfg[i].pa_duty & 0x0f)) == ((power_ramp_cfg[i].pa_duty & 0xf0) + pa_bias)))
                    {
                        return i+1;
                    } else if((pa_duty_en == false)&&((pa_duty | 0x70) == ((power_ramp_cfg[i].pa_duty & 0xf0) + pa_bias)))
                    {
                        return i+1;
                    }
                }
            }
        }
        return 0;
    } else if ( (freq >= freq_810000000) && (freq <= freq_890000000))
    {
        pwr_table = 2;
    } else if ( (freq >= freq_890000000) && (freq <= freq_1080000000))
    {
        pwr_table = 3;
    } else
    {
        return FAIL;
    }

    for(i=0; i<PAN3029_MAX_RAMP; i++)
    {
        if(open_ramp == power_ramp[i][pwr_table].ramp)
        {
            if((pa_trim == power_ramp[i][pwr_table].pa_trim)&&(pa_ldo == power_ramp[i][pwr_table].pa_ldo))
            {
                if((pa_duty_en == true)&&(pa_duty == power_ramp[i][pwr_table].pa_duty))
                {
                    return i+1;
                } else if((pa_duty_en == false)&&(0xff == power_ramp[i][pwr_table].pa_duty))
                {
                    return i+1;
                }
            }
        }
    }
    return 0;
}

/**
 * @brief set preamble
 * @param[in] <reg> preamble
 * @return result
 */
uint32_t PAN3029_set_preamble(uint16_t reg)
{
    uint8_t tmp_value;
    tmp_value = reg & 0xff;

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x13, tmp_value)  != OK)
    {
        return FAIL;
    }

    tmp_value = (reg >> 8) & 0xff;
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x14, tmp_value)  != OK)
    {
        return FAIL;
    }
    return OK;
}

/**
 * @brief get preamble
 * @param[in] <none>
 * @return preamble
 */
uint32_t PAN3029_get_preamble(void)
{
    uint8_t reg1, reg2;

    reg1 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x13);
    reg2 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x14);

    return (reg2 << 8) | reg1;
}

/**
 * @brief set RF GPIO as input
 * @param[in] <gpio_pin>  pin number of GPIO to be enable
 * @return result
 */
uint32_t PAN3029_set_gpio_input(uint8_t gpio_pin)
{
    uint8_t tmpreg = 0;

    if(gpio_pin < 8)
    {
        tmpreg = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x63);
        tmpreg |= (1 << gpio_pin);
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x63, tmpreg) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }
    }
    else
    {
        tmpreg = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x64);
        tmpreg |= (1 << (gpio_pin - 8));
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x64, tmpreg) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }
    }
}

/**
 * @brief set RF GPIO as output
 * @param[in] <gpio_pin>  pin number of GPIO to be enable
 * @return result
 */
uint32_t PAN3029_set_gpio_output(uint8_t gpio_pin)
{
    uint8_t tmpreg = 0;

    if(gpio_pin < 8)
    {
        tmpreg = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x65);
        tmpreg |= (1 << gpio_pin);
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x65, tmpreg) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }
    }
    else
    {
        tmpreg = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x66);
        tmpreg |= (1 << (gpio_pin - 8));
        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x66, tmpreg) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }
    }
}

/**
 * @brief set GPIO output state, SET or RESET
 * @param[in] <gpio_pin>  pin number of GPIO to be opearted
 *			<state>   0  -  reset,
 *					  1  -  set
 * @return result
 */
uint32_t PAN3029_set_gpio_state(uint8_t gpio_pin, uint8_t state)
{
    uint8_t tmpreg = 0;

    if(gpio_pin < 8)
    {
        tmpreg = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x67);

        if(state == 0)
        {
            tmpreg &= ~(1 << gpio_pin);
        }
        else
        {
            tmpreg |= (1 << gpio_pin);
        }

        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x67, tmpreg) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }
    }
    else
    {
        tmpreg = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x68);

        if(state == 0)
        {
            tmpreg &= ~(1 << (gpio_pin - 8));
        }
        else
        {
            tmpreg |= (1 << (gpio_pin - 8));
        }

        if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x68, tmpreg) != OK)
        {
            return FAIL;
        }
        else
        {
            return OK;
        }
    }
}

/**
 * @brief get GPIO input state
 * @param[in] <gpio_pin>  pin number of GPIO to be opearted
 *			<state>   0  -  low,
 *					  1  -  high
 * @return result
 */
uint8_t PAN3029_get_gpio_state(uint8_t gpio_pin)
{
    uint8_t tmpreg = 0;

    if(gpio_pin < 6)
    {
        tmpreg = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x74);
    }
    else
    {
        tmpreg = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x75);
        gpio_pin -= 6;
    }

    return (tmpreg >> gpio_pin) & 0x01;
}

/**
 * @brief CAD function enable
 * @param[in] <none>
 * @return  result
 */
uint32_t PAN3029_cad_en(uint8_t threshold, uint8_t chirps)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;

    PAN3029_set_gpio_output(11);
    temp_val_1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x5e);
    temp_val_2 = temp_val_1 & 0xbf;
    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x5e, temp_val_2) != OK)
    {
        return FAIL;
    }

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x25);
    temp_val_2 = (temp_val_1 & 0xfc) | chirps;
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x25, temp_val_2) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x0f, threshold) != OK)
    {
        return FAIL;
    }

    return OK;
}

/* @brief CAD function disable
* @param[in] <none>
* @return  result
*/
uint32_t PAN3029_cad_off(void)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x5e);
    temp_val_2 = temp_val_1 | 0x40;
    PAN3029_write_spec_page_reg(PAGE0_SEL, 0x5e, temp_val_2);

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x0f, 0x0a) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief set rf syncword
 * @param[in] <sync> syncword
 * @return result
 */
uint32_t PAN3029_set_syncword(uint32_t sync)
{
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x0f, sync) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief read rf syncword
 * @param[in] <none>
 * @return syncword
 */
uint8_t PAN3029_get_syncword(void)
{
    uint8_t tmpreg;

    tmpreg = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x0f);

    return tmpreg;
}

/**
 * @brief send one packet
 * @param[in] <buff> buffer contain data to send
 * @param[in] <len> the length of data to send
 * @return result
 */
uint32_t PAN3029_send_packet(uint8_t *buff, uint32_t len)
{
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, REG_PAYLOAD_LEN, len) != OK)
    {
        return FAIL;
    }
    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_TX) != OK)
    {
        return FAIL;
    }
    else
    {
        PAN3029_write_fifo(REG_FIFO_ACC_ADDR, buff, len);
        return OK;
    }
}

/**
 * @brief receive a packet in non-block method, it will return 0 when no data got
 * @param[in] <buff> buffer provide for data to receive
 * @return length, it will return 0 when no data got
 */
uint8_t PAN3029_recv_packet(uint8_t *buff)
{
    uint32_t len = 0;

    len = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x7D);
    PAN3029_read_fifo(REG_FIFO_ACC_ADDR, buff, len);

    /* clear rx done irq */
    PAN3029_clr_irq();

    return len;
}

/**
 * @brief set early interruption
 * @param[in] <earlyirq_val> PLHD IRQ to set
 *			  PLHD_IRQ_ON / PLHD_IRQ_OFF
 * @return result
 */
uint32_t PAN3029_set_early_irq(uint32_t earlyirq_val)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x2b);
    temp_val_2 = ((temp_val_1 & 0xbf) | (earlyirq_val << 6)) ;

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x2b, temp_val_2) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief read plhd irq status
 * @param[in] <none>
 * @return plhd irq status
 */
uint8_t PAN3029_get_early_irq(void)
{
    uint8_t tmpreg;

    tmpreg = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x2b);
    return (tmpreg >>6 ) & 0x01;
}

/**
 * @brief set plhd
 * @param[in] <addr> PLHD start addr,Range:0..7f
 *			  <len> PLHD len
 *			  PLHD_LEN8 / PLHD_LEN16
 * @return result
 */
uint32_t PAN3029_set_plhd(uint8_t addr,uint8_t len)
{
    uint8_t temp_val_2;

    temp_val_2 = ((addr & 0x7f) | (len << 7)) ;

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x2e, temp_val_2) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief get plhd len reg value
 * @param[in] <none>
 * @return <len> PLHD_LEN8 / PLHD_LEN16
 */
uint8_t PAN3029_get_plhd_len(void)
{
    uint8_t tmpreg;

    tmpreg = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x2e);

    return ((tmpreg & 0x80) >> 7);
}

/**
 * @brief set plhd mask
 * @param[in] <plhd_val> plhd mask to set
 *			  PLHD_ON / PLHD_OFF
 * @return result
 */
uint32_t PAN3029_set_plhd_mask(uint32_t plhd_val)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x58);
    temp_val_2 = ((temp_val_1 & 0xef) | (plhd_val << 4)) ;

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x58, temp_val_2) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief read plhd mask
 * @param[in] <none>
 * @return plhd mask
 */
uint8_t PAN3029_get_plhd_mask(void)
{
    uint8_t tmpreg;

    tmpreg = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x58);

    return tmpreg;
}

/**
 * @brief receive 8 bytes plhd data
 * @param[in] <buff> buffer provide for data to receive
 * @return result
 */
uint8_t PAN3029_recv_plhd8(uint8_t *buff)
{
    uint32_t i,len = 8;
    for(i = 0; i < len; i++)
    {
        buff[i] = PAN3029_read_spec_page_reg(PAGE2_SEL, 0x76 + i);
    }

    PAN3029_clr_irq();
    return len;
}

/**
 * @brief receive 16 bytes plhd data
 * @param[in] <buff> buffer provide for data to receive
 * @return result
 */
uint8_t PAN3029_recv_plhd16(uint8_t *buff)
{
    uint32_t i,len = 16;
    for(i = 0; i < len; i++)
    {
        if(i<10)
        {
            buff[i] = PAN3029_read_spec_page_reg(PAGE2_SEL, 0x76 + i);
        } else {
            buff[i] = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x76 + i - 10);
        }
    }

    PAN3029_clr_irq();
    return len;
}

/**
 * @brief receive a packet in non-block method, it will return 0 when no data got
 * @param[in] <buff> buffer provide for data to receive
 *			  <len> PLHD_LEN8 / PLHD_LEN16
 * @return result
 */
uint32_t PAN3029_plhd_receive(uint8_t *buf,uint8_t len)
{
    if(len == PLHD_LEN8)
    {
        return PAN3029_recv_plhd8(buf);
    } else if (len == PLHD_LEN16)
    {
        return PAN3029_recv_plhd16(buf);
    }
    return FAIL;
}

/**
 * @brief set dcdc mode, The default configuration is DCDC_OFF, PAN3029 should set DCDC_OFF before enter sleep/deepsleep
 * @param[in] <dcdc_val> dcdc switch
 *			  DCDC_ON / DCDC_OFF
 * @return result
 */
uint32_t PAN3029_set_dcdc_mode(uint32_t dcdc_val)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x24);
    temp_val_2 = ((temp_val_1 & 0xf7) | (dcdc_val << 3)) ;

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x24, temp_val_2) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief set LDR mode
 * @param[in] <mode> LDR switch
 *			  LDR_ON / LDR_OFF
 * @return result
 */
uint32_t PAN3029_set_ldr(uint32_t mode)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x12);
    temp_val_2 = ((temp_val_1 & 0xF7) | (mode << 3)) ;
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x12, temp_val_2) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief get LDR mode
 * @param[in] <none>
 * @return result LDR_ON / LDR_OFF
 */
uint32_t PAN3029_get_ldr(void)
{
    uint8_t tmpreg;

    tmpreg = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x12);

    return (tmpreg >> 3) & 0x01;
}

/**
 * @brief set preamble by Spreading Factor,It is useful in all_sf_search mode
 * @param[in] <sf> Spreading Factor
 * @return result
 */
uint32_t PAN3029_set_all_sf_preamble(uint32_t sf)
{
    switch(sf)
    {
    case 5:
        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x14, 2) != OK)
        {
            return FAIL;
        }

        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x13, 0x6d) != OK)
        {
            return FAIL;
        } else
        {
            return OK;
        }

    case 6:
        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x14, 1) != OK)
        {
            return FAIL;
        }

        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x13, 0x2e) != OK)
        {
            return FAIL;
        } else
        {
            return OK;
        }

    case 7:
        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x14, 0) != OK)
        {
            return FAIL;
        }

        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x13, 130) != OK)
        {
            return FAIL;
        } else
        {
            return OK;
        }

    case 8:
        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x14, 0) != OK)
        {
            return FAIL;
        }

        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x13, 82) != OK)
        {
            return FAIL;
        } else
        {
            return OK;
        }

    case 9:
        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x14, 0) != OK)
        {
            return FAIL;
        }

        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x13, 48) != OK)
        {
            return FAIL;
        } else
        {
            return OK;
        }

    case 10:
        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x14, 0) != OK)
        {
            return FAIL;
        }

        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x13, 24) != OK)
        {
            return FAIL;
        } else
        {
            return OK;
        }

    case 11:
        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x14, 0) != OK)
        {
            return FAIL;
        }

        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x13, 16) != OK)
        {
            return FAIL;
        } else
        {
            return OK;
        }

    case 12:
        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x14, 0) != OK)
        {
            return FAIL;
        }

        if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x13, 12) != OK)
        {
            return FAIL;
        } else
        {
            return OK;
        }

    default:
        return FAIL;
    }
}

/**
 * @brief open all sf auto-search mode
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_set_all_sf_search(void)
{
    uint8_t tmp_val;

    tmp_val = (PAN3029_read_spec_page_reg(PAGE3_SEL, 0x12) | 0x01);

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x12, tmp_val) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x25, 0x04) != OK)
    {
        return FAIL;
    }

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x2d, 0xff) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief close all sf auto-search mode
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_set_all_sf_search_off(void)
{
    uint8_t tmp_val;

    tmp_val = (PAN3029_read_spec_page_reg(PAGE3_SEL, 0x12) & 0xFE);

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x12, tmp_val) != OK)
    {
        return FAIL;
    }

    if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x14, 0) != OK)
    {
        return FAIL;
    }

    if (PAN3029_write_spec_page_reg(PAGE3_SEL, 0x13, 8) != OK)
    {
        return FAIL;
    } else
    {
        return OK;
    }
}

/**
 * @brief RF IRQ server routine, it should be call at ISR of IRQ pin
 * @param[in] <none>
 * @return <none>
 */
void PAN3029_irq_handler(void)
{
    pan3029_irq_trigged_flag = true;
}

/**
 * @brief set carrier_wave mode on,Set BW and SF before calling this function
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_set_carrier_wave_on(void)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;

    if(PAN3029_write_reg(REG_OP_MODE,PAN3029_MODE_STB3) != OK)
    {
        return FAIL;
    }

    PAN3029_set_tx_mode(PAN3029_TX_CONTINOUS);
    PAN3029_set_tx_power(PAN3029_MAX_RAMP);

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x1e);
    temp_val_2 = ((temp_val_1 & 0xFE) | (1 << 0)) ;
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x1e, temp_val_2) != OK)
    {
        return FAIL;
    }
    return OK;
}

/**
 * @brief set carrier_wave mode frequence and send carrier_wave
 * @param[in] <freq> RF frequency(in Hz) to set
 * @return result
 */
uint32_t PAN3029_set_carrier_wave_freq(uint32_t freq)
{
    uint8_t buf[1]= {0};

    if(PAN3029_write_reg(REG_OP_MODE,PAN3029_MODE_STB3) != OK)
    {
        return FAIL;
    }

    if(PAN3029_set_tx_mode(PAN3029_TX_CONTINOUS) != OK)
    {
        return FAIL;
    }

    if(PAN3029_set_freq(freq) != OK)
    {
        return FAIL;
    }

    if(PAN3029_set_ldo_pa_on() != OK)
    {
        return FAIL;
    }

    rf_port.set_tx();

    if(PAN3029_send_packet(buf, 1) != OK)
    {
        return FAIL;
    }
    return OK;
}

/**
 * @brief set carrier_wave mode off
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_set_carrier_wave_off(void)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;

    if(PAN3029_write_reg(REG_OP_MODE, PAN3029_MODE_STB3) != OK)
    {
        return FAIL;
    }

    if(PAN3029_set_ldo_pa_off() != OK)
    {
        return FAIL;
    }

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x1e);
    temp_val_2 = ((temp_val_1 & 0xFE) | (0 << 0)) ;
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x1e, temp_val_2) != OK)
    {
        return FAIL;
    }
    return OK;
}


/**
 * @brief set mapm mode enable
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_mapm_en(void)
{
    uint8_t tmp_val;

    tmp_val = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x38);
    tmp_val |= 0x01;
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x38, tmp_val) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief set mapm mode disable
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_mapm_dis(void)
{
    uint8_t tmp_val;

    tmp_val = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x38);
    tmp_val &= ~0x01;
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x38, tmp_val) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief set mapm mask
 * @param[in] <mapm_val> mapm mask to set
 *			  MAPM_ON / MAPM_OFF
 * @return result
 */
uint32_t PAN3029_set_mapm_mask(uint32_t mapm_val)
{
    uint8_t temp_val_1;
    uint8_t temp_val_2;

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x58);
    temp_val_2 = ((temp_val_1 & 0xbf) | (mapm_val << 6)) ;

    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x58, temp_val_2) != OK)
    {
        return FAIL;
    }
    else
    {
        return OK;
    }
}

/**
 * @brief get the number of fields
 * @param[in] <none>

 * @return <fn>
 */
uint32_t PAN3029_get_mapm_field_num(void)
{
    uint8_t reg_fn, fn_h, fn_l, fn;

    reg_fn = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x3d);
    fn_h = ((reg_fn>>4) -1) * 15;
    fn_l = (reg_fn & 0x0f) -1;
    fn = fn_h + fn_l;

    return fn;
}

/**
 * @brief set the number of fields(range in 0x01~0xe0)
 * @param[in] <fn> the number of fields you want to set

 * @return result
 */
uint32_t PAN3029_set_mapm_field_num(uint8_t fn)
{
    uint8_t reg_fn, fn_h, fn_l;

    fn_h = fn/15 + 1;
    fn_l = fn%15 + 1;
    reg_fn = (fn_h << 4) + fn_l;
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x3d, reg_fn) != OK)
    {
        return FAIL;
    }


    return OK;
}

/**
 * @brief set the unit code word of the field counter represents several fields
 * @param[in] <fnm> the represents number you want to set
              0--1
              1--2
              2--4
              3--8
 * @return result
 */
uint32_t PAN3029_set_mapm_field_num_mux(uint8_t fnm)
{
    uint8_t tmp_val;

    tmp_val = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x37);
    tmp_val &= 0x3f;
    tmp_val |= (fnm << 6);
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x37, tmp_val) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief set the last group function selection
 * @param[in] <group_fun_sel> The last group in the Field, its ADDR position function selection
              0:ordinary address      1:Field counter

 * @return result
 */
uint32_t PAN3029_set_mapm_group_fun_sel(uint8_t gfs)
{
    uint8_t tmp_val;

    tmp_val = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x38);
    tmp_val &= 0xfd;
    tmp_val |= (gfs << 1);
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x38, tmp_val) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief set the number of groups in Field
 * @param[in] <gn> the number of groups

 * @return result
 */
uint32_t PAN3029_set_mapm_group_num(uint8_t gn)
{
    uint8_t tmp_val;

    tmp_val = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x38);
    tmp_val &= 0xf3;
    tmp_val |= (gn << 2);
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x38, tmp_val) != OK)
    {
        return FAIL;
    }

    return OK;
}


/**
 * @brief set the number of Preambles in first groups
 * @param[in] <pgl> The numbers want set to Preambles in first groups(at least 10)

 * @return result
 */
uint32_t PAN3029_set_mapm_firgroup_preamble_num(uint8_t pgl)
{
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x3b, pgl) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief set the number of preambles for groups other than the first group
 * @param[in] <pgn>  the number of Preambles in other groups


 * @return result
 */
uint32_t PAN3029_set_mapm_group_preamble_num(uint8_t pgn)
{
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x3c, pgn) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief set group address1 of mapm mode
 * @param[in] <addr> The value of group address1 you want to set

 * @return result
 */
uint32_t PAN3029_set_mapm_neces_preamble_num(uint16_t pn)
{
    uint8_t tmp_val_1, tmp_val_2, tmp_val_3;

    tmp_val_1 = PAN3029_read_spec_page_reg(PAGE1_SEL, 0x39);
    tmp_val_2 = (tmp_val_1&0xf0) | (pn>>8);

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x39, tmp_val_2) != OK)
    {
        return FAIL;
    }

    tmp_val_3 = pn & 0xff;

    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x3a, tmp_val_3) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief set group address4 of mapm mode
 * @param[in] <addr> The value of group address4 you want to set

 * @return result
 */
uint32_t PAN3029_set_mapm_addr(uint8_t addr_no, uint8_t addr)
{
    if(PAN3029_write_spec_page_reg(PAGE1_SEL, 0x3e + addr_no, addr) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief calculate mapm preamble can sleep time
 * @param[in] <none>
 * @return sleeptime(ms)
 */
uint32_t PAN3029_calculate_mapm_preambletime(stc_mapm_cfg_t *mapm_cfg, uint32_t one_chirp_time)
{
    uint8_t fnm, gn, pgn, pg1, fn, pn;
    uint16_t one_field_chirp, chirp_num;
    uint32_t preamble_time;

    pn = mapm_cfg->pn;
    pgn = mapm_cfg->pgn;
    pg1 = mapm_cfg->pg1;
    gn = mapm_cfg->gn;
    fnm = mapm_cfg->fnm;
    fn = mapm_cfg->fn;
    one_field_chirp = pg1+2 + (pgn+2) * gn;
    chirp_num = (1 << fnm) * fn * one_field_chirp + pn - one_field_chirp;
    preamble_time = one_chirp_time * chirp_num;

    return preamble_time/1000;
}

/**
 * @brief efuse function enable
 * @param[in] <none>
 * @return  result
 */
uint32_t PAN3029_efuse_on(void)
{
    uint8_t temp_val;

    temp_val = PAN3029_read_spec_page_reg(PAGE2_SEL, 0x3e);
    temp_val &= ~(1<<3);
    if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x3e, temp_val) !=OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief efuse function disable
 * @param[in] <none>
 * @return  result
 */
uint32_t PAN3029_efuse_off(void)
{
    uint8_t temp_val;

    temp_val = PAN3029_read_spec_page_reg(PAGE2_SEL, 0x3e);
    temp_val |= (1<<3);
    if(PAN3029_write_spec_page_reg(PAGE2_SEL, 0x3e, temp_val) !=OK)
    {
        return FAIL;
    }

    return OK;

}

/**
 * @brief read efuse area data in unencrypted mode
 * @param[in] <reg_addr> Efuse Register address, customer uses a fixed setting of 0x3c
              <efuse_addr> aaddress want to read data in efuse, customer's usage range is 0x2d~0x7f
 * @return data
 */
uint8_t PAN3029_efuse_read_byte(uint8_t reg_addr, uint8_t efuse_addr)
{
    uint8_t value = 0;
    uint16_t timeout = 100;

    efuse_addr <<= 1;
    PAN3029_switch_page(PAGE2_SEL);
    PAN3029_write_fifo(reg_addr, &efuse_addr, 1);
    do {
        if(PAN3029_read_spec_page_reg(PAGE0_SEL, 0x6c) & 0x80)
        {
            break;
        }
    } while(timeout--);
    PAN3029_switch_page(PAGE2_SEL);
    PAN3029_read_fifo(reg_addr, &value, 1);
    return value;
}

/**
 * @brief write efuse area data in unencrypted mode
 * @param[in] <reg_addr> Efuse Register address, customer uses a fixed setting of 0x3c
              <efuse_addr> address want to write data in efuse, customer's usage range is 0x2d~0x7f
              <value> data want to write in efuse
 * @return <none>
 */
void PAN3029_efuse_write_byte(uint8_t reg_addr, uint8_t efuse_addr, uint8_t value)
{
    uint8_t data_buf[5];
    uint16_t timeout = 100;

    memset(data_buf, 0, sizeof(data_buf));
    data_buf[0] = (efuse_addr << 1) | 0x01;
    data_buf[1] = value;

    PAN3029_switch_page(PAGE2_SEL);
    PAN3029_write_fifo(reg_addr, data_buf, 2);
    do {
        if(PAN3029_read_spec_page_reg(PAGE0_SEL, 0x6c) & 0x80)
        {
            break;
        }
    } while(timeout--);
}

/**
 * @brief read efuse data for initialize
 * @return data
 */
uint8_t PAN3029_efuse_read_encry_byte(uint8_t reg_addr, uint16_t pattern, uint8_t efuse_addr)
{
    uint8_t data_buf[5];
    uint8_t value = 0;
    uint16_t timeout = 100;

    memset(data_buf, 0, sizeof(data_buf));
    data_buf[0] = pattern >> 8;
    data_buf[1] = pattern & 0xff;
    data_buf[2] = efuse_addr << 1;

    PAN3029_switch_page(PAGE2_SEL);
    PAN3029_write_fifo(reg_addr, data_buf, 3);
    do {
        if(PAN3029_read_spec_page_reg(PAGE0_SEL, 0x6c) & 0x80)
        {
            break;
        }
        else
        {
        }
    } while(timeout--);
    PAN3029_switch_page(PAGE2_SEL);
    PAN3029_read_fifo(reg_addr, &value, 1);

    return value;
}
/**
 * @brief enable DCDC calibration
 * @param[in] <calibr_type> calibrated point
              1--ref calibration
              2--zero calibration
              3--imax calibration
 * @return result
 */
uint32_t PAN3029_set_dcdc_calibr_on(uint8_t calibr_type)
{
    if((calibr_type<CALIBR_REF_CMP)||(calibr_type>CALIBR_IMAX_CMP))
    {
        return FAIL;
    }

    uint8_t loop_time = 5;
    uint8_t dcdc_cal = 0;
    uint8_t temp_val_1, temp_val_2;
    uint8_t rd_data, wr_data;
    uint8_t offset_reg_addr;


    if(calibr_type == CALIBR_ZERO_CMP)
    {
        offset_reg_addr = 0x1e;
    }
    else if(calibr_type == CALIBR_REF_CMP)
    {
        offset_reg_addr = 0x1d;
    }
    else if(calibr_type == CALIBR_IMAX_CMP)
    {
        offset_reg_addr = 0x1c;
    }

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x20);
    temp_val_2 = (temp_val_1 & 0x9f) | (calibr_type << 5);

    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x20, temp_val_2) != OK)//calibration on
    {
        return FAIL;
    }

    for(; loop_time>0; loop_time--)
    {
        dcdc_cal |= (0x01 << (loop_time - 1));
        wr_data = 0x80 | dcdc_cal;
        if(PAN3029_write_spec_page_reg(PAGE3_SEL, offset_reg_addr, wr_data) != OK)
        {
            return FAIL;
        }
        rd_data = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x27);
        if (rd_data & 0x01)
        {
            dcdc_cal &= ~(0x01 << (loop_time - 1));
        }
        else
        {
            dcdc_cal |= (0x01 << (loop_time - 1));
        }
        wr_data = 0x80 | dcdc_cal;
        if(PAN3029_write_spec_page_reg(PAGE3_SEL, offset_reg_addr, wr_data) != OK)
        {
            return FAIL;
        }
    }

    return OK;
}

/**
 * @brief disable DCDC calibration
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_set_dcdc_calibr_off(void)
{
    uint8_t temp_val_1, temp_val_2;

    temp_val_1 = PAN3029_read_spec_page_reg(PAGE3_SEL, 0x20);
    temp_val_2 = temp_val_1 & 0x9f;
    if(PAN3029_write_spec_page_reg(PAGE3_SEL, 0x20, temp_val_2) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief enable LDO PA
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_set_ldo_pa_on(void)
{
    uint8_t temp_val;

    temp_val = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x4f);
    temp_val |= 0x08;
    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x4f, temp_val) != OK)
    {
        return FAIL;
    }

    return OK;
}

/**
 * @brief disable LDO PA
 * @param[in] <none>
 * @return result
 */
uint32_t PAN3029_set_ldo_pa_off(void)
{
    uint8_t temp_val;

    temp_val = PAN3029_read_spec_page_reg(PAGE0_SEL, 0x4f);
    temp_val &= 0xf7;
    if(PAN3029_write_spec_page_reg(PAGE0_SEL, 0x4f, temp_val) != OK)
    {
        return FAIL;
    }

    return OK;
}

