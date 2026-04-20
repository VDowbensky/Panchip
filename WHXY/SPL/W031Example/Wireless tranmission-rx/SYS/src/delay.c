#include "delay.h"
#include "system_cw32w031.h"



void delay1ms(uint32_t uint32_tCnt)
{
    uint32_t uint32_tend;

    SysTick->LOAD = 0xFFFFFF;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

    while(uint32_tCnt-- > 0)
    {
        SysTick->VAL  = 0;
        uint32_tend = 0x1000000 - SystemCoreClock/1000;
        while(SysTick->VAL > uint32_tend)
        {
            ;
        }
    }

    SysTick->CTRL = (SysTick->CTRL & (~SysTick_CTRL_ENABLE_Msk));
}


/**
 * \brief   delay100us
 *          delay approximately 100us.
 * \param   [in]  uint32_tCnt
 * \retval  void
 */
void delay100us(uint32_t uint32_tCnt)
{
    uint32_t uint32_tend;

    SysTick->LOAD = 0xFFFFFF;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

    while(uint32_tCnt-- > 0)
    {
        SysTick->VAL = 0;

        uint32_tend = 0x1000000 - SystemCoreClock/10000;
        while(SysTick->VAL > uint32_tend)
        {
            ;
        }
    }

    SysTick->CTRL = (SysTick->CTRL & (~SysTick_CTRL_ENABLE_Msk));
}


/**
 * \brief   delay10us
 *          delay approximately 10us.
 * \param   [in]  uint32_tCnt
 * \retval  void
 */
void delay10us(uint32_t uint32_tCnt)
{
    uint32_t uint32_tend;

    SysTick->LOAD = 0xFFFFFF;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

    while(uint32_tCnt-- > 0)
    {
        SysTick->VAL = 0;

        uint32_tend = 0x1000000 - SystemCoreClock/100000;
        while(SysTick->VAL > uint32_tend)
        {
            ;
        }
    }

    SysTick->CTRL = (SysTick->CTRL & (~SysTick_CTRL_ENABLE_Msk));
}

/**
 * \brief   delay10us
 *          delay approximately 10us.
 * \param   [in]  uint32_tCnt
 * \retval  void
 */
void delay1us(uint32_t uint32_tCnt)
{
    uint32_t uint32_tend;

    SysTick->LOAD = 0xFFFFFF;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

    while(uint32_tCnt-- > 0)
    {
        SysTick->VAL = 0;

        uint32_tend = 0x1000000 - SystemCoreClock/1000000;
        while(SysTick->VAL > uint32_tend)
        {
            ;
        }
    }

    SysTick->CTRL = (SysTick->CTRL & (~SysTick_CTRL_ENABLE_Msk));
}



