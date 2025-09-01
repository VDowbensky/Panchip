#include "oled.h"
#include "oledfont.h"

#define OLED_DC(level)              level > 0 ? PORT_SetBits(PortB, Pin00) : PORT_ResetBits(PortB, Pin00)
#define OLED_CS(level)              level > 0 ? PORT_SetBits(PortA, Pin05) : PORT_ResetBits(PortA, Pin05)
#define OLED_RST(level)             level > 0 ? PORT_SetBits(PortB, Pin01) : PORT_ResetBits(PortB, Pin01)
#define OLED_SCLK(level)            level > 0 ? PORT_SetBits(PortA, Pin06) : PORT_ResetBits(PortA, Pin06)
#define OLED_SDIN(level)            level > 0 ? PORT_SetBits(PortA, Pin07) : PORT_ResetBits(PortA, Pin07)

#define OLED_CMD                0
#define OLED_DATA               1

/* memory */
static uint8_t OLED_GRAM[128][8];

/**
 * @brief write one byte in SSD1306
 * @param[in] <dat> data or order
 * @param[in] <cmd> 1:data 0:order
 * @return result
 */
void OLED_WriteByte(uint8_t dat, uint8_t cmd)
{
    uint8_t i;
    OLED_DC(cmd);

    OLED_CS(0);
    for (i = 0; i < 8; i++)
    {
        OLED_SCLK(0);
        if (dat & 0x80)
            OLED_SDIN(1);
        else
            OLED_SDIN(0);
        OLED_SCLK(1);
        dat <<= 1;
    }
    OLED_CS(1);
    OLED_DC(1);
}

/**
 * @brief Refresh OLED after wright memory
 * @param[in] <none>
 * @return none
 */
void OLED_Refresh(void)
{
    uint8_t i, j;
    for (i = 0; i < 8; i++)
    {
        OLED_WriteByte(0xb0 + i, OLED_CMD);
        OLED_WriteByte(0x00, OLED_CMD);
        OLED_WriteByte(0x10, OLED_CMD);
        for (j = 0; j < 128; j++)
            OLED_WriteByte(OLED_GRAM[j][i], OLED_DATA);
    }
}

/**
 * @brief turn on the OLED
 * @param[in] <none>
 * @return none
 */
void OLED_DisplayOn(void)
{
    OLED_WriteByte(0X8D, OLED_CMD);
    OLED_WriteByte(0X14, OLED_CMD);
    OLED_WriteByte(0XAF, OLED_CMD);
}

/**
 * @brief turn off the OLED
 * @param[in] <none>
 * @return none
 */
void OLED_DisplayOff(void)
{
    OLED_WriteByte(0X8D, OLED_CMD);
    OLED_WriteByte(0X10, OLED_CMD);
    OLED_WriteByte(0XAE, OLED_CMD);
}

/**
 * @brief clear the OLED configuration to initialize
 * @param[in] <none>
 * @return none
 after clear the OLED will be black
 */
void OLED_Clear(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
        for (n = 0; n < 128; n++)
            OLED_GRAM[n][i] = 0x00;
    OLED_Refresh();
}

/**
 * @brief weight memory
 * @param[in] <x> abscissa axis
 * @param[in] <y> vertical axis
 * @param[in] <t> 1:point 0:no point
 * @return none
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t)
{
    uint8_t pos, bx, temp = 0;
    if (x > 127 || y > 63)
        return;
    pos = 7 - y / 8;
    bx = y % 8;
    temp = 1 << (7 - bx);
    if (t)
        OLED_GRAM[x][pos] |= temp;
    else
        OLED_GRAM[x][pos] &= ~temp;
}

/**
 * @brief paint  area
 * @param[in] <x1> abscissa axis of left
 * @param[in] <y1> vertical axis of left
 * @param[in] <x2> abscissa axis of right
 * @param[in] <y2> vertical axis of right
 * @param[in] <dot> 1:point 0:no point
 * @return none
 */
void OLED_ShowFill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot)
{
    uint8_t x, y;
    for (x = x1; x <= x2; x++)
    {
        for (y = y1; y <= y2; y++)
            OLED_DrawPoint(x, y, dot);
    }
    OLED_Refresh();
}

/**
 * @brief show a char on OLED
 * @param[in] <x> abscissa axis
 * @param[in] <y> vertical axis
 * @param[in] <char> char
 * @param[in] <size> size of word 12 0r 16 or 24
* @param[in] <mode> show mode  0:with shadow 1:without shadow
 * @return result
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
    uint8_t temp, t, t1;
    uint8_t y0 = y;
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    chr = chr - ' ';
    for (t = 0; t < csize; t++)
    {
        if (size == 12)
            temp = asc2_1206[chr][t];
        else if (size == 16)
            temp = asc2_1608[chr][t];
        else if (size == 24)
            temp = asc2_2412[chr][t];
        else
            return;
        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
                OLED_DrawPoint(x, y, mode);
            else
                OLED_DrawPoint(x, y, !mode);
            temp <<= 1;
            y++;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

/**
 * @brief calculate m^n
 * @param[in] <m>
 * @param[in] <n>
 * @return result
 */
uint32_t mypow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while (n--)
        result *= m;
    return result;
}

/**
 * @brief show number on OLED
 * @param[in] <x> abscissa axis
 * @param[in] <y> vertical axis
 * @param[in] <num> number (0~4294967295)
 * @param[in] <len> length of number
 * @param[in] <size> size of word 16 0r 12
 * @return result
 */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
    uint8_t t, temp;
    uint8_t enshow = 0;
    for (t = 0; t < len; t++)
    {
        temp = (num / mypow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                OLED_ShowChar(x + (size / 2) * t, y, ' ', size, 1);
                continue;
            }
            else
            {
                enshow = 1;
            }
        }
        OLED_ShowChar(x + (size / 2) * t, y, temp + '0', size, 1);
    }
}

/**
 * @brief show string on OLED
 * @param[in] <x> abscissa axis
 * @param[in] <y> vertical axis
 * @param[in] <p> string
 * @param[in] <size> size of word 16 0r 12
 * @return result
 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *p, uint8_t size)
{
    while ((*p <= '~') && (*p >= ' '))
    {
        if (x > (128 - (size / 2)))
        {
            x = 0;
            y += size;
        }
        if (y > (64 - size))
        {
            y = x = 0;
            OLED_Clear();
        }
        OLED_ShowChar(x, y, *p, size, 1);
        x += size / 2;
        p++;
    }
}

/**
 * @brief do basic configuration to initialize
 * @param[in] <none>
 * @return none
 */
void OLED_Init(void)
{
    stc_port_init_t stcPortInit;

    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;

    PORT_Init(PortB, Pin00, &stcPortInit);
    PORT_Init(PortA, Pin05, &stcPortInit);
    PORT_Init(PortA, Pin06, &stcPortInit);
    PORT_Init(PortA, Pin07, &stcPortInit);
    PORT_Init(PortB, Pin01, &stcPortInit);

    OLED_CS(1);
    OLED_RST(1);
    SysTick_Delay(10);
    OLED_RST(0);
    SysTick_Delay(100);
    OLED_RST(1);
    SysTick_Delay(10);

    OLED_WriteByte(0xAE, OLED_CMD);
    OLED_WriteByte(0xD5, OLED_CMD);
    OLED_WriteByte(0x50, OLED_CMD);
    OLED_WriteByte(0xA8, OLED_CMD);
    OLED_WriteByte(0x3F, OLED_CMD);
    OLED_WriteByte(0xD3, OLED_CMD);
    OLED_WriteByte(0X00, OLED_CMD);
    OLED_WriteByte(0x40, OLED_CMD);

    OLED_WriteByte(0x8D, OLED_CMD);
    OLED_WriteByte(0x14, OLED_CMD);
    OLED_WriteByte(0x20, OLED_CMD);
    OLED_WriteByte(0x02, OLED_CMD);
    OLED_WriteByte(0xA1, OLED_CMD);
    OLED_WriteByte(0xC0, OLED_CMD);
    OLED_WriteByte(0xDA, OLED_CMD);
    OLED_WriteByte(0x12, OLED_CMD);

    OLED_WriteByte(0x81, OLED_CMD);
    OLED_WriteByte(0xEF, OLED_CMD);
    OLED_WriteByte(0xD9, OLED_CMD);
    OLED_WriteByte(0xf1, OLED_CMD);
    OLED_WriteByte(0xDB, OLED_CMD);
    OLED_WriteByte(0x30, OLED_CMD);

    OLED_WriteByte(0xA4, OLED_CMD);
    OLED_WriteByte(0xA6, OLED_CMD);
    OLED_WriteByte(0xAF, OLED_CMD);
    OLED_Clear();
}
