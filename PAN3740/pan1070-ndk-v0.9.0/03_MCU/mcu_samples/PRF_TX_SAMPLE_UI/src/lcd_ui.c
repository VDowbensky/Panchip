#include "lcd_ui.h"

#include "lcd_view_ctrl.h"
#include "lcd_matrix.h"
#include "tft_lcd_128x160.h"
#include "sample_config.h"

uint16_t ui_prf_channel = 2478;
int8_t ui_prf_power = 0;
uint8_t ui_prf_phy = PRF_PHY_SELECT;
uint16_t ui_prf_packet_length = 37;
uint8_t ui_prf_conf = PRF_MODE_CONFIG;

extern uint32_t rx_cnt;
extern uint32_t rx_timeout_cnt;
extern uint32_t rx_crc_err_cnt;
extern uint32_t rx_pid_err_cnt;
extern uint32_t rx_len_err_cnt;
extern uint32_t rx_acc_err_cnt;

/* 0 ==> channel; 1 ==> power; 2 ==> phy*/
uint8_t ui_param_index = 0;

bool ui_ready_test = false;

char lcd_str[30] = {0};
char test_str[30] = {0};

test_mode_t test_mode = NORMAL_TEST_MODE;


bool is_297_test_mode(void)
{
    return (test_mode == XN297_TEST_MODE);
}

bool is_b250k_test_mode(void)
{
    return (test_mode == B250K_TEST_MODE);
}

void update_test_result_ui(void)
{
    sprintk(test_str,"rx count:%d      ",rx_cnt);
    LcdDrawEnglish6x12Str(5,97,test_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
    sprintk(test_str,"rx crc err:%d    ",rx_crc_err_cnt);
    LcdDrawEnglish6x12Str(5,109,test_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
    sprintk(test_str,"rx timeout:%d    ",rx_timeout_cnt);
    LcdDrawEnglish6x12Str(5,121,test_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
    sprintk(test_str,"rx len err:%d    ",rx_len_err_cnt);
    LcdDrawEnglish6x12Str(5,133,test_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
    sprintk(test_str,"rx pid err:%d    ",rx_pid_err_cnt);
    LcdDrawEnglish6x12Str(5,145,test_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
    sprintk(test_str,"rx acc err:%d    ",rx_acc_err_cnt);
    LcdDrawEnglish6x12Str(5,157,test_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
}

void update_settings_ui(void)
{
    if(test_mode == XN297_TEST_MODE || test_mode == B250K_TEST_MODE)
    {
        if(test_mode == XN297_TEST_MODE)
        {
            LcdDrawEnglish6x12Str(5,1,"297 TX SIDE    ",TFT_COLOR_BLUE,TFT_COLOR_WHITE);
        }

        if(test_mode == B250K_TEST_MODE)
        {
            LcdDrawEnglish6x12Str(5,1,"B250K TX SIDE    ",TFT_COLOR_BLUE,TFT_COLOR_WHITE);
        }

        sprintk(lcd_str,"                ");
        LcdDrawEnglish6x12Str(5,13,lcd_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
        LcdDrawEnglish6x12Str(5,25,lcd_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
        LcdDrawEnglish6x12Str(5,37,lcd_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
        LcdDrawEnglish6x12Str(5,49,lcd_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
        LcdDrawEnglish6x12Str(5,61,lcd_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
    }else {
        LcdDrawEnglish6x12Str(5,1,"PRF TX SIDE",TFT_COLOR_BLUE,TFT_COLOR_WHITE);
        sprintk(lcd_str,"FREQ:%dMhz",ui_prf_channel);
        LcdDrawEnglish6x12Str(5,13,lcd_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
        sprintk(lcd_str,"POWER:%ddbm    ",ui_prf_power);
        LcdDrawEnglish6x12Str(5,25,lcd_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);

        switch(ui_prf_phy)
        {
            case 1:
                sprintk(lcd_str,"PHY:%dM      ",ui_prf_phy);
                break;
            case 2:
                sprintk(lcd_str,"PHY:%dM      ",ui_prf_phy);
                break;
            case 3:
                sprintk(lcd_str,"PHY:Coded S8");
                break;
            case 4:
                sprintk(lcd_str,"PHY:Coded S2");
                break;
            case 5:
                sprintk(lcd_str,"PHY:250k     ");
                break;
        }
        LcdDrawEnglish6x12Str(5,37,lcd_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);

        sprintk(lcd_str,"Packet Length:%d    ",ui_prf_packet_length);
        LcdDrawEnglish6x12Str(5,49,lcd_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);

        switch(ui_prf_conf)
        {
            case 1:
                sprintk(lcd_str,"1M 250K Dev  ");
                break;
            case 2:
                sprintk(lcd_str,"1M 170k Dev  ");
                break;
            case 3:
                sprintk(lcd_str,"G-250K       ");
                break;
            case 4:
                sprintk(lcd_str,"B-250K       ");
                break;
            case 5:
                sprintk(lcd_str,"Param Err    ");
                break;
        }
        LcdDrawEnglish6x12Str(5,61,lcd_str,TFT_COLOR_BLUE,TFT_COLOR_WHITE);
    }

    switch(ui_param_index)
    {
        case UI_PARAM_INDEX_FREQ:
            sprintk(lcd_str,"Param: FREQ         ");
            break;
        case UI_PARAM_INDEX_POWER:
            sprintk(lcd_str,"Param: Power        ");
            break;
        case UI_PARAM_INDEX_PHY:
            sprintk(lcd_str,"Param: Phy          ");
            break;
        case UI_PARAM_INDEX_LENGTH:
            sprintk(lcd_str,"Param:Packet Length ");
            break;
        case UI_PARAM_INDEX_DEVIATION:
            sprintk(lcd_str,"Deviation mode      ");
            break;
        case UI_PARAM_INDEX_TEST_MODE:
            sprintk(lcd_str,"Test Mode           ");
            break;
        case UI_PARAM_INDEX_START_KEY:
            sprintk(lcd_str,"Start Press Key1  ");
            break;
        default:
            sprintk(lcd_str,"Error Param         ");
            break;
    }
    LcdDrawEnglish6x12Str(5,73,lcd_str,TFT_COLOR_RED,TFT_COLOR_WHITE);

    if(ui_ready_test)
    {
        LcdDrawEnglish6x12Str(20,85,"START TEST",TFT_COLOR_BLUE,TFT_COLOR_WHITE);
    }else{
        LcdDrawEnglish6x12Str(20,85,"STOP TEST",TFT_COLOR_BLUE,TFT_COLOR_WHITE);
    }
}

__IO bool f_key_1 = false;
__IO bool f_key_2 = false;
__IO bool f_key_3 = false;

void key_process(void)
{
        switch(ui_param_index)
    {
        case UI_PARAM_INDEX_FREQ:
            //sprintk(lcd_str,"Param: FREQ         ");
            if(f_key_1)
            {
                f_key_1 = false;
                if(ui_prf_channel<2480)
                    ui_prf_channel++;
            }

            if(f_key_2)
            {
                f_key_2 = false;
                if(ui_prf_channel>2402)
                    ui_prf_channel--;
            }
            break;
        case UI_PARAM_INDEX_POWER:
            //sprintk(lcd_str,"Param: Power        ");
            if(f_key_1)
            {
                f_key_1 = false;
                if(ui_prf_power<9)
                    ui_prf_power++;
            }

            if(f_key_2)
            {
                f_key_2 = false;
                if(ui_prf_power>-40)
                    ui_prf_power--;
            }
            break;
        case UI_PARAM_INDEX_PHY:
            //sprintk(lcd_str,"Param: Phy          ");
            if(f_key_1)
            {
                f_key_1 = false;
                if(ui_prf_phy<5)
                    ui_prf_phy++;
            }

            if(f_key_2)
            {
                f_key_2 = false;
                if(ui_prf_phy>1)
                    ui_prf_phy--;
            }
            break;
        case UI_PARAM_INDEX_LENGTH:
            //sprintk(lcd_str,"Param:Packet Length ");
            if(f_key_1)
            {
                f_key_1 = false;

                if((ui_prf_packet_length+10) <256)
                    ui_prf_packet_length += 10;
            }

            if(f_key_2)
            {
                f_key_2 = false;

                if(ui_prf_packet_length >= 10)
                    ui_prf_packet_length -= 10;
            }
            break;
        case UI_PARAM_INDEX_DEVIATION:
            //sprintk(lcd_str,"prf mode config  ");
            if(f_key_1)
            {
                f_key_1 = false;

                if(ui_prf_conf<4)
                    ui_prf_conf++;
            }

            if(f_key_2)
            {
                f_key_2 = false;

               if(ui_prf_conf>1)
                    ui_prf_conf--;
            }
            break;
        case UI_PARAM_INDEX_TEST_MODE:
            //sprintk(lcd_str,"Start   Press Key1  ");
            if(f_key_1)
            {
                f_key_1 = false;
                if(test_mode < B250K_TEST_MODE)
                    test_mode++;
            }

            if(f_key_2)
            {
               f_key_2 = false;
               if(test_mode > NORMAL_TEST_MODE)
                    test_mode--;
            }
            break;
        case UI_PARAM_INDEX_START_KEY:
            //sprintk(lcd_str,"Start   Press Key1  ");
            if(f_key_1)
            {
                ui_ready_test = true;
            }
            break;
        default:
            //sprintk(lcd_str,"Error Param         ");
            break;
    }

    if(f_key_3)
    {
        f_key_3 = false;
        ui_param_index++;
        ui_param_index %= UI_PARAM_INDEX_MAX;
    }

    update_settings_ui();
}

void GPIO0_IRQHandler(void)
{
    SYS_TEST("GPIO ISR in..\n");

    if (GPIO_GetIntFlag(P0, BIT6)) {
        GPIO_ClrIntFlag(P0, BIT6);
        f_key_1 = true;
        SYS_TEST("P06 occurred\n");
    }

    if (GPIO_GetIntFlag(P0, BIT2)) {
        GPIO_ClrIntFlag(P0, BIT2);
        f_key_3 = true;
        SYS_TEST("P02 occurred\n");
    }

    key_process();
}

void GPIO1_IRQHandler(void)
{
    SYS_TEST("GPIO ISR in..\n");

    if (GPIO_GetIntFlag(P1, BIT2)) {
        GPIO_ClrIntFlag(P1, BIT2);
        f_key_2 = true;
        SYS_TEST("P12 occurred\n");
    }

    key_process();
}

void key_init(void)
{
    CLK_AHBPeriphClockCmd(CLK_AHBPeriph_GPIO, ENABLE);
    GPIO_SetDebounceTime(GPIO_DBCTL_DBCLKSRC_HCLK, GPIO_DBCTL_DBCLKSEL_128);    //set the debounce period to 64 cycles of HCLK

    SYS_SET_MFP(P0, 6, GPIO);
    SYS_SET_MFP(P1, 2, GPIO);
    SYS_SET_MFP(P0, 2, GPIO);

    GPIO_EnableDebounce(P0, BIT6);
    GPIO_EnableDebounce(P1, BIT2);
    GPIO_EnableDebounce(P0, BIT2);

    GPIO_EnableDigitalPath(P0, BIT2|BIT6);
    GPIO_EnableDigitalPath(P1, BIT2);

    GPIO_SetMode(P0, BIT2|BIT6, GPIO_MODE_INPUT);
    GPIO_EnablePullupPath(P0, BIT2|BIT6);

    GPIO_SetMode(P1, BIT2, GPIO_MODE_INPUT);
    GPIO_EnablePullupPath(P1, BIT2);

    GPIO_EnableInt(P0, 6, GPIO_INT_FALLING);
    GPIO_EnableInt(P1, 2, GPIO_INT_FALLING);
    GPIO_EnableInt(P0, 2, GPIO_INT_FALLING);
    NVIC_EnableIRQ(GPIO0_IRQn);
    NVIC_EnableIRQ(GPIO1_IRQn);
}

void init_lcd(void)
{
    key_init();
    InitTftLcd128x160();
    update_settings_ui();
}

void delay_us(uint32_t delay_us)
{
    while (delay_us--) {
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    }
}

void prf_config_updated_from_ui(pan_prf_config_t* config)
{
    config->rf_channel = ui_prf_channel;
    config->tx_power = ui_prf_power;
    config->phy = (prf_phy_t)ui_prf_phy;
    config->mode_conf = (prf_mode_conf_sel_t)ui_prf_conf;
}

void prf_packet_updated_from_ui(panchip_prf_payload_t* config)
{
    config->data_length = ui_prf_packet_length;
}

bool ui_get_ready_test(void)
{
    return ui_ready_test;
}
