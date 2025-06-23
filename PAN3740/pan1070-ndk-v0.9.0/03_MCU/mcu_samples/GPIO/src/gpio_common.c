/**************************************************************************//**
 * @file     gpio_common.c
 * @version  V1.0
 * $Date:    19/09/03 17:00 $
 * @brief    Source file for GPIO test.
 *
 * @note
 * Copyright (C) 2019 Panchip Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "PanSeries.h"
#include "gpio_common.h"

/* Config target pins and auxiliary pins for test */
const uint8_t GPIO_TEST_PINS[][2] =
{
//  {GRP1: In/Out,  GRP2: Out/In}
#ifdef FPGA_MODE
//    {P0_0,          P0_1},
    {P0_2,          P0_3},
    {P0_4,          P0_7},
    {P1_0,          P1_1},
    {P1_2,          P1_3},
    {P1_4,          P1_5},
    {P1_6,          P1_7},
    {P2_0,          P2_1},
    {P2_2,          P2_3},
    {P2_4,          P2_5},
    {P2_6,          P2_7},
    {P3_0,          P3_1},
#else
//    {P0_0,          P0_1},
    {P0_2,          P0_3},
    {P0_4,          P0_7},
    {P1_0,          P1_1},
    {P1_2,          P1_3},
    {P1_4,          P1_5},
    // {P1_6,          P1_7},
    {P2_0,          P2_1},
    {P2_2,          P2_3},
    {P2_4,          P2_5},
    {P2_6,          P2_7},
#endif  /* FPGA_MODE */
};

const uint8_t GPIO_TEST_GRP_NUM = sizeof(GPIO_TEST_PINS) / sizeof(GPIO_TEST_PINS[0]);   //Row (Group Number)


/* GPIO Driver Wrapper Functions */
void GPIO_SetModeByPin(uint8_t PinID, GPIO_ModeDef Mode)
{
    uint8_t port = PinID / 0x10;
    uint8_t bit = PinID % 0x10;

    GPIO_SetMode((GPIO_T*)(P0_BASE + 0x40 * (port)), BIT0 << bit, Mode);

    if (Mode == GPIO_MODE_INPUT || Mode == GPIO_MODE_QUASI)
    {
        if (PinID == P0_0 || PinID == P0_1 || PinID == P0_2)
        {
            // These Pins' DINOFF bits should be synced to 3v area
            CLK_Wait3vSyncReady();
        }
    }
}

void GPIO_EnablePullupPathByPin(uint32_t PinID)
{
    uint8_t port = PinID / 0x10;
    uint8_t bit = PinID % 0x10;

    GPIO_EnablePullupPath((GPIO_T*)(P0_BASE + 0x40 * (port)), BIT0 << bit);
}

void GPIO_DisablePullupPathByPin(uint32_t PinID)
{
    uint8_t port = PinID / 0x10;
    uint8_t bit = PinID % 0x10;

    GPIO_DisablePullupPath((GPIO_T*)(P0_BASE + 0x40 * (port)), BIT0 << bit);
}

void GPIO_EnableIntByPin(uint32_t PinID, GPIO_IntAttrDef IntAttribs)
{
    uint8_t port = PinID / 0x10;
    uint8_t bit = PinID % 0x10;

    GPIO_EnableInt((GPIO_T*)(P0_BASE + 0x40 * (port)), bit, IntAttribs);
}

void GPIO_DisableIntByPin(uint32_t PinID)
{
    uint8_t port = PinID / 0x10;
    uint8_t bit = PinID % 0x10;

    GPIO_DisableInt((GPIO_T*)(P0_BASE + 0x40 * (port)), bit);
}

void GPIO_EnableDebounceByPin(uint32_t PinID)
{
    uint8_t port = PinID / 0x10;
    uint8_t bit = PinID % 0x10;

    GPIO_EnableDebounce((GPIO_T*)(P0_BASE + 0x40 * (port)), BIT0 << bit);
}

void GPIO_DisableDebounceByPin(uint32_t PinID)
{
    uint8_t port = PinID / 0x10;
    uint8_t bit = PinID % 0x10;

    GPIO_DisableDebounce((GPIO_T*)(P0_BASE + 0x40 * (port)), BIT0 << bit);
}

void GPIO_EnableDoutMaskByPin(uint32_t PinID)
{
    uint8_t port = PinID / 0x10;
    uint8_t bit = PinID % 0x10;

    GPIO_EnableDoutMask((GPIO_T*)(P0_BASE + 0x40 * (port)), BIT0 << bit);
}

void GPIO_DisableDoutMaskByPin(uint32_t PinID)
{
    uint8_t port = PinID / 0x10;
    uint8_t bit = PinID % 0x10;

    GPIO_DisableDoutMask((GPIO_T*)(P0_BASE + 0x40 * (port)), BIT0 << bit);
}

void GPIO_SetOutValByPin(uint32_t PinID, bool PinVal)
{
    uint8_t port = PinID / 0x10;
    uint8_t bit = PinID % 0x10;
    uint32_t Data;

    Data = GPIO_GetOutData((GPIO_T*)(P0_BASE + 0x40 * (port)));
    Data = (Data & ~(BIT0 << bit)) | PinVal << bit;

    GPIO_SetOutData((GPIO_T*)(P0_BASE + 0x40 * (port)), Data);
}

bool GPIO_GetOutValByPin(uint32_t PinID)
{
    uint8_t port = PinID / 0x10;
    uint8_t bit = PinID % 0x10;

    return (bool)(GPIO_GetOutData((GPIO_T*)(P0_BASE + 0x40 * (port))) & (BIT0 << bit));
}

uint32_t GPIO_GetIntAttrib(GPIO_T *gpio, uint32_t u32Pin)
{
    uint32_t IntAttr = ((gpio->INTEN & GPIO_INTEN_MASK_BASE << u32Pin) >> u32Pin) |
                        ((gpio->INTTYPE & GPIO_INTTYPE_MASK_BASE << u32Pin) >> u32Pin << 24);
    return IntAttr;
}

char* GPIO_GetIntAttribName(uint32_t IntAttr)
{
    switch (IntAttr)
    {
    case GPIO_INT_RISING:
        return "GPIO_INT_RISING";
    case GPIO_INT_FALLING:
        return "GPIO_INT_FALLING";
    case GPIO_INT_BOTH_EDGE:
        return "GPIO_INT_BOTH_EDGE";
    case GPIO_INT_HIGH:
        return "GPIO_INT_HIGH";
    case GPIO_INT_LOW:
        return "GPIO_INT_LOW";
    default:
        return "GPIO_INT_UNKNOWN";
    }
}

void UTIL_GpioPinPullHigh(uint32_t PinID)
{
    SYS_ConfigMFP(PinID, SYS_MFP_GPIO);
    GPIO_SetModeByPin(PinID, GPIO_MODE_OUTPUT); //Push-pull mode

    P(PinID) = 1;
}

void UTIL_GpioPinPullLow(uint32_t PinID)
{
    SYS_ConfigMFP(PinID, SYS_MFP_GPIO);
    GPIO_SetModeByPin(PinID, GPIO_MODE_OUTPUT); //Push-pull mode

    P(PinID) = 0;
}

void SYS_ConfigMFP(uint8_t PinID, uint32_t PinFunction)
{
    uint8_t tmp_id,port,bit;
    tmp_id = PinID;
    port = tmp_id / 0x10;
    bit = tmp_id % 0x10;

    (&SYS->P0_MFP)[port] = ((&SYS->P0_MFP)[port] & ~(SYS_MFP_P00_Msk << bit)) | PinFunction;
}


IRQn_Type GPIO_GetIRQn(uint8_t PinId)
{
    uint8_t port = PinId / 0x10;
    switch (port) {
        case 0:
            return GPIO0_IRQn;
        case 1:
            return GPIO1_IRQn;
        case 2:
            return GPIO2_IRQn;
        case 3:
            return GPIO3_IRQn;
        default:
            return GPIO0_IRQn;
    }
}

/**
 * @brief       General GPIO IRQ Handler
 * @param       None
 * @return      None
 * @details     The GPIO general IRQ Handler.
 */
void GPIO_IRQHandler(size_t port)
{
    /* To see which interrupt occurred in GPIO Port0 ~ Port5 */
//    for (size_t port = 3; port < 6; port++)
    {
        GPIO_T *gpio = (GPIO_T*)(P0_BASE + 0x40 * (port - 0));

        if (GPIO_GetIntFlag(gpio, 0xFF) != 0)
        {
            for (size_t i = 0; i < 8; i++)
            {
                if (GPIO_GetIntFlag(gpio, BIT(i)))
                {
                    GPIO_ClrIntFlag(gpio, BIT(i));
                    uint32_t Attr = GPIO_GetIntAttrib(gpio, i);
                    if (Attr == GPIO_INT_HIGH || Attr == GPIO_INT_LOW)
                    {
                        GPIO_DisableInt(gpio, i); //Disable INT of curr pin to avoid endless Level Trigger
                    }
                    SYS_TEST("P%d_%d INT occurred, type: %s.\r\n", ((uint32_t)gpio-(uint32_t)P0)/0x40, i, GPIO_GetIntAttribName(Attr));
                }
            }
            GPIO_ClrAllIntFlag(gpio);
        }
    }
}

void GPIO0_IRQHandler(void)
{
    GPIO_IRQHandler(0);
}

void GPIO1_IRQHandler(void)
{
    GPIO_IRQHandler(1);
}

void GPIO2_IRQHandler(void)
{
    GPIO_IRQHandler(2);
}

void GPIO3_IRQHandler(void)
{
    GPIO_IRQHandler(3);
}


T_GPIO_TEST_RESULT (* const GPIO_TestCase[])(uint8_t, uint8_t) =
{
    GPIO_RegisterDefaultValueCheckCase0,
    GPIO_PushPullModeTestCase1,
    GPIO_OpenDrainModeTestCase2,
    GPIO_QuasiBidirectionalModeTestCase3,
    GPIO_DebounceTestCase4,
    GPIO_InterruptTestCase5,
    GPIO_WakeupTestCase6,
    GPIO_DoutWriteMaskTestCase7,
};

void GPIO_TestFunctionEnter(uint16_t TcIdx)
{
    T_GPIO_TEST_RESULT r = GPIO_TST_OK;

    if (TcIdx >= sizeof(GPIO_TestCase) / sizeof(void*))
    {
        SYS_TEST("Error, cannot find Testcase %d!\n", TcIdx);
        return;
    }

#ifdef FPGA_MODE
    if (TcIdx == 4)
    {
        SYS_TEST("Warning, Testcase %d cannot be tested on FPGA!\n", TcIdx);
        return;
    }
#endif

    if (TcIdx == 0 || TcIdx == 6)
    {
        r = (GPIO_TestCase[TcIdx])(NULL, NULL); //Testcase 0/6 does not need parameter.
        if (r != GPIO_TST_OK)
        {
            SYS_TEST("GPIO Test Fail, Fail case: %d, Error Code: %d.\n", TcIdx, r);
        }
        else
        {
            SYS_TEST("GPIO Test OK, Success case: %d.\n", TcIdx);
        }
    }
    else
    {
        for (size_t i = 0; i < GPIO_TEST_GRP_NUM; i++)
        {
            r = (GPIO_TestCase[TcIdx])(GPIO_TEST_PINS[i][0], GPIO_TEST_PINS[i][1]);
            if (r != GPIO_TST_OK)
            {
                SYS_TEST("GPIO Test Fail, Fail case: %d, Error Code: %d, Fail Pin: P%02x\n", TcIdx, r, GPIO_TEST_PINS[i][0]);
            }
            else
            {
                SYS_TEST("GPIO Test OK, Success case: %d, Success Pin: P%02x\n", TcIdx, GPIO_TEST_PINS[i][0]);
            }

            //Switch target and auxiliary pin
            r = (GPIO_TestCase[TcIdx])(GPIO_TEST_PINS[i][1], GPIO_TEST_PINS[i][0]);
            if (r != GPIO_TST_OK)
            {
                SYS_TEST("GPIO Test Fail, Fail case: %d, Error Code: %d, Fail Pin: P%02x\n", TcIdx, r, GPIO_TEST_PINS[i][1]);
            }
            else
            {
                SYS_TEST("GPIO Test OK, Success case: %d, Success Pin: P%02x\n", TcIdx, GPIO_TEST_PINS[i][1]);
            }
        }
    }
}


T_GPIO_TEST_RESULT GPIO_RegisterDefaultValueCheckCase0(uint8_t TargetPin, uint8_t AuxiliaryPin)
{
    SYS_TEST("GPIO Register Default Values:\n");
    SYS_TEST("--------------------------\n");
   SYS_TEST("P0_MODE    = 0x%08x\n",P0->MODE);
   SYS_TEST("P1_MODE    = 0x%08x\n",P1->MODE);
   SYS_TEST("P2_MODE    = 0x%08x\n",P2->MODE);
    // SYS_TEST("P3_MODE    = 0x%08x\n",P0->MODE);
    // SYS_TEST("P4_MODE    = 0x%08x\n",P1->MODE);
    // SYS_TEST("P5_MODE    = 0x%08x\n",P2->MODE);
    SYS_TEST("--------------------------\n");
   SYS_TEST("P0_DINOFF  = 0x%08x\n",P0->DINOFF);
   SYS_TEST("P1_DINOFF  = 0x%08x\n",P1->DINOFF);
   SYS_TEST("P2_DINOFF  = 0x%08x\n",P2->DINOFF);
    // SYS_TEST("P3_DINOFF  = 0x%08x\n",P0->DINOFF);
    // SYS_TEST("P4_DINOFF  = 0x%08x\n",P1->DINOFF);
    // SYS_TEST("P5_DINOFF  = 0x%08x\n",P2->DINOFF);
    SYS_TEST("--------------------------\n");
   SYS_TEST("P0_DOUT    = 0x%08x\n",P0->DOUT);
   SYS_TEST("P1_DOUT    = 0x%08x\n",P1->DOUT);
   SYS_TEST("P2_DOUT    = 0x%08x\n",P2->DOUT);
    // SYS_TEST("P3_DOUT    = 0x%08x\n",P0->DOUT);
    // SYS_TEST("P4_DOUT    = 0x%08x\n",P1->DOUT);
    // SYS_TEST("P5_DOUT    = 0x%08x\n",P2->DOUT);
    SYS_TEST("--------------------------\n");
   SYS_TEST("P0_DATMSK  = 0x%08x\n",P0->DATMSK);
   SYS_TEST("P1_DATMSK  = 0x%08x\n",P1->DATMSK);
   SYS_TEST("P2_DATMSK  = 0x%08x\n",P2->DATMSK);
    // SYS_TEST("P3_DATMSK  = 0x%08x\n",P0->DATMSK);
    // SYS_TEST("P4_DATMSK  = 0x%08x\n",P1->DATMSK);
    // SYS_TEST("P5_DATMSK  = 0x%08x\n",P2->DATMSK);
    SYS_TEST("--------------------------\n");
   SYS_TEST("P0_PIN     = 0x%08x\n",P0->PIN);
   SYS_TEST("P1_PIN     = 0x%08x\n",P1->PIN);
   SYS_TEST("P2_PIN     = 0x%08x\n",P2->PIN);
    // SYS_TEST("P3_PIN     = 0x%08x\n",P0->PIN);
    // SYS_TEST("P4_PIN     = 0x%08x\n",P1->PIN);
    // SYS_TEST("P5_PIN     = 0x%08x\n",P2->PIN);
    SYS_TEST("--------------------------\n");
   SYS_TEST("P0_DBEN    = 0x%08x\n",P0->DBEN);
   SYS_TEST("P1_DBEN    = 0x%08x\n",P1->DBEN);
   SYS_TEST("P2_DBEN    = 0x%08x\n",P2->DBEN);
    // SYS_TEST("P3_DBEN    = 0x%08x\n",P0->DBEN);
    // SYS_TEST("P4_DBEN    = 0x%08x\n",P1->DBEN);
    // SYS_TEST("P5_DBEN    = 0x%08x\n",P2->DBEN);
    SYS_TEST("--------------------------\n");
   SYS_TEST("P0_INTTYPE = 0x%08x\n",P0->INTTYPE);
   SYS_TEST("P1_INTTYPE = 0x%08x\n",P1->INTTYPE);
   SYS_TEST("P2_INTTYPE = 0x%08x\n",P2->INTTYPE);
    // SYS_TEST("P3_INTTYPE = 0x%08x\n",P0->INTTYPE);
    // SYS_TEST("P4_INTTYPE = 0x%08x\n",P1->INTTYPE);
    // SYS_TEST("P5_INTTYPE = 0x%08x\n",P2->INTTYPE);
    SYS_TEST("--------------------------\n");
   SYS_TEST("P0_INTEN   = 0x%08x\n",P0->INTEN);
   SYS_TEST("P1_INTEN   = 0x%08x\n",P1->INTEN);
   SYS_TEST("P2_INTEN   = 0x%08x\n",P2->INTEN);
    // SYS_TEST("P3_INTEN   = 0x%08x\n",P0->INTEN);
    // SYS_TEST("P4_INTEN   = 0x%08x\n",P1->INTEN);
    // SYS_TEST("P5_INTEN   = 0x%08x\n",P2->INTEN);
    SYS_TEST("--------------------------\n");
   SYS_TEST("P0_INTSRC  = 0x%08x\n",P0->INTSRC);
   SYS_TEST("P1_INTSRC  = 0x%08x\n",P1->INTSRC);
   SYS_TEST("P2_INTSRC  = 0x%08x\n",P2->INTSRC);
    // SYS_TEST("P3_INTSRC  = 0x%08x\n",P0->INTSRC);
    // SYS_TEST("P4_INTSRC  = 0x%08x\n",P1->INTSRC);
    // SYS_TEST("P5_INTSRC  = 0x%08x\n",P2->INTSRC);
    SYS_TEST("--------------------------\n");
    SYS_TEST("GPIO_DBCTL = 0x%08x\n",GPIO_DB->DBCTL);
//    SYS_TEST("GPIO_DBCTL addr  = 0x%p\n",&(GPIO_DB->DBCTL));
    SYS_TEST("--------------------------\n");

    return GPIO_TST_OK;
}

