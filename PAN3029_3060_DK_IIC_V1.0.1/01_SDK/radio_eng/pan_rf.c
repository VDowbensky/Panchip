/**
 * @file      pan_rf.c
 * @brief     PAN3029/PAN3060 driver implementation
 * @version   V1.0.1
 * @date      2025-08-18
 * @copyright Panchip Microelectronics Co., Ltd. All rights reserved.
 * @code
 *             ____              ____ _     _
 *            |  _ \ __ _ _ __  / ___| |__ (_)_ __
 *            | |_) / _` | '_ \| |   | '_ \| | '_ \
 *            |  __/ (_| | | | | |___| | | | | |_) |
 *            |_|   \__,_|_| |_|\____|_| |_|_| .__/
 *                                           |_|
 *            (C)2009-2025 PanChip
 * @endcode
 * @author    PanChip
 * @note      The encoding of this file is utf-8.
 */
#include <stdio.h>
#include <string.h>
#include "bsp.h"
#include "pan_param.h"
#include "pan_rf.h"

/**
* @brief PAN3029/3060 Received Data Packet Structure
* @note This structure is used to store received data packets, including data length, data buffer, SNR, and RSSI information.
*/
volatile RfRxPkt_t g_RfRxPkt = {0};

/**
* @brief PAN3029/3060 Configuration Parameter Structure
* @note This structure is used to store PAN3029/3060 configuration parameters, including transmit power, frequency, spreading factor, bandwidth, coding rate, etc.
*/
static volatile RfConfig_t g_RfCfgParams = {0};

/**
* @brief Saves the current RF operation status.
*/
static volatile RfOpState_t g_RfOperatetate;

#pragma push /* Save compiler optimization settings */
#pragma O0 /* !!!Disable compiler optimization of delay function to ensure delay accuracy!!! */
/**
* @brief Microsecond delay function
* @param us Delay in microseconds
* @note The specific implementation of this function needs to be modified according to the actual hardware platform.
* @note Ensure that RF_DelayUs(1) is greater than or equal to 1us.
*/
void RF_DelayUs(uint32_t us)
{
  volatile int i, j;
  for (i = 0; i < us; i++)
  {
    for (j = 0; j < 12; j++)
    {
      __NOP(); /* Each loop is about 1 cycle */
    }
  }
}

/**
* @brief Millisecond delay function
* @param ms Delay in milliseconds
* @note The specific implementation of this function needs to be modified according to the actual hardware platform.
* @note Ensure that RF_DelayMs(1) is greater than or equal to 1ms.
*/
void RF_DelayMs(uint32_t ms)
{
  RF_DelayUs(ms * 1000); /* Call the microsecond delay function */
}
#pragma pop /* Restore compiler optimization settings */

/**
* @brief I2C start signal
* @param None
* @return None
* @note Before calling this function, ensure that the SDA and SCL lines are high; otherwise, a bus error may occur.
* @note After calling this function, both the SDA and SCL lines will be pulled low, waiting for the host to send further data.
* @note When the SCL line is high, pulling the SDA line low generates a start signal. The timing is as follows:
* SCL: ‾‾‾‾‾\___
* SDA: ‾‾‾‾\_____
*/
static void I2C_Start(void)
{
  /* TODO: Depending on your hardware implementation, the following is a software I2C implementation example. */
  I2C_DELAY(); /* Delay. If using a low-speed MCU, the delay statement can be deleted. */
  I2C_SDA_OUTPUT; /* Set SDA to output mode. */
  I2C_SDA_HIGH; /* Pull the SDA line high. */
  I2C_SCK_HIGH; /* Pull the SCK line high. */
  I2C_DELAY();
  I2C_SDA_LOW; /* Pull the SDA line low to generate a start signal. */
  I2C_DELAY();
  I2C_SCK_LOW; /* Pull the SCK line low to prepare for data transmission. */
  I2C_DELAY();
}

/**
* @brief I2C stop signal
* @param None
* @return None
* @note Before calling this function, ensure that the SCL line is low; otherwise, a bus error may occur.
* @note After calling this function, the SDA and SCL lines are pulled high, and the I2C bus is in an idle state.
* @note When the SCL line is high, the SDA line is pulled high from a low level to generate a stop signal. The timing is as follows:
* SCL: ___/‾‾‾‾‾‾
* SDA: _____/‾‾‾‾
*/
static void I2C_Stop(void)
{
  /* TODO: Depending on your hardware implementation, the following is a software I2C implementation example. */
  I2C_DELAY(); /* Delay. If using a low-speed MCU, the delay statement can be deleted. */
  I2C_SDA_OUTPUT; /* Set SDA to output mode. */
  I2C_SDA_LOW; /* Pull the SDA line low. */
  I2C_DELAY();
  I2C_SCK_HIGH; /* Pull the SCK line high to prepare for sending a stop signal. */
  I2C_DELAY();
  I2C_SDA_HIGH; /* Pull the SDA line high, generating a stop signal */
  I2C_DELAY();
  I2C_SDA_INPUT; /* Set SDA to input mode and release the SDA line */
}

/**
* @brief I2C Send ACK
* @param None
* @return None
* @note Before calling this function, ensure that SCL is low.
* @note After sending the ACK signal, the SDA line is pulled high and the SCL line is pulled low. The timing is as follows:
* SCL: ___/‾‾‾\_____
* SDA: _________/‾‾‾
* ACK signal
*/
static void I2C_SendAck(void)
{
  /* TODO: Depending on your hardware implementation, the following is a software I2C implementation example */
  I2C_SDA_OUTPUT; /* Set SDA to output mode */
  I2C_SDA_LOW; /* Pull the SDA line low to prepare for sending an ACK signal */
  I2C_DELAY();
  I2C_SCK_HIGH; /* Pull the SCL line high to send an ACK signal */
  I2C_DELAY();
  I2C_SCK_LOW; /* Pull the SCL line low to prepare for sending the next bit */
  I2C_DELAY();
  I2C_SDA_HIGH; /* Pull the SDA line high after sending the ACK signal */
}

/**
* @brief I2C Send NACK
* @param None
* @return None
* @note Before calling this function, ensure that the SDA line is low.
* @note After sending a NACK signal, the SDA line will be pulled high and the SCL line will be pulled low. The timing is as follows:
* SCL: _____/‾‾‾\___
* SDA: ___/‾‾‾‾‾‾‾‾
*/
static void I2C_SendNack(void)
{
  /* TODO: Depending on your hardware implementation, the following is a software I2C implementation example */
  I2C_SDA_OUTPUT; /* Set SDA to output mode */
  I2C_SDA_HIGH; /* Pull the SDA line high to prepare for sending a NACK signal */
  I2C_DELAY();
  I2C_SCK_HIGH; /* Pull the SCL line high to send a NACK signal */
  I2C_DELAY();
  I2C_SCK_LOW; /* Pull the SCL line low to prepare for sending the next bit */
  I2C_DELAY();
}

/**
* @brief I2C Write a single byte
* @param[in] Value: Single byte of data to be written
* @return: I2C bus status
* @note: This function waits for an ACK signal. If no ACK signal is received, it returns SI2C_NACK
* @note: After calling this function, the SDA line will be pulled low, indicating that the receiver is ready to receive the next byte.
* Example: Byte write timing (SCL rising edge sampling, write 0xCC):
* SCL: ____|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|___
* SDA: __|‾‾‾‾‾‾‾‾‾‾‾|___________|‾‾‾‾‾‾‾‾‾‾‾|____________________
* 1 1 0 0 1 1 0 0 ACK
* Note: Data is sampled on the rising edge of SCL.
*/
static I2C_Bus_State_t I2C_WriteByte(unsigned char Value)
{
  /* TODO: Depending on your hardware implementation, the following is a software I2C implementation example. */
  unsigned char i;
  uint32_t Timeout = ACK_TIMEOUT_DEF;

  I2C_DELAY(); /* Delay. If using a low-speed MCU, the delay statement can be deleted. */
  I2C_SDA_OUTPUT; /* Set SDA to output mode. */
  I2C_SCK_LOW; /* Pull the SCL line low. */
  I2C_DELAY();
  I2C_SDA_LOW; /* Pull the SDA line low to prepare for data transmission. */
  /* Send data to the I2C bus in order from high to low. */
  for (i = 0; i < 8; i++)
  {
    I2C_DELAY();
    if (Value & 0x80) /* Determine the highest bit data */
    {
      I2C_SDA_HIGH; /* If the highest bit is 1, send a high level */
    }
    else
    {
      I2C_SDA_LOW; /* If the highest bit is 0, send a low level */
    }
    Value <<= 1; /* Shift left one bit, prepare to send the next bit */
    I2C_DELAY();
    I2C_SCK_HIGH; /* Pull the SCL line high to transmit one bit of data */
    I2C_DELAY();
    I2C_SCK_LOW; /* Pull the SCL line low to prepare to send the next bit */
  }
  I2C_SDA_INPUT; /* Set SDA to input mode, prepare to receive the ACK signal */
  I2C_DELAY();
  I2C_SCK_HIGH; /* Pull the SCL line high to prepare to receive the ACK signal */
  I2C_DELAY();
  do /* Wait for ACK signal */
  {
    if (I2C_SDA_STATUS == 0) /* If the SDA line is low, it indicates that the ACK signal has been received */
    {
      I2C_SCK_LOW;
      return SI2C_ACK; /* Return ACK status */
    }
    I2C_DELAY();
  } while (--Timeout > 0);
  I2C_SDA_OUTPUT; /* Set SDA to output mode */
  I2C_DELAY();
  I2C_SCK_LOW; /* Pull the SCL line low to prepare for sending the next bit */
  return SI2C_NACK; /* If the ACK signal was not received after the timeout, return NACK status */
}

/**
* @brief I2C read single byte
* @return the data byte read
* @note This function waits for an ACK signal. If no ACK signal is received, it returns SI2C_NACK
* @note Example: Byte write timing (SCL rising edge sampling, write 0x33):
* SCL: ____|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__
* SDA: ______________|‾‾‾‾‾‾‾‾‾‾‾|__________|‾‾‾‾‾‾‾‾‾‾|_______
* 0 0 1 1 0 0 1 1 ACK
*/
static unsigned char I2C_READ_BYTE(void)
{
  /* TODO: Depending on your hardware implementation, the following is a software I2C implementation example */
  unsigned char i;
  unsigned char Value = 0;

  I2C_DELAY();
  I2C_SDA_INPUT; /* Set SDA to input mode and prepare to receive data */
  I2C_DELAY();
  I2C_SCK_LOW; /* Pull the SCL line low and prepare to receive data */
  I2C_DELAY();
  /* Receive 8-bit data in MSB first format */
  /* When reading data, the SDA line is in input mode and the SCL line is in output mode */
  for (i = 0; i < 8; i++)
  {
    I2C_SCK_HIGH; /* Pull the SCL line high, triggering data on the rising edge */
    I2C_DELAY();
    Value <<= 1; /* Shift the variable left one bit to prepare for the next bit */
    if (I2C_SDA_STATUS) /* Check the SDA pin level */
    {
      Value |= 0x01; /* If the SDA pin is high, set the lowest bit of Value to 1 */
    }
    I2C_SCK_LOW; /* Pull the SCL line low to prepare for receiving the next bit */
    I2C_DELAY();
  }
  return Value;
}

/**
* @brief I2C write data
* @param Addr Address to write
* @param Buf Data buffer to write
* @param Len Length of data to write
* @return Status of the write operation
*/
static unsigned char I2C_WriteBuf(unsigned char Addr, unsigned char *Buf, unsigned char =Len)
{
  unsigned char i;
  I2C_Bus_State_t Status;

  I2C_Start(); /* Send start signal */

  /* Send device address and read/write bit. Bit 0 is the read/write bit, 0 indicates a write operation, and bits 7 to 1 indicate the device address. */
  Status = I2C_WriteByte(I2C_DEV_ADDR << 1);
  if (Status != SI2C_ACK)
  {
    goto exit;
  }
  /* Send register address to be written. Bit 0 is the read/write bit, 0 indicates a write operation, and bits 7 to 1 indicate the register address. */
  Status = I2C_WriteByte(Addr << 1);
  if (Status != SI2C_ACK)
  {
    goto exit;
  }
  /* Send one or more data to be written to the register. */
  for (i = 0; i < Len; i++)
  {
    Status = I2C_WriteByte(Buf[i]);
    if (Status != SI2C_ACK)
    {
      goto exit;
    }
  }
  Status = SI2C_READY;
  exit:
  I2C_Stop();
  return Status;
}

/**
* @brief I2C read data
* @param Addr Address to read
* @param Buf Buffer to store read data
* @param Len Length of data to read
* @return Read operation status
*/
static unsigned char I2C_ReadBuf(unsigned char Addr, unsigned char *Buf, unsigned char Len)
{
  unsigned char i;
  I2C_Bus_State_t Status;

  I2C_Start(); /* Send start signal */
  /* Send device address and read/write bit. Bit 0 is the read/write bit, 0 indicates a write operation, and bits 7 to 1 indicate the device address. */
  Status = I2C_WriteByte((I2C_DEV_ADDR << 1));
  if (Status != SI2C_ACK) goto exit;
  /* Send the register address to be read. Bit 0 is the read/write bit, 0 indicates a write operation, and bits 7 to 1 indicate the register address. */
  Status = I2C_WriteByte((Addr << 1) | 0x01);
  if (Status != SI2C_ACK) goto exit;
  I2C_Start(); /* Send a repeated start signal. */
  /* Send the device address and read/write bit. Bit 0 is the read/write bit, 1 indicates a read operation, and bits 7 to 1 indicate the device address. */
  Status = I2C_WriteByte((I2C_DEV_ADDR << 1) | 0x01);
  if (Status != SI2C_ACK) goto exit;
  /* Read data, sending a NACK signal for the last byte */
  for (i = 0; i < Len; i++) 
  {
    Buf[i] = I2C_READ_BYTE(); /* Read data */
    if (i != Len - 1) I2C_SendAck(); /* Send an ACK signal to indicate that data continues to be received */
    else I2C_SendNack(); /* Send a NACK signal to indicate that data reception is complete */
  }
  Status = SI2C_READY;
  exit:
  I2C_Stop();
  return Status;
}


/**
* @brief I2C reset, used to clear the error status on the I2C bus
* @param None
* @return None
* @note The MCU may reset or error during I2C bus operation, causing the I2C bus to be in an error state.
* @note This function can reset the I2C bus to normal state to avoid subsequent operation failures.
*/
void I2C_Reset(void)
{
  /* TODO: Depending on your hardware implementation, here's an example of a software I2C implementation. */
  unsigned char Timeout = 10;
  unsigned char i = 0;
  while (Timeout--)
  {
    /* Send 8 clock pulses and a stop signal to reset the I2C bus. */
    for (i = 0; i < 8; i++)
    {
      I2C_SCK_HIGH;
      I2C_DELAY();
      I2C_SCK_LOW;
      I2C_DELAY();
    }
    I2C_Stop();

    if (I2C_SDA_STATUS) /* If the SDA line is high, the I2C bus has been reset. */
    {
      break;
    }
  }
}
/**
* @brief Get the number of trailing zeros in a byte
* @param Value The value to check
* @return The number of trailing zeros
*/
uint8_t __ctz(uint8_t Value)
{
  int i;

  for (i = 0; i < 8; ++i)
  {
    if ((Value >> i) & 1)
    return (uint8_t)i;
  }
  return 0;
}

/**
* @brief Reads a single byte from the specified register.
* @param Addr The register address to read from.
* @return uint8_t The value read from the register.
* @note The SPI_CS_LOW(), SPI_CS_HIGH(), SPI_WriteByte(), and SPI_ReadByte() functions in this function need to be modified based on the actual hardware implementation.
*/
uint8_t RF_ReadReg(uint8_t Addr)
{
  uint8_t Temp;
  I2C_ReadBuf(Addr, &Temp, 1); /* Read register value using I2C */
  return Temp;
}

/**
* @brief Writes a single byte to the specified register
* @param Addr Register address to be written
* @param Value Single byte of data to be written to the register
* @return RF_Err_t Returns the operation result
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
* @note The SPI_CS_LOW(), SPI_CS_HIGH(), SPI_WriteByte()
*, and SPI_ReadByte() functions in this function need to be modified based on the actual hardware implementation.
*/
RF_Err_t RF_WriteReg(uint8_t Addr, uint8_t Value)
{
  I2C_WriteBuf(Addr, &Value, 1); /* Write register value using I2C */
#if USE_RF_REG_CHECK /* Whether to use register readback confirmation function */
  /**
  * This part of the code reads the register value and compares it with the written value. If they are not equal, an error message is printed.
  * This part of the code can be commented out or deleted after debugging.
  */
  {
    uint8_t Temp = RF_ReadReg(Addr);
    if (Temp == Value)
    {
      // printf("Write reg ok: 0x%02x, 0x%02x, 0x%02x\n", Addr, Value, Temp);
      return RF_OK; /* The written value is equal to the read value, and the operation is successful. */
    }
    else
    {
      /* The read value is not equal to the written value, and an error is returned. */
      printf("Write reg fail: 0x%02x, 0x%02x, 0x%02x\n", Addr, Value, Temp);
      return RF_FAIL;
    }
  }
#else
  return RF_OK;
#endif
}

/**
* @brief Write multiple bytes continuously to the specified register area.
* @param Addr The starting address of the register area to be written.
* @param Buffer The buffer pointer to the register to be written.
* @param Size The number of bytes to be written.
* @note The SPI_CS_LOW(), SPI_CS_HIGH(), and SPI_WriteByte() functions in this function need to be modified according to the actual hardware implementation.
*/
void RF_WriteRegs(uint8_t Addr, uint8_t *Buffer, uint8_t Size)
{
  I2C_WriteBuf(Addr, Buffer, Size); /* Read register values using I2C and confirm write success. */
}

/**
* @brief Continuously read multiple bytes from the specified register.
* @param Addr: Register address to read.
* @param Buffer: Buffer pointer to store read data.
* @param Size: Number of bytes to read.
* @note: The SPI_CS_LOW(), SPI_CS_HIGH(), SPI_WriteByte(), and SPI_ReadByte() functions in this function need to be modified based on the actual hardware implementation.
*/
void RF_ReadRegs(uint8_t Addr, uint8_t *Buffer, uint8_t Size)
{
  I2C_ReadBuf(Addr, Buffer, Size); /* Read register values using I2C */
}

/**
* @brief Select register page
* @param Page The register page to select, page range 0-3
* @return RF_Err_t Returns the operation result
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
* @note If the current page is already the required page, no register configuration is required
*/
RF_Err_t RF_SetPage(uint8_t Page)
{
  static uint8_t gCurrPage = 0xFF;
  
  if(gCurrPage == Page) return RF_OK;
  gCurrPage = Page;
  RF_ASSERT(RF_WriteReg(0x00, gCurrPage)); /* Select register page */
  return RF_OK;
}

/**
* @brief Writes a single byte to a register in a specified page
* @param Page Register page to be written, page range 0-3
* @param Addr Register address to be written
* @param Value Single byte of data to be written to the register
* @return RF_Err_t Returns the operation result
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
*/
RF_Err_t RF_WritePageReg(uint8_t Page, uint8_t Addr, uint8_t Value)
{
  RF_SetPage(Page);
  RF_WriteReg(Addr, Value);
  return RF_OK;
}

/**
* @brief Writes multiple bytes to a register in a specified page
* @param Page Register page to be written, page range 0-3
* @param Addr Register address to be written
* @param Buffer Buffer pointer to the register to be written
* @param Size Number of bytes to be written
*/
void RF_WritePageRegs(uint8_t Page, uint8_t Addr, uint8_t *Buffer, uint8_t Size)
{
  RF_SetPage(Page); /* Select register page */
  RF_WriteRegs(Addr, Buffer, Size); /* Write register value */
}

/**
* @brief Read a single byte from a register in the specified page
* @param Page Register page to be read, page range 0-3
* @param Addr Register address to be read
* @return uint8_t Value read from register
*/
uint8_t RF_ReadPageReg(uint8_t Page, uint8_t Addr)
{
  RF_SetPage(Page);
  return RF_ReadReg(Addr);
}

/**
* @brief Read multiple bytes from a register range in the specified page
* @param Page Register page to read, page range 0-3
* @param Addr Register address to read
* @param Buffer Buffer pointer to store read data
* @param Size Number of bytes to read
*/
void RF_ReadPageRegs(uint8_t Page, uint8_t Addr, uint8_t *Buffer, uint8_t Size)
{
  RF_SetPage(Page); /* Select register page */
  RF_ReadRegs(Addr, Buffer, Size); /* Read register value */
}

/**
* @brief Set register bits in the specified page
* @param Page Register page to read, page range 0-3
* @param Addr Register address to set
* @param Mask Bit mask to set
* @return RF_Err_t Returns the operation result
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
*/
RF_Err_t RF_SetPageRegBits(uint8_t Page, uint8_t Addr, uint8_t Mask)
{
  uint8_t Temp;

  RF_SetPage(Page);
  Temp = RF_ReadReg(Addr);
  RF_WriteReg(Addr, Temp | Mask);
  return RF_OK;
}

/**
* @brief Resets the register bits of the specified page
* @param Page The register page to be read, page range 0-3
* @param Addr The register address to be reset
* @param Mask The bit mask to be reset
* @return RF_Err_t Returns the operation result
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
*/
RF_Err_t RF_ResetPageRegBits(uint8_t Page, uint8_t Addr, uint8_t Mask)
{
  uint8_t Temp;

  RF_SetPage(Page); /* Select register page */
  Temp = RF_ReadReg(Addr); /* Read register value */
  RF_WriteReg(Addr, Temp & (~Mask)); /* Clear register bits corresponding to the mask */
  return RF_OK;
}

/**
* @brief Write register bits in the specified page
* @param Page Register page to read, page range 0-3
* @param Addr Register address to write to
* @param Value Value to write to
* @param Mask Bit mask to write to
* @return RF_Err_t Returns the result of the operation
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
* @note This function first clears the register bits corresponding to the mask and then sets the new value.
* @note For example, to set bits 2 and 3 of register 0x08 on page 1 to 0b10, leaving other bits unchanged, you can call
* RF_WritePageRegBits(1, 0x08, 0x02, 0x0C);
* Where Value = 0x02, Mask = 0x0C. Value does not need to be left-shifted because the mask already specifies the bits to be set.
*/
RF_Err_t RF_WritePageRegBits(uint8_t Page, uint8_t Addr, uint8_t Value, uint8_t Mask)
{
  uint8_t Temp;
  uint8_t shift = __ctz(Mask); /* Get the shift value of the mask */

  Value <<= shift; /* Shift the value left to the position corresponding to the mask */
  Value &= Mask; /* AND the value with the mask to ensure that only the bits corresponding to the mask are set */
  RF_SetPage(Page); /* Select the register page */
  Temp = RF_ReadReg(Addr); /* Read the register value */
  RF_WriteReg(Addr, (Temp & (~Mask)) | Value); /* Clear the bits in the register corresponding to the mask, then set the new value. */
  return RF_OK;
}

/**
* @brief Configure GPIO mode
* @param <GpioPin> Pin number
* <GpioMode> GPIO mode
* - GPIO_MODE_INPUT: Input mode
* - GPIO_MODE_OUTPUT: Output mode
* @return RF_Err_t Returns the operation result
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
*/
RF_Err_t RF_ConfigGpio(uint8_t GpioPin, uint8_t GpioMode)
{
  if(GpioMode == GPIO_MODE_INPUT)
  {
    if(GpioPin < 8) RF_ASSERT(RF_SetPageRegBits(0, 0x63, (1 << GpioPin))); 
    else RF_ASSERT(RF_SetPageRegBits(0, 0x64, (1 << (GpioPin - 8)))); 
  } 
  else if(GpioMode == GPIO_MODE_OUTPUT) 
  { 
    if(GpioPin < 8) RF_SetPageRegBits(0, 0x65, (1 << GpioPin)); 
    else  RF_SetPageRegBits(0, 0x66, (1 << (GpioPin - 8)));
  }
  else return RF_FAIL;
  return RF_OK;
}

/**
* @brief Control GPIO output level
* @param <GpioPin> Pin number
* <Level> GPIO level
* - 0: Low level
* - 1: High level
* @return RF_Err_t Returns operation result
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
*/
RF_Err_t RF_WriteGpioLevel(uint8_t GpioPin, uint8_t Level)
{
  if(GpioPin < 8) RF_WritePageRegBits(0, 0x67, Level, (1 << GpioPin));
  else RF_WritePageRegBits(0, 0x68, Level, (1 << (GpioPin - 8)));
  return RF_OK;
}

/**
* @brief Read GPIO level
* @param <GpioPin> Pin number
* @return Read GPIO level
* - 0: Low level
* - 1: High level
*/
uint8_t RF_ReadGpioLevel(uint8_t GpioPin)
{
  uint8_t Temp;

  if(GpioPin < 6) Temp = RF_ReadPageReg(0, 0x74);
  else
  {
    Temp = RF_ReadPageReg(0, 0x75);
    GpioPin -= 6;
  }
  return (bool)((Temp >> GpioPin) & 0x01);
}

/**
* @brief Initializes the PAN3029/3060 antenna control GPIO
* @note This function initializes the PAN3029/3060 antenna control GPIO, configures it to output mode, and sets the initial level to low.
* @note If you use the MCU's GPIO to control the antenna switch, you need to re-adapt this function.
*/
void RF_InitAntGpio(void)
{
  RF_ConfigGpio(MODULE_GPIO_RX, GPIO_MODE_OUTPUT);
  RF_ConfigGpio(MODULE_GPIO_TX, GPIO_MODE_OUTPUT);
  RF_WriteGpioLevel(MODULE_GPIO_RX, 0);
  RF_WriteGpioLevel(MODULE_GPIO_TX, 0);
}

/**
* @brief Turns on the PAN3029/3060's transmitting antenna.
* @note This function turns on the PAN3029/3060's transmitting antenna, setting the TX pin high and the RX pin low.
* @note If you use the MCU's GPIO to control the antenna, you will need to reconfigure this function.
*/
void RF_TurnonTxAnt(void)
{
  RF_WriteGpioLevel(MODULE_GPIO_RX, 0);
  RF_WriteGpioLevel(MODULE_GPIO_TX, 1);
}

/**
* @brief Turns on the PAN3029/3060's receiving antenna.
* @note This function turns on the PAN3029/3060's receiving antenna, setting the RX pin high and the TX pin low.
* @note If you use the MCU's GPIO to control the antenna, you will need to reconfigure this function.
*/
void RF_TurnonRxAnt(void)
{
  RF_WriteGpioLevel(MODULE_GPIO_TX, 0);
  RF_WriteGpioLevel(MODULE_GPIO_RX, 1);
}

/**
* @brief Turn off the PAN3029/3060 antenna.
* @note This function turns off the PAN3029/3060 antenna, setting both the RX and TX pins to low.
* @note If you use the MCU's GPIO to control the antenna switch, you will need to reconfigure this function.
*/
void RF_ShutdownAnt(void)
{
  RF_WriteGpioLevel(MODULE_GPIO_RX, 0);
  RF_WriteGpioLevel(MODULE_GPIO_TX, 0);
}

/**
* @brief Initialize the TCXO control GPIO.
* @note This function initializes the PAN3029/3060's TCXO control GPIO, configuring it in output mode and setting the initial level to high. * @note If you use the MCU's GPIO to control the TCXO on/off, you need to reconfigure this function.
*/
void RF_InitTcxoGpio(void)
{
  RF_ConfigGpio(MODULE_GPIO_TCXO, GPIO_MODE_OUTPUT);
  RF_WriteGpioLevel(MODULE_GPIO_TCXO, 1);
}

/**
* @brief Turns on the TCXO power supply.
* @note This function turns on the PAN3029/3060's TCXO and sets the TCXO pin to a high level.
* @note If you use the MCU's GPIO to control the TCXO on/off, you need to reconfigure this function.
*/
void RF_TurnonTcxo(void)
{
  RF_WriteGpioLevel(MODULE_GPIO_TCXO, 1);
}

/**
* @brief Turns off the TCXO power supply.
* @note This function turns off the PAN3029/3060's TCXO and sets the TCXO pin to a low level.
* @note If you use the MCU's GPIO to control the TCXO, you will need to reconfigure this function.
*/
void RF_TurnoffTcxo(void)
{
  RF_WriteGpioLevel(MODULE_GPIO_TCXO, 0);
}

/**
* @brief Enable LDO PA
*/
void RF_TurnonLdoPA(void)
{
  RF_SetPageRegBits(0, 0x4F, 0x08);
}

/*
* @brief Disable LDO PA
*/
void RF_TurnoffLdoPA(void)
{
  RF_ResetPageRegBits(0, 0x4F, 0x08);
}

/**
* @brief Turn off internal and external PAs
*/
void RF_TurnoffPA(void)
{
  RF_TurnoffLdoPA(); /* Turn off the internal PA */
  RF_ShutdownAnt(); /* Turn off the external PA */
  /* After transmission is complete, if configured in DCDC power mode, switch back to DCDC power mode */
  if(g_RfCfgParams.RegulatorMode == USE_DCDC) RF_WritePageReg(3, 0x24, 0x08);
}

/**
* @brief Turn on the internal and external PAs
*/
void RF_TurnonPA(void)
{
  /* If currently in DCDC power mode, switch to LDO power mode before transmission */
  if(g_RfCfgParams.RegulatorMode == USE_DCDC) RF_WritePageReg(3, 0x24, 0x00);
  RF_TurnonLdoPA(); /* Turn on internal PA */
  RF_TurnonTxAnt(); /* Turn on external PA */
}

/**
* @brief Set chip mode
* @param <ChipMode> Chip mode
* - CHIPMODE_MODE0
* - CHIPMODE_MODE1
*/
void RF_SetChipMode(RfChipMode_t ChipMode)
{
  if(ChipMode == CHIPMODE_MODE0)
  {
    /* Mode0 config */
    RF_WritePageRegBits(1, 0x25, 0, 0xF0);
    RF_WritePageRegBits(1, 0x25, 0, 0x08);
    RF_WritePageRegBits(3, 0x12, 1, 0x04);
    RF_WritePageRegBits(3, 0x12, 1, 0x10); 
    RF_WritePageRegBits(0, 0x58, 1, 0x04); /* Enable crc interrupt */ 
  } 
  else 
  { 
    /* Mode1 config */ 
    RF_WritePageRegBits(1, 0x25, 4, 0xF0); 
    RF_WritePageRegBits(1, 0x25, 1, 0x08); 
    RF_WritePageRegBits(3, 0x12, 0, 0x04); 
    RF_WritePageRegBits(3, 0x12, 0, 0x10); 
    RF_WritePageRegBits(0, 0x58, 0, 0x04); /* Disable crc interrupt */ 
  } 
  g_RfCfgParams.ChipMode = ChipMode;
}

/** 
* @brief Get chip mode 
* @param -
*/
RfChipMode_t RF_GetChipMode(void)
{
  return g_RfCfgParams.ChipMode;
}

/**
* @brief Read byte from information area
* @param <Addr> register address
* <Pattern> pattern match value
* <InfoAddr> information area address
* @return byte value read from information area
*/
uint8_t RF_ReadInfoByte(uint8_t Addr, uint16_t Pattern, uint8_t InfoAddr)
{
  uint8_t Value;
  uint8_t Buffer[3];
  uint16_t Timeout = 10000;

  Buffer[0] = Pattern >> 8;
  Buffer[1] = Pattern & 0xFF;
  Buffer[2] = InfoAddr << 1;
  RF_WritePageRegs(2, Addr, Buffer, sizeof(Buffer)); 
  do 
  { 
    if (RF_ReadPageReg(0, 0x6C) & 0x80) break; 
  } while (Timeout--); 
  Value = RF_ReadPageReg(2, Addr);
  return Value;
}

/**
* @brief Calibrate RF related parameters
* @return RF_Err_t Return operation result
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
*/
RF_Err_t RF_Calibrate(void)
{
  int i;
  uint8_t Temp[3] = {0};

  /* Temp[0]: efuse[0x1E] - DCDCIMAX
  Temp[1]: efuse[0x1F] - DCDCREF
  Temp[2]: efuse[0x20] - PABIAS */
  RF_ResetPageRegBits(2, 0x3E, 0x08); // Unlock info
  for (i = 0; i < sizeof(Temp); i++) Temp[i] = RF_ReadInfoByte(0x3B, 0x5AA5, 0x1E + i); 
  if (RF_ReadInfoByte(0x3B, 0x5AA5, 0x1C) == 0x5A) 
  { 
    RF_WritePageReg(2, 0x3D, 0xFD); 
    if (Temp[2] != 0) 
    { 
      /* Write PABIAS */ 
      RF_WritePageReg(0, 0x45, Temp[2]); 
    } 
    RF_WritePageReg(3, 0x1C, (0xC0 | (Temp[0] & 0x1F))); /* Write DCDCIMAX */ 
    RF_WritePageReg(3, 0x1D, Temp[1]); /* Write DDCCREF */ 
  } 
  RF_ASSERT(RF_SetPageRegBits(2, 0x3E, 0x08)); // Lock info
  return RF_OK;
}

/**
* @brief Configure AGC function
* @return RF_Err_t Returns the operation result
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
*/
RF_Err_t RF_ConfigAgc(void)
{
  /* Enable AGC function
  - [Page2][0x06][Bit0] equal to 0 means enable AGC function
  - [Page2][0x06][Bit1] equal to 1 means disable AGC function */
  RF_ASSERT(RF_ResetPageRegBits(2, 0x06, 0x01));
#if REGION_DEFAULT == REGION_CN470_510
  RF_WritePageRegs(2, 0x0A, (uint8_t *)g_LowFreqAgcCfg, 40);
#elif REGION_DEFAULT == REGION_EU_863_870 || REGION_DEFAULT == REGION_US_902_928
  RF_WritePageRegs(2, 0x0A, (uint8_t *)g_HighFreqAgcCfg, 40);
#endif
  RF_ASSERT(RF_WritePageReg(2, 0x34, 0xEF));
  return RF_OK;
}

/**
* @brief Configure default RF register parameters
* @return RF_Err_t Returns operation result
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
*/
RF_Err_t RF_ConfigDefaultParams(void)
{
  int i;
  for(i = 0; i < sizeof(g_RfDefaultConfig) / sizeof(PAN_RegCfg_t); i++)
  {
    RF_WritePageReg(g_RfDefaultConfig[i].Page, g_RfDefaultConfig[i].Addr, g_RfDefaultConfig[i].Value);
  }
  return RF_OK;
}

/**
* @brief Initializes the RF transceiver to STB3 state after power-up
* @return RF_Err_t Returns the operation result
* - RF_OK: Operation successful
* - RF_FAIL: Operation failed
* @note Before calling this function, you must configure the MCU's SPI and related GPIO pins.
*/
RF_Err_t RF_Init(void)
{
#if USE_RF_RST_GPIO == 1
  RF_RESET_PIN_LOW(); /* Pull the chip reset pin low to initiate reset */
  RF_DelayUs(100); /* Ensure that the actual delay is above 100us */
  RF_RESET_PIN_High(); /* Pull up the chip reset pin and release the reset */
  RF_DelayUs(100); /* Ensure that the actual delay is above 100us */
#endif
  I2C_Reset(); /* Reset the I2C bus */
  /* [Pagex][0x04][BIT4] is the reset control. When it is 0, the chip is reset. When it is 1, the reset is released */
  RF_WriteReg(0x04, 0x06); /* Start POR reset chip */
  RF_DelayUs(100); /* Ensure that the actual delay is above 100us */
  RF_SetPage(0); /* Select register page 0 */
  RF_ASSERT(RF_WriteReg(0x02, RF_STATE_DEEPSLEEP)); /* Enter deepsleep state */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
  RF_ASSERT(RF_WriteReg(0x02, RF_STATE_SLEEP)); /* Enter sleep state */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
  RF_ASSERT(RF_WritePageReg(3, 0x06, 0x20)); /* Enable ISO */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
  RF_ASSERT(RF_WriteReg(0x02, RF_STATE_STB1)); /* Enter stb1 state */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
#if USE_ACTIVE_CRYSTAL == 1 /* If an active crystal oscillator is used, the TCXO GPIO pin needs to be configured */
  RF_ASSERT(RF_WritePageReg(3, 0x26, 0xA0)); /* Enable the core power supply and turn on the active crystal oscillator channel */
  RF_DelayUs(100); /* Ensure that the actual delay is above 100us */
  RF_ASSERT(RF_WriteReg(0x04, 0x36)); /* Enable LFT and release POR reset */
  RF_DelayMs(1); /* Ensure that the actual delay is above 1ms */
  RF_InitTcxoGpio(); /* Initialize the TCXO GPIO pin */
#else
  RF_ASSERT(RF_WritePageReg(3, 0x26, 0x20)); /* Enable the core power supply */
  RF_DelayUs(100); /* Ensure that the actual delay is above 100us */
  RF_ASSERT(RF_WriteReg(0x04, 0x36)); /* Enable LFT and release POR reset */
  RF_DelayMs(1); /* Ensure that the actual delay is above 1ms */
#endif
  RF_ASSERT(RF_WriteReg(0x02, RF_STATE_STB2)); /* Enter stb2 state */
  RF_DelayMs(1); /* Ensure that the actual delay is above 1ms */
  RF_ASSERT(RF_WriteReg(0x02, RF_STATE_STB3)); /* Enter stb3 state */
  RF_DelayUs(100); /* Ensure that the actual delay is above 100us */
  RF_ASSERT(RF_ConfigDefaultParams()); /* Configure the default parameters of the RF register */
  RF_ASSERT(RF_Calibrate()); /* Calibrate RF related parameters */
  RF_ASSERT(RF_ConfigAgc()); /* Configure AGC function */
  RF_InitAntGpio(); /* Initialize antenna GPIO pins */
  g_RfOperatetate = RF_STATE_STB3; /* Set the current operating state to STB3 */
  return RF_OK;
}

/**
* @brief Configure user parameters of RF chip
*/
void RF_ConfigUserParams(void)
{
  RF_SetTxPower(22); /* Set power level */
  RF_SetFreq(RF_FREQ_DEFAULT); /* Set frequency */
  RF_SetBW(RF_BW_DEFAULT); /* Set bandwidth */
  RF_SetSF(RF_SF_DEFAULT); /* Set spreading factor */
  RF_SetCR(RF_CR_DEFAULT); /* Set channel coding rate */
  RF_SetCRC(RF_CRC_DEFAULT); /* Set CRC checksum */
  RF_SetLDR(RF_LDR_DEFAULT); /* Set low-rate mode */
  RF_SetPreamLen(RF_PREAMBLE_DEFAULT); /* Set preamble length */
  RF_SetInvertIQ(RF_IQ_INVERT_DEFAULT); /* Set IQ non-invert */
  RF_SetRegulatorMode(USE_LDO); /* Set chip to LDO power mode */
  RF_SetChipMode(CHIPMODE_MODE0); /* Set chip mode to MODE0 */
}

/**
* @brief Software reset RF chip control logic
*/
void RF_ResetLogic(void)
{
  RF_WriteReg(0x00, 0x80);
  RF_WriteReg(0x00, 0x00);
  (void)RF_ReadReg(0x00); /* A dummy read of register 0x00 is required for the reset to take effect. */
}

/**
* @brief Get the RF chip's operating state.
* @return RfOpState_t Current operating state.
* - RF_STATE_SLEEP: Chip is in sleep mode.
* - RF_STATE_STB3: Chip is in standby mode.
* - RF_STATE_TX: Chip is in transmit mode.
* - RF_STATE_RX: Chip is in receive mode.
*/
RfOpState_t RF_GetOperateState(void)
{
  return g_RfOperate;
}

/**
* @brief Set the RF chip's operating state.
* @param <RfState> Operating state.
* - RF_STATE_SLEEP: Chip is in sleep mode.
* - RF_STATE_STB3: Chip in standby mode
* - RF_STATE_TX: Chip in transmit mode
* - RF_STATE_RX: Chip in receive mode
*/
void RF_SetOperateState(RfOpState_t RfState)
{
  g_RfOperatetate = RfState;
}

/**
* @brief Set the operating state of the RF chip
* @param <RfState>
* - RF_STATE_DEEPSLEEP
* - RF_STATE_SLEEP
* - RF_STATE_STB3
* - RF_STATE_TX
* - RF_STATE_RX
*/
void RF_SetRfState(uint8_t RfState)
{
  RF_WriteReg(0x02, RfState);
  g_RfOperatetate = (RfOpState_t)RfState;
}

/**
* @brief Enter deep sleep mode
* @note This function is used to put the RF chip into deep sleep mode, turn off the antenna power supply and TCXO power supply
* @note This function will set the chip's working state to MODE_DEEPSLEEP
* @note After executing this function, if you need to wake up the RF chip, you need to call the RF_Init() function to wake up the chip
*/
void RF_EnterDeepsleepState(void)
{
  RF_ShutdownAnt(); /* Turn off the antenna */
  RF_WriteReg(0x02, RF_STATE_STB3); /* Enter STB3 state */
  RF_DelayUs(150); /* Ensure that the actual delay is above 150us */
  RF_WriteReg(0x02, RF_STATE_STB2); /* Enter STB2 state */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
  RF_WriteReg(0x02, RF_STATE_STB1); /* Enter STB1 state */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
#if USE_ACTIVE_CRYSTAL == 1 /* If using an active crystal oscillator, you need to turn off the TCXO power supply */
  RF_TurnoffTcxo(); /* Turn off the TCXO power supply */
#endif
  RF_WriteReg(0x04, 0x06); /* Turn off LFT */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
  RF_WriteReg(0x02, RF_STATE_SLEEP); /* Enter SLEEP state */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
  RF_WritePageReg(3, 0x06, 0x00); /* Turn off ISO */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
  RF_WritePageReg(3, 0x26, 0x00); /* Turn off the internal power supply */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
  RF_WriteReg(0x02, RF_STATE_DEEPSLEEP); /* Enter the DEEPSLEEP state */
  g_RfOperatetate = RF_STATE_DEEPSLEEP;
}

/**
* @brief Enter sleep mode
* @note This function is used to put the RF chip into sleep mode, turning off the antenna power supply and TCXO power supply
* @note This function sets the chip's operating state to MODE_SLEEP
* @note After executing this function, if you need to wake up the RF chip, you need to call the RF_ExitSleepState() function
*/
void RF_EnterSleepState(void)
{
  RF_ShutdownAnt(); /* Turn off the antenna */
  RF_WriteReg(0x02, RF_STATE_STB3); /* Enter STB3 state */
  RF_DelayUs(150); /* Ensure that the actual delay is above 150us */
  RF_WriteReg(0x02, RF_STATE_STB2); /* Enter STB2 state */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
  RF_WriteReg(0x02, RF_STATE_STB1); /* Enter STB1 state */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
#if USE_ACTIVE_CRYSTAL == 1 /* If using an active crystal oscillator, turn off the TCXO power supply */
  RF_TurnoffTcxo(); /* Turn off TCXO power */
#endif
  RF_WriteReg(0x04, 0x16); /* Turn off LFT */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
  RF_WriteReg(0x02, RF_STATE_SLEEP); /* Enter SLEEP state */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
  RF_ResetPageRegBits(3, 0x06, 0x20); /* Turn off ISO */
  RF_DelayUs(10); /* Ensure that the actual delay is above 10us */
  RF_WritePageReg(3, 0x26, 0x00); /* Turn off internal power */
  g_RfOperatetate = RF_STATE_SLEEP;
}

/**
* @brief Exit sleep mode
* @note This function is used to exit the RF chip from sleep mode and turn on the antenna power supply and TCXO power supply
* @note This function will set the chip's operating state to MODE_STDBY
*/
void RF_ExitSleepState(void)
{
  RF_SetPageRegBits(3, 0x06, 0x20); /* Enable ISO */
  RF_DelayUs(10); /* Ensure the actual delay is above 10us */
  RF_WriteReg(0x02, RF_STATE_STB1); /* Enter STB1 state */
  RF_DelayUs(10); /* Ensure the actual delay is above 10us */
#if USE_ACTIVE_CRYSTAL == 1
  RF_WritePageReg(3, 0x26, 0xA0); /* Enable the core power supply and turn on the active crystal oscillator channel */
  RF_DelayUs(100); /* Ensure that the actual delay is above 100us */
  RF_WriteReg(0x04, 0x36); /* Enable LFT */
  RF_DelayUs(100); /* Ensure that the actual delay is above 100us */
  RF_TurnonTcxo(); /* Turn on TCXO */
#else
  RF_WritePageReg(3, 0x26, 0x20); /* Enable core power */
  RF_DelayUs(100); /* Ensure that the actual delay is above 100us */
  RF_WriteReg(0x04, 0x36); /* Enable LFT */
  RF_DelayUs(100); /* Ensure that the actual delay is above 100us */
#endif
  RF_WriteReg(0x02, RF_STATE_STB2); /* Enter STB2 state */
  RF_DelayMs(1); /* Ensure the actual delay is above 1ms */
  RF_WriteReg(0x02, RF_STATE_STB3); /* Enter STB3 state */
  RF_DelayUs(100); /* Ensure the actual delay is above 100us*/
  g_RfOperate = RF_STATE_STB3;
}

/**
* @brief Enter standby mode
* @note This function sets the chip's operating state to MODE_STDBY
*/
void RF_EnterStandbyState(void)
{
  RF_SetRfState(RF_STATE_STB3);
  RF_SetOperateState(RF_STATE_STB3);
}

/**
* @brief Check if the RF chip is in sleep mode
* @note This function checks if the RF chip is in sleep mode.
* @note If it is in sleep mode, it exits sleep mode and enters standby mode.
* @note This function sets the chip's operating state to MODE_STDBY
*/
void RF_CheckDeviceReady(void)
{
  if (RF_GetOperateState() == RF_STATE_SLEEP) RF_ExitSleepState();
}

/**
* @brief Set the chip's power supply mode.
* @param <RegulatorMode> Power supply mode.
* - USE_LDO: Use LDO power supply.
* - USE_DCDC: Use DCDC power supply.
* @note: In transmit mode, the RF must use the LDO power supply mode.
* In other modes, any power supply mode can be selected.
*/
void RF_SetRegulatorMode(RfRegulatorMode_t RegulatorMode)
{
  RF_WritePageReg(3, 0x24, (RegulatorMode == USE_DCDC) ? 0x08 : 0x00);
  g_RfCfgParams.RegulatorMode = RegulatorMode;
}

/**
* @brief Set the RF chip frequency
* @param <Frequency> Communication frequency (Hz)
* @note Supported frequency range:
* Low frequency band:
* - 138.33MHz to 282.5MHz
* - 405.00MHz to 565.00MHz
* High frequency band:
* - 810.00MHz to 1080.00MHz
*/
RF_Err_t RF_SetFreq(uint32_t Frequency)
{
  int i;
  uint32_t Fa, Fb; 
  uint32_t Temp = 0; 
  uint32_t IntegerPart; 
  uint8_t FreqReg[4], Fab[3]; 
  uint32_t FreqTableNum = (sizeof(g_RfFreqTable) / sizeof(RadioFreqTable_t)); 

  if (Frequency < g_RfFreqTable[0].StartFreq || Frequency > g_RfFreqTable[FreqTableNum - 1].StopFreq) return RF_FAIL; 
  /* Traverse the frequency table and find the matching frequency segment */ 
  for (i = 0; i < FreqTableNum; i++) 
  { 
    if (Frequency > g_RfFreqTable[i].StartFreq && Frequency <= g_RfFreqTable[i].StopFreq) 
    { 
      uint8_t LoMux = (g_RfFreqTable[i].LoParam & 0x70) >> 4; 
      Temp = Frequency * g_VcoDivTable[LoMux]; 
      RF_WritePageRegs(0, 0x40, (uint8_t *)&g_RfFreqTable[i].VcoParam, 2); 
      RF_WriteReg(0x3D, g_RfFreqTable[i].LoParam); 
      break; 
    } 
  } 
  /* No frequency range matched */ 
  if (i >= FreqTableNum) return RF_FAIL; 
  IntegerPart = Temp / 32000000; 
  Fa = IntegerPart - 20; 
  Fb = (Temp % 32000000) / 40000; 
  FreqReg[0] = (uint8_t)Frequency; FreqReg[1] = (uint8_t)(Frequency >> 8);
  FreqReg[2] = (uint8_t)(Frequency >> 16);
  FreqReg[3] = (uint8_t)(Frequency >> 24);
  RF_WritePageRegs(3, 0x09, FreqReg, 4);
  Fab[0] = (uint8_t)(Fa);
  Fab[1] = (uint8_t)(Fb);
  Fab[2] = (uint8_t)((Fb >> 8) & 0x0F);
  RF_WritePageRegs(3, 0x15, Fab, 3);
  g_RfCfgParams.Frequency = Frequency;
  return RF_OK;
}

/**
* @brief Set IQ inversion
* @param <NewState> Enable or disable IQ inversion
* - true: Enable IQ inversion 
* - false: disable IQ inversion 
*/
void RF_SetInvertIQ(bool NewState)
{ 
  if(NewState) 
  { 
    /* 
    * BIT6 = 0: invert rx IQ 
    * BIT5 = 1: invert tx IQ 
    */ 
    RF_WritePageRegBits(1, 0x0E, 0x01, 0x40 | 0x20); 
    g_RfCfgParams.InvertIQ = RF_IQ_INVERTED; 
  } 
  else 
  { 
    /* 
    * BIT6 = 1: non-invert rx IQ 
    * BIT5 = 0: non-invert tx IQ 
    */ 
    RF_WritePageRegBits(1, 0x0E, 0x02, 0x40 | 0x20); g_RfCfgParams.InvertIQ = RF_IQ_NORMAL;
  }
}

/**
* @brief Set the preamble length
* @param <PreamLen> Preamble length value
* Range is 4 - 65535
*/
void RF_SetPreamLen(uint16_t PreamLen)
{
  uint8_t Temp[2] = {(uint8_t)(PreamLen), (uint8_t)((PreamLen >> 8))};
  RF_WritePageRegs(3, 0x13, Temp, 2);
  g_RfCfgParams.PreambleLen = PreamLen;
}

/**
* @brief Set the synchronization word
* @param <syncWord> Synchronization word value
* @note The synchronization word size supported by PAN3029/3060 is 1 byte
* @note The sync word is used for synchronization detection when receiving data packets. Typically, the same sync word should be set when sending and receiving data packets.
* For example, if the sync word is set to 0x12 when sending a data packet, the sync word should also be set to 0x12 when receiving a data packet.
*/
void RF_SetSyncWord(uint8_t SyncWord)
{
  RF_WritePageReg(3, 0x0F, SyncWord);
  g_RfCfgParams.SyncWord = SyncWord;
}

/**
* @brief Set the transmit power.
* @param <TxPower> Transmit power level, range: 1-22
* @note The power values corresponding to the power levels are shown in the following table:
*|------|------------------|------------------|------------------|------------------|
*| Level | 410MHz Power (dBm) | 430MHz Power (dBm) | 450MHz Power (dBm) | 460MHz Power (dBm) |
*|------|------------------|------------------|------------------|------------------|
*| 1 | -18.7 | -18.2 | -18.8 | -19.7 |
*| 2 | -8.3 | -7.9 | -8.6 | -9.5 |
*| 3 | 1.6 | 1.7 | 0.9 | -0.1 |
*| 4 | 3.9 | 4.5 | 4.1 | 3.3 |
*| 5 | 4.9 | 5.3 | 4.8 | 3.9 |
*| 6 | 5.4 | 5.5 | 4.8 | 3.8 |
*| 7 | 7.1 | 7.6 | 7.3 | 6.8 |
*| 8 | 7.7 | 8.2 | 8.0 | 7.5 |
*| 9 | 8.6 | 9.2 | 8.8 | 8.1 |
*| 10 | 9.0 | 9.3 | 9.1 | 8.9 |
*| 11 | 10.3 | 10.7 | 10.6 | 10.3 |
*| 12 | 10.8 | 11.0 | 10.8 | 10.6 |
*| 13 | 11.6 | 11.8 | 11.6 | 11.5 |
*| 14 | 12.8 | 13.2 | 13.1 | 12.9 |
*| 15 | 13.8 | 14.3 | 14.2 | 14.0 |
*| 16 | 14.9 | 15.4 | 15.4 | 15.0 |
*| 17 | 15.4 | 16.0 | 15.8 | 15.3 |
*| 18 | 16.1 | 16.6 | 16.5 | 16.3 |
*| 19 | 16.5 | 17.0 | 16.9 | 16.7 | 
*| 20 | 17.4 | 17.9 | 17.8 | 17.6 | 
*| 21 | 18.2 | 18.3 | 18.0 | 17.7 | 
*| 22 | 19.4 | 19.4 | 19.1 | 18.7 | 
*|------|------------------|------------------|------------------|------------------| 
*/
void RF_SetTxPower(uint8_t TxPower)
{ 
  int Index; 
  uint8_t Temp1, Temp2; 
  static bool PaBiasReadFlag = false; // for read efuse only once 
  static uint8_t PaBiasVal = 0; 

  TxPower = (TxPower > RF_MAX_RAMP ? RF_MAX_RAMP : TxPower); 
  TxPower = (TxPower < RF_MIN_RAMP ? RF_MIN_RAMP : TxPower); 
  Index = TxPower - 1; 
  /* Modulate wave ramp mode */ 
  RF_WritePageReg(3, 0x22, g_RfPowerRampCfg[Index].Ldo & 0x01); 
  RF_WritePageReg(0, 0x1E, g_RfPowerRampCfg[Index].Ramp); 
  RF_WritePageReg(0, 0x4B, g_RfPowerRampCfg[Index].Ldo >> 4); 
  if (g_RfPowerRampCfg[Index].PAbias != 0x70) RF_SetPageRegBits(0, 0x46, 0x04); // page0, reg0x46, bit2=1 
  else RF_ResetPageRegBits(0, 0x46, 0x04); // page0, reg0x46, bit2=0 
  if(!PaBiasReadFlag) 
  { 
    PaBiasReadFlag = true; 
    RF_ResetPageRegBits(2, 0x3E, 0x08); /* RF unlock info */ 
    PaBiasVal = RF_ReadInfoByte(0x3B, 0x5AA5, 0x20); 
    RF_SetPageRegBits(2, 0x3E, 0x08); /* RF lock info */ 
    if (PaBiasVal == 0) PaBiasVal = 8; 
  } 
  Temp1 = PaBiasVal - (g_RfPowerRampCfg[Index].PAbias & 0x0F); 
  Temp2 = (g_RfPowerRampCfg[Index].PAbias & 0xF0) | Temp1;
  RF_WritePageReg(0, 0x45, Temp2);
  g_RfCfgParams.TxPower = TxPower; /* save current TxPower value */
}

/**
* @brief Set the modulation bandwidth
* @param <BandWidth> Modulation bandwidth value
* - RF_BW_062K / RF_BW_125K / RF_BW_250K / RF_BW_500K
* @note A larger modulation bandwidth increases the data rate but shortens the transmission distance.
* @note The modulation bandwidth range of the PAN3029 chip is RF_BW_062K - RF_BW_500K
* @note The modulation bandwidth range of the PAN3060 chip is RF_BW_125K - RF_BW_500K
*/
void RF_SetBW(uint8_t BandWidth)
{
  /* Page 3, Reg 0x0D, Bit[7:4] = BandWidth */
  RF_WritePageRegBits(3, 0x0D, BandWidth, 0xF0);
  if (BandWidth != RF_BW_500K) RF_SetPageRegBits(2, 0x3F, 0x02);
  else RF_ResetPageRegBits(2, 0x3F, 0x02);
  g_RfCfgParams.Bandwidth = (RfBandwidths_t)BandWidth; // save current BW value
}

/**
* @brief Set the spreading factor
* @param <SpreadFactor> Spreading factor value
* - RF_SF5 / RF_SF6 / RF_SF7 / RF_SF8 / RF_SF9 / RF_SF10 / RF_SF11 / RF_SF12
* @note A larger spreading factor increases the transmission distance, but also reduces the data rate.
* @note The spreading factor range for the PAN3029 chip is RF_SF5 - RF_SF12
* @note The spreading factor range for the PAN3060 chip is RF_SF5 - RF_SF9
*/
void RF_SetSF(uint8_t SpreadFactor)
{
  /* Page 3, Reg 0x0E, Bit[7:4] = SpreadFactor */
  RF_WritePageRegBits(3, 0x0E, SpreadFactor, 0xF0);
  g_RfCfgParams.SpreadingFactor = (RfSpreadFactor_t)SpreadFactor; // Save current SF value
}

/**
* @brief Set the channel coding rate
* @param <CodingRate> Channel coding rate value
* - RF_CR_4_5 / RF_CR_4_6 / RF_CR_4_7 / RF_CR_4_8
*/
void RF_SetCR(uint8_t CodingRate)
{
  /* Page 3, Reg 0x0D, Bit[3:1] = CodingRate */
  RF_WritePageRegBits(3, 0x0D, CodingRate, 0x0E);
  g_RfCfgParams.CodingRate = (RfCodingRates_t)CodingRate; // Save current CR value
}

/**
* @brief Set CRC checksum
* @param <CrcMode> Enable or disable CRC checksum
* - RF_CRC_ON: Enable CRC checksum
* - RF_CRC_OFF: Disable CRC checksum
*/
void RF_SetCRC(uint8_t CrcMode)
{
  /* Page 3, Reg 0x0D, Bit[0] = CRC */
  RF_WritePageRegBits(3, 0x0E, CrcMode, 0x08);
  g_RfCfgParams.CrcMode = (RfCrcModes_t)CrcMode; // save current CRC value
}

/**
* @brief Set low-rate mode
* @param <LdrMode> Low-rate mode value
* - RF_LDR_ON: Enable low-rate mode
* - RF_LDR_OFF: Disable low-rate mode
*/
void RF_SetLDR(uint8_t LdrMode)
{
  /* Page 3, Reg 0x12, Bit[3] = LDR */
  RF_WritePageRegBits(3, 0x12, LdrMode, 0x08); 
  g_RfCfgParams.LowDatarateOptimize = LdrMode; // save current LDR value
}

/** 
* @brief Set modem mode 
* @param <modem_mode> 
* - MODEM_MODE_NORMAL 
* - MODEM_MODE_MULTI_SECTOR 
* @note This function should be called after RF_SetSF(uint8_t SpreadFactor) 
*/
void RF_SetModemMode(uint8_t ModemMode)
{ 
  if (ModemMode == MODEM_MODE_NORMAL) RF_WritePageReg(1, 0x0B, 0x08); 
  else if (ModemMode == MODEM_MODE_MULTI_SECTOR) 
  { 
    RF_WritePageReg(1, 0x0B, 0x18); 
    if( g_RfCfgParams.SpreadingFactor <= RF_SF6 )
    {
      RF_WritePageReg(1, 0x2F, 0x74);
      RF_WritePageReg(1, 0x30, 0x01);
    }
    else
    {
      RF_WritePageReg(1, 0x2F, 0x54);
      RF_WritePageReg(1, 0x30, 0x40);
    }
  }
}

/**
* @brief Set the transmit mode
* @param Buffer Data buffer to be sent
* @param Size Number of data bytes to be sent
* @note Ensure that the RF is in standby (STB3) mode before calling this function
* @note This function is in single-shot transmit mode and will automatically enter standby (STB3) mode after the transmit is complete.
* @note The TX_DONE interrupt will be triggered after the transmit is complete.
*/
void RF_SetTx(uint8_t *Buffer, uint8_t Size)
{
  RF_TxSinglePkt(Buffer, Size);
  g_RfOperatetate = RF_STATE_TX;
}

/**
* @brief Set the receive mode
* @param TimeoutMs Receive timeout, in milliseconds
* - 0: Continuous receive mode
* - >0: Single receive mode, after timeout, a timeout interrupt is generated and the device automatically enters standby (STB3) mode.
* @note Ensure that the RF is in standby (STB3) mode before calling this function.
*/
void RF_SetRx(uint32_t TimeoutMs)
{
  if (TimeoutMs == 0) RF_EnterContinousRxState();
  else RF_EnterSingleRxWithTimeout(TimeoutMs);
  g_RfOperatetate = RF_STATE_RX;
}

/**
* @brief Start CAD detection 
* @param <Threshold> 
- RF_CAD_THRESHOLD_0A 
- RF_CAD_THRESHOLD_10 
- RF_CAD_THRESHOLD_15 
- RF_CAD_THRESHOLD_20 
<Chirps> 
- RF_CAD_01_SYMBOL 
- RF_CAD_02_SYMBOL 
- RF_CAD_03_SYMBOL 
- RF_CAD_04_SYMBOL 
*/
void RF_StartCad(uint8_t Threshold, uint8_t Chirps)
{ 
  /* Configure GPIO11 as output for CAD indication */ 
  RF_ConfigGpio(MODULE_GPIO_CAD_IRQ, GPIO_MODE_OUTPUT); 
  /* [Page0][Reg0x5E][Bit6] = 0, enable GPIO11 CAD indication */ RF_ResetPageRegBits(0, 0x5E, 0x40); 
  RF_WritePageReg(1, 0x0F, Threshold); /* [Page1][Reg0x0F] = Threshold */ 
  RF_WritePageRegBits(1, 0x25, Chirps - 1, 0x03); /* [Page1][Reg0x25][Bit[1:0]] = Chirps - 1 */ 
  RF_WritePageReg(1, 0x35, 0xFE); /* [Page1][Reg0x35] payload cad config */ 
  RF_EnterContinousRxState(); /* Enter continous RX state */
}

/** 
* @brief Set CAD detection threshold 
* @param <Threshold> CAD detection threshold 
* - RF_CAD_THRESHOLD_0A 
* - RF_CAD_THRESHOLD_10
* - RF_CAD_THRESHOLD_15
* - RF_CAD_THRESHOLD_20
*/
void RF_SetCadThreshold(uint8_t Threshold)
{
  RF_WritePageReg(1, 0x0F, Threshold);
}

/**
* @brief Set the number of CAD detection symbols
* @param <Chirps> Number of CAD detection symbols
* - RF_CAD_01_SYMBOL
* - RF_CAD_02_SYMBOL
* - RF_CAD_03_SYMBOL
* - RF_CAD_04_SYMBOL
*/
void RF_SetCadChirps(uint8_t Chirps)
{
  RF_WritePageRegBits(1, 0x25, Chirps - 1, 0x03); /* [Page1][Reg0x25][Bit[1:0]] = Chirps */
}

/** 
* @brief Stop CAD detection
*/
void RF_StopCad(void)
{ 
  RF_SetPageRegBits(0, 0x5E, 0x40); /* [Page0][Reg0x5E][Bit6] = 1, disable GPIO11 CAD indication */ 
  RF_WritePageReg(1, 0x0F, 0x0A); /* Reset CAD threshold */ 
  RF_WritePageReg(1, 0x35, 0xF4); /* Reset payload cad config */ 
  RF_SetRfState(RF_STATE_STB3); /* Enter standby state */ 
  RF_ResetLogic(); /* Soft reset the RF chip, clear cad state */ 
  g_RfOperatetate = RF_STATE_STB3;
}

/**
* @brief Set the transmit mode
* @param <TxMode>
* - RF_TX_SINGLE: Single transmit mode
* - RF_TX_CONTINOUS: Continuous transmit mode
* @note This function is only used to set the transmit mode and does not change the chip's working state
*/
void RF_SetTxMode(uint8_t TxMode)
{
  RF_WritePageRegBits(3, 0x06, TxMode, 0x04); /* [Page3][Reg0x06][Bit[2]] = TxMode */
}

/**
* @brief Send a single data packet
* @param <Buffer> Data buffer to be sent
* @param <Size> Number of data bytes to be sent
* @note TX_DONE interrupt will be triggered after the transmission is completed
* @note In the transmit completion interrupt, the RF_TurnoffPA() function needs to be called to turn off the chip's internal and external PAs
*/
void RF_TxSinglePkt(uint8_t *Buffer, uint8_t Size)
{
  RF_WriteReg(0x02, RF_STATE_STB3); /* Enter standby mode */
  RF_SetTxMode(RF_TX_MODE_SINGLE); /* Set single transmit mode */
  RF_WritePageReg(1, 0x0C, Size); /* Set transmit data length */
  RF_TurnonPA(); /* Turn on the PA before transmitting data */
  RF_SetRfState(RF_STATE_TX); /* Set the chip to transmit mode */
  RF_WriteRegs(0x01, Buffer, Size); /* Write data to the FIFO. After writing the data, the chip starts transmitting data. 0x01 is the FIFO register address */
}

/**
* @brief Set the receive mode
* @param <RxMode>
* - RF_RX_SINGLE: Single receive mode, automatically enters standby mode after receiving a packet of data.
* - RF_RX_SINGLE_TIMEOUT: Single receive mode with timeout, automatically enters standby mode after the timeout.
* - RF_RX_CONTINOUS: Continuous receive mode, continues receiving after receiving a packet of data.
* @note This function only sets the receive mode and does not change the chip's operating state.
*/
void RF_SetRxMode(uint8_t RxMode)
{
  RF_WritePageRegBits(3, 0x06, RxMode, 0x03); /* [Page3][Reg0x06][Bit[1:0]] = RxMode */
}

/**
* @brief Sets the chip to continuous receive mode.
* @note After calling this function, the chip will enter continuous receive mode.
* @note This function sets the chip's operating state to MODE_RX.
*/
void RF_EnterContinuousRxState(void)
{
  RF_SetRfState(RF_STATE_STB3); /* Enter standby mode */
  RF_TurnonRxAnt(); /* Turn on the receiving antenna */
  RF_TurnoffLdoPA(); /* Turn off the internal PA */
  RF_SetRxMode(RF_RX_MODE_CONTINOUS); /* Set the receive mode to continuous receive */
  RF_SetRfState(RF_STATE_RX); /* Enter receive mode */
}

/**
* @brief Set the receive timeout
* @param <TimeoutMs> Timeout time, in milliseconds
* Timeout range: 0-65535ms
* @note This function only sets the receive timeout and does not change the chip's operating state.
*/
void RF_SetRxTimeout(uint16_t TimeoutMs)
{
  uint8_t Temp[2] = {(uint8_t)TimeoutMs, (uint8_t)(TimeoutMs >> 8)};
  RF_WritePageRegs(3, 0x07, Temp, 2);
}

/**
* @brief Set the chip to single receive state with timeout
* @param <TimeoutMs> Timeout period, in ms
* Timeout period range: 1~65535ms
* @note After calling this function, the chip will enter single receive state with timeout
* @note This function will set the chip's operating state to MODE_RX
*/
void RF_EnterSingleRxWithTimeout(uint16_t TimeoutMs)
{
  RF_SetRfState(RF_STATE_STB3); /* Enter standby mode */
  RF_TurnonRxAnt(); /* Turn on the receive antenna */
  RF_TurnoffLdoPA(); /* Turn off the internal PA */
  RF_SetRxTimeout(TimeoutMs); /* Set the receive timeout */
  RF_SetRxMode(RF_RX_MODE_SINGLE_TIMEOUT); /* Set the receive mode to single receive mode */
  RF_SetRfState(RF_STATE_RX); /* Enter receive mode */
}

/**
* @brief Get the received data length
* @return Received data length
* @note This function must be called before the receive interrupt is cleared, as clearing the receive interrupt will clear the receive length register.
*/
uint8_t RF_GetRxPayloadLen(void)
{
  return RF_ReadPageReg(1, 0x7D);
}

/**
* @brief Function used to obtain the length and content of received data
* @param *Buffer Pointer address of the data area to be received
* @return Received data length
* @note This function must be called before the receive interrupt is cleared, as clearing the receive interrupt will clear the receive length register
*/
uint8_t RF_GetRecvPayload(uint8_t *Buffer)
{
  uint8_t Size;
  Size = RF_GetRxPayloadLen(); /* Get the received data length */
  RF_ReadRegs(0x01, Buffer, Size); /* Read the received data from the FIFO, where 0x01 is the FIFO register address */
  return Size;
}

/**
* @brief Get the RSSI value of the received data packet
* @return RSSI value
* @note RSSI value range: -125~-10, unit: dBm, RSSI value is less than or equal to the sensitivity value
* @note This function must be called before the receive interrupt is cleared, because clearing the receive interrupt will clear the signal strength register
*/
int8_t RF_GetPktRssi(void)
{
  return RF_ReadPageReg(1, 0x7F);
}

/**
* @brief Get real-time RSSI value
* @return RSSI value
* @note RSSI value range: -125~-10, unit: dBm
* @note Before calling this function, ensure that RF is in the receiving state
*/
int8_t RF_GetRealTimeRssi(void)
{
  /* Clear Bit[2] of register 0x06 to 0 and then set it to 1 to update the value of [Page1][Reg0x7E] */
  RF_ResetPageRegBits(2, 0x06, 0x04);
  RF_SetPageRegBits(2, 0x06, 0x04);
  return (int8_t)RF_ReadPageReg(1, 0x7E);
}

/**
* @brief Get the SNR value of the received data packet
* @return SNR value
* @note This function must be called before the receive interrupt is cleared, because clearing the receive interrupt will clear the SNR register
* @note SNR value range: -20~10, unit dB
*/
int32_t RF_GetPktSnr(void)
{
  int32_t PktSnr = 0, SnrVal;
  uint8_t i, Temp[6];
  uint32_t NoiseStrength;
  uint32_t SingalStrength;

  RF_ReadPageRegs(2, 0x71, &Temp[0], 3); // Noise strength 
  RF_ReadPageRegs(1, 0x74, &Temp[3], 3); // Singal strength 
  SingalStrength = (((uint32_t)Temp[5] << 16) | ((uint32_t)Temp[4] << 8) | Temp[3]); 
  NoiseStrength = (((uint32_t)Temp[2] << 16) | ((uint32_t)Temp[1] << 8) | Temp[0]); 
  if (NoiseStrength == 0) NoiseStrength = 1;  
  if(g_RfCfgParams.SpreadingFactor <= 9) SnrVal = (SingalStrength << (9 - g_RfCfgParams.SpreadingFactor)) / NoiseStrength; 
  else SnrVal = (SingalStrength >> (g_RfCfgParams.SpreadingFactor - 9)) / NoiseStrength;
  for (i = 0; i < 31; i++)
  {
    if (SnrVal <= g_SnrLog10Talbe[i])
    {
      PktSnr = (int32_t)i - (int32_t)20;
      break;
    }
  }
  return PktSnr;
}

/**
* @brief Get interrupt flags
* @return IRQ flags
* - 0x00: No interrupt
* - 0x01: RF_IRQ_TX_DONE
* - 0x02: RF_IRQ_RX_TIMEOUT
* - 0x04: RF_IRQ_CRC_ERR
* - 0x08: RF_IRQ_RX_DONE
* - 0x40: RF_IRQ_MAPM_DONE
*/
uint8_t RF_GetIRQFlag(void)
{
  return (RF_ReadPageReg(0, 0x6C) & 0x7F);
}

/**
* @brief Clear the interrupt flag
* @param <IRQFlag> Interrupt flag
*/
void RF_ClrIRQFlag(uint8_t IRQFlag)
{
  RF_WritePageReg(0, 0x6C, IRQFlag);
}

/**
* * @brief Get the current frequency setting
*/
uint32_t RF_GetFreq(void)
{
  return g_RfCfgParams.Frequency;
}

/**
* * @brief Get the current IQ inversion value
*/
RfIQModes_t RF_GetInvertIQ(void)
{
  return g_RfCfgParams.InvertIQ;
}

/**
* @brief Get the current preamble length setting
*/
uint16_t RF_GetPreamLen(void)
{
  return g_RfCfgParams.PreambleLen;
}

/**
* @brief Get the current transmit power setting
*/
uint8_t RF_GetTxPower(void)
{
  return g_RfCfgParams.TxPower;
}

/**
* @brief Get the current modulation bandwidth setting
*/
uint8_t RF_GetBandWidth(void)
{
  return g_RfCfgParams.Bandwidth;
}

/**
* @brief Get the current spreading factor setting
*/
uint8_t RF_GetSF(void)
{
  return g_RfCfgParams.SpreadingFactor;
}

/**
* @brief Get the current CRC check setting
*/
uint8_t RF_GetCRC(void)
{
  return g_RfCfgParams.CrcMode;
}

/**
* @brief Get the current coding rate setting
*/
uint8_t RF_GetCR(void)
{
  return g_RfCfgParams.CodingRate;
}

/**
* @brief Get the current sync word setting
*/
uint8_t RF_GetSyncWord(void)
{
  return g_RfCfgParams.SyncWord;
}

/**
* @brief Get the current transmit mode setting
*/
uint8_t RF_GetLDR(void)
{
  return g_RfCfgParams.LowDatarateOptimize;
}

/**
* @brief Get the time of a single symbol
* @param <bw> Bandwidth
* - RF_BW_062K / RF_BW_125K / RF_BW_250K / RF_BW_500K
* @param <sf> Spreading factor
* - RF_SF5 / RF_SF6 / RF_SF7 / RF_SF8 / RF_SF9 / RF_SF10 / RF_SF11 / RF_SF12
* @return Single symbol time, in us
* @note This function is used to calculate the time of a single symbol
*/
uint32_t RF_GetOneSymbolTime(uint8_t bw, uint8_t sf)
{
  const uint32_t BwTable[4] = {62500, 125000, 250000, 500000};

  if(bw < RF_BW_062K || bw > RF_BW_500K) return 0;
  return (1000000 * (1 << sf) / BwTable[bw - RF_BW_062K]);
}

/**
* @brief Calculates the time to send a packet
* @param <Size> The size of the packet to be sent, in bytes
* @return The time to send the packet, in milliseconds
*/
uint32_t RF_GetTxTimeMs(uint8_t Size)
{
  uint8_t sf, cr, bw, ldr;
  uint16_t PreambleLen; /* Preamble length */
  float SymbolTime; /* Symbol time in milliseconds */
  float PreambleTime; /* Preamble time:ms */ 
  float PayloadTime; /* Payload time:ms */ 
  float TotalTime; /* Total time:ms */ 
  const float BwTable[4] = {62.5, 125, 250, 500}; 

  sf = RF_GetSF(); 
  cr = RF_GetCR(); 
  bw = RF_GetBandWidth(); 
  ldr = RF_GetLDR(); 
  PreambleLen = RF_GetPreamLen(); 
  SymbolTime = (float)(1 << sf) / BwTable[bw - RF_BW_062K]; /* Symbol time: ms */ 
  if (sf < 7) 
  { 
    PreambleTime = (PreambleLen + 6.25f) * SymbolTime; 
    PayloadTime = ceil((float)(Size * 8 - sf * 4 + 36) / ((sf - ldr * 2) * 4)); 
  } 
  else 
  { 
    PreambleTime = (PreambleLen + 4.25f) * SymbolTime; 
    PayloadTime = ceil((float)(Size * 8 - sf * 4 + 44) / ((sf - ldr * 2) * 4)); 
  }
  TotalTime = PreambleTime + (PayloadTime * (cr + 4) + 8) * SymbolTime;
  if(TotalTime < 1) TotalTime = 1; /* If less than 1ms, treat as 1ms */
  return (uint32_t)TotalTime;
}

/**
* @brief Enable MPM mode
*/
void RF_EnableMapm(void)
{
  RF_SetPageRegBits(1, 0x38, 0x01); /* Enable MPM mode */
  RF_WritePageRegBits(0, 0x58, 0, 0x40); /* Enable MPM interrupt */
}

/**
* @brief Disable MPM mode
*/
void RF_DisableMapm(void)
{
  RF_ResetPageRegBits(1, 0x38, 0x01); /* Disable MAMP mode */
  RF_WritePageRegBits(0, 0x58, 1, 0x40); /* Disable MAMP interrupt */
}

/**
* @brief Configure MAMP parameters
* @param <pMapmCfg>
* fn: Number of fields in the MAMP, totaling fn*(2^fnm) fields (usually fnm=0)
* fnm: Number of times the same field is sent (default fnm=0)
* - When fnm=0, the same field is sent once, totaling fn*1 fields
* - When fnm=1, the same field is sent twice, totaling fn*2 fields
* - When fnm=2, the same field is sent four times, totaling fn*4 fields
* - When fnm=3, the same field is sent eight times, totaling fn*8 fields
* gfs: Payload function selection for the last group in each field
* - When gfs=0, the last group's payload is an address.
* - When gfs=1, the last group's payload is the number of remaining fields (including the current field).
* gn: Number of groups in a field.
* pg1: Number of preambles in the first group in a field, range 8-255.
* - When pg1=8, the first group has 8 preambles.
* - When pg1=200, the first group has 200 preambles.
* pgn: Number of preambles in all groups except the first group in a field, range 0-255.
* - When pgn=8, the first group has 8 preambles.
* - When pgn=200, the first group has 200 preambles.
* pn: Number of preambles in the packet, range 1-65535.
* @note: The address or count value in a group occupies two chirps.
*/
void RF_ConfigMapm(RF_MapmCfg_t *pMapmCfg)
{ 
  uint8_t reg_fn, fn_h, fn_l; 

  fn_h = pMapmCfg->fn / 15 + 1; 
  fn_l = pMapmCfg->fn % 15 + 1; 
  reg_fn = (fn_h << 4) + fn_l; 
  RF_WritePageReg(1, 0x3D, reg_fn); /* set the number of fields */ 
  RF_WritePageRegBits(1, 0x37, pMapmCfg->fnm, 0x80 | 0x40); /* set the unit code word of the field counter represents several fields */ 
  RF_WritePageRegBits(1, 0x38, pMapmCfg->gfs, 0x02); /* set the last group function selection */ 
  RF_WritePageRegBits(1, 0x38, pMapmCfg->gn - 1, 0x08 | 0x04); /* set the number of groups in Field */ 
  RF_WritePageReg(1, 0x3B, pMapmCfg->pg1); /* set the number of Preambles in first groups */ 
  /* set the number of preambles for groups other than the first group */ 
  RF_WritePageReg(1, 0x3C, pMapmCfg->pgn); 
  /* set the number of preamble between the last group and the sync word */ 
  RF_WritePageRegBits(1, 0x39, (uint8_t)(pMapmCfg->pn >> 8), 0x0F); 
  RF_WritePageReg(1, 0x3A, (uint8_t)(pMapmCfg->pn));
}

/**
* @brief Set the group address in mapm mode
* @param <MapmAddr> mapm group address
* <AddrWidth> Address width, range 1~4
* @note MapmAddr[0] on the receiving end must be consistent with MapmAddr[0] on the sending end,
* Otherwise, the receiving end will not trigger a mapm interrupt.
* @note Mapm address register description:
* [Page1][Reg0x3E] is MapmAddr[0]
* [Page1][Reg0x3F] is MapmAddr[1]
* [Page1][Reg0x40] is MapmAddr[2]
* [Page1][Reg0x41] is MapmAddr[3]
*/
void RF_SetMapmAddr(uint8_t *MapmAddr, uint8_t AddrWidth)
{
  RF_WritePageRegs(1, 0x3E, MapmAddr, AddrWidth);
}

/**
* @brief Calculate the time it takes to calculate a field (ms)
* @param <pMapmCfg> mapm configuration parameters
* <SymbolTime> Single symbol (chirp) time
* @note The number of chirps in Group1 is (pg1 + 2), where pg1 is the number of preambles in Group 1, and 2 is the number of chirps occupied by the addresses in Group 1.
* @note The number of chirps in other groups is (pgn + 2) * (gn - 1), where pgn is the number of preambles in another single group,
* 2 is the number of chirps occupied by the addresses (or count values) in another single group, and (gn - 1) is the number of groups remaining after removing Group 1.
*/
uint32_t RF_GetMapmOneFieldTime(RF_MapmCfg_t *pMapmCfg, uint32_t SymbolTime)
{
  uint8_t pgn = pMapmCfg->pgn;
  uint8_t pg1 = pMapmCfg->pg1;
  uint8_t gn = pMapmCfg->gn;
  uint16_t ChirpNumInOneField = (pg1 + 2) + (pgn + 2) * (gn - 1);
  return ChirpNumInOneField * SymbolTime / 1000;
}

/**
* @brief Get the remaining Mapm time in mapm mode
* @param <pMapmCfg> mapm configuration parameter
* <SymbolTime> Single symbol (chirp) time
* @return Remaining Mapm time, in milliseconds
* @note The remaining Mapm time is the time from the current moment to the completion of sending the remaining fields.
* @note The remaining field count includes the current field.
*/
uint32_t RF_GetLeftMapmTime(RF_MapmCfg_t *pMapmCfg, uint32_t SymbolTime)
{
  uint8_t fnm, gn, pgn, pg1, fn;
  uint16_t ChirpNumInOneField;
  uint16_t NumberOfLeftChirps;
  uint32_t LeftMapmTime;

  pgn = pMapmCfg->pgn;
  pg1 = pMapmCfg->pg1;
  gn = pMapmCfg->gn;
  fnm = pMapmCfg->fnm;
  fn = pMapmCfg->fn;
  /**
  * @brief Calculate the number of chirps in a field.
  * @note The number of chirps in Group 1 is (pg1 + 2), where pg1 is the number of preambles in Group 1, and 2 is the number of chirps occupied by the addresses in Group 1.
  * @note The number of chirps in other groups is (pgn + 2) * (gn - 1), where pgn is the number of preambles in other groups,
  * (gn - 1) is the number of groups remaining after removing Group 1, and 2 is the number of chirps occupied by the addresses (or counts) in other groups.
  */
  ChirpNumInOneField = (pg1 + 2) + (pgn + 2) * (gn - 1);
  /**
  * @brief Calculating the number of remaining chirps
  * @note fn is the number of fields in mapm. If fnm > 0, the number of fields actually sent is fn * (2^fnm).
  * @note pn is the number of preambles between a field and the sync word, corresponding to pn chirps in the air.
  * @note The number of chirps per field is subtracted to account for the time spent on the field itself.
  */
  NumOfLeftChirps = (1 << fnm) * fn * ChirpNumInOneField - ChirpNumInOneField;
  /* The remaining time is the number of remaining chirps multiplied by the time of a single chirp. */
  LeftMapmTime = SymbolTime * NumberOfLeftChirps;
  return LeftMapmTime / 1000; /* Convert microseconds to milliseconds. */
}

/**
* @brief Start transmitting continuous carrier waves.
* @note The transmit power and frequency must be set before calling this function.
* @note After calling this function, the chip will remain in the transmitting state until the RF_StopTxContinuousWave() function is called to stop transmitting.
*/
void RF_StartTxContinuousWave(void)
{
  RF_WriteReg(0x02, RF_STATE_STB3); /* Set the chip to idle state. */
  RF_WritePageReg(0, 0x58, 0x00); /* Disable all RF interrupts */
  RF_SetTxMode(RF_TX_MODE_CONTINOUS); /* Set continuous transmit mode */
  RF_WritePageReg(1, 0x0C, 1); /* Set transmit data length to 1 byte */
  RF_TurnonPA(); /* Enable the PA before transmitting data */
  RF_WriteReg(0x02, RF_STATE_TX); /* Set the chip to transmit mode */
  RF_WriteReg(0x01, 0xFF); /* 0x01 is the FIFO register address. After writing data, start transmitting the carrier on the rising edge of CS */
  g_RfOperatetate = RF_STATE_TX;
}

/**
* @brief Stop transmitting continuous carrier waves
* @note After calling this function, the chip stops transmitting and enters standby mode.
*/
void RF_StopTxContinuousWave(void)
{
  RF_WriteReg(0x02, RF_STATE_STB3); /* Set the chip to idle state */
  RF_TurnoffPA(); /* Turn off the PA after transmission is complete */
  RF_WritePageReg(0, 0x58, 0x0F); /* Restore the RF default interrupt */
  g_RfOperatetate = RF_STATE_STB3;
}

/**
* @brief Handle RF interrupt events
* @note This function can be called from an interrupt service routine;
* It can also be called in a polling manner to handle RF interrupt events.
*/
void RF_IRQ_Process(void)
{
  if (CHECK_RF_IRQ()) /* RF interrupt detected, a high level indicates an interrupt */
  {
    uint8_t IRQFlag;

    IRQFlag = RF_GetIRQFlag(); /* Get interrupt flag */
    if (IRQFlag & RF_IRQ_TX_DONE) /* Transmit complete interrupt */
    {
      RF_TurnoffPA(); /* Turn off PA after transmission completes */
      RF_ClrIRQFlag(RF_IRQ_TX_DONE); /* Clear transmit complete interrupt flag */
      IRQFlag &= ~RF_IRQ_TX_DONE;
    }
    if (IRQFlag & RF_IRQ_RX_DONE) /* Receive complete interrupt */
    {
      g_RfRxPkt.Snr = RF_GetPktSnr(); /* Get the SNR value of the received packet */
      g_RfRxPkt.Rssi = RF_GetPktRssi(); /* Get the RSSI value of the received packet */
      /* Get the received data and length */
      g_RfRxPkt.RxLen = RF_GetRecvPayload((uint8_t *)g_RfRxPkt.RxBuf);
      RF_ClrIRQFlag(RF_IRQ_RX_DONE); /* Clear the receive complete interrupt flag */
      IRQFlag &= ~RF_IRQ_RX_DONE;
    }
    if (IRQFlag & RF_IRQ_MAPM_DONE) /* Mapm receive complete interrupt */
    {
      uint8_t MapmAddr = RF_ReadPageReg(0, 0x6E);
      g_RfRxPkt.MapmRxBuf[g_RfRxPkt.MapmRxIndex++] = MapmAddr;
      RF_ClrIRQFlag(RF_IRQ_MAPM_DONE); /* Clear the MAMP receive completion interrupt flag */
      IRQFlag &= ~RF_IRQ_MAPM_DONE;
    }
    if (IRQFlag & RF_IRQ_CRC_ERR) /* CRC error interrupt */
    {
      RF_ClrIRQFlag(RF_IRQ_CRC_ERR); /* Clear the CRC error interrupt flag */
      IRQFlag &= ~RF_IRQ_CRC_ERR;
    }
    if (IRQFlag & RF_IRQ_RX_TIMEOUT) /* Receive timeout interrupt */
    {
      /* rf_refresh(); */
      IRQFlag &= ~RF_IRQ_RX_TIMEOUT;
      RF_ClrIRQFlag(RF_IRQ_RX_TIMEOUT); /* Clear the receive timeout interrupt flag */
    }
    if (IRQFlag) RF_ClrIRQFlag(IRQFlag); /* Clear the pending interrupt flag */
  }
}
