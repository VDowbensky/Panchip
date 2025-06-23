/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"            /* Obtains integer types */
#include "diskio.h"        /* Declarations of disk functions */

#ifndef DISK_USE_EXTERNAL_SPI_FLASH
#define DISK_USE_EXTERNAL_SPI_FLASH         0
#endif

/* Definitions of physical drive number for each drive */
#define DEV_FLASH    0    /* Example: Map Flash to physical drive 0 */

#if DISK_USE_EXTERNAL_SPI_FLASH
extern DRESULT spi_flash_disk_status(void);
extern DRESULT spi_flash_disk_initialize(void);
extern DRESULT spi_flash_disk_read(BYTE *buff, LBA_t sector, UINT count);
extern DRESULT spi_flash_disk_write(const BYTE *buff, LBA_t sector, UINT count);
extern DRESULT spi_flash_disk_ioctl(BYTE cmd, void *buff);
#else
extern DRESULT soc_flash_disk_status(void);
extern DRESULT soc_flash_disk_initialize(void);
extern DRESULT soc_flash_disk_read(BYTE *buff, LBA_t sector, UINT count);
extern DRESULT soc_flash_disk_write(const BYTE *buff, LBA_t sector, UINT count);
extern DRESULT soc_flash_disk_ioctl(BYTE cmd, void *buff);
#endif

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv        /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat;

    switch (pdrv) {
    case DEV_FLASH:
#if DISK_USE_EXTERNAL_SPI_FLASH
        stat = spi_flash_disk_status();
#else
        stat = soc_flash_disk_status();
#endif
        return stat;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv                /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat;

    switch (pdrv) {
    case DEV_FLASH:
#if DISK_USE_EXTERNAL_SPI_FLASH
        stat = spi_flash_disk_initialize();
#else
        stat = soc_flash_disk_initialize();
#endif
        return stat;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,        /* Physical drive nmuber to identify the drive */
    BYTE *buff,        /* Data buffer to store read data */
    LBA_t sector,    /* Start sector in LBA */
    UINT count        /* Number of sectors to read */
)
{
    DRESULT res;

    switch (pdrv) {
    case DEV_FLASH:
#if DISK_USE_EXTERNAL_SPI_FLASH
        res = spi_flash_disk_read(buff, sector, count);
#else
        res = soc_flash_disk_read(buff, sector, count);
#endif
        return res;
    }

    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
    BYTE pdrv,            /* Physical drive nmuber to identify the drive */
    const BYTE *buff,    /* Data to be written */
    LBA_t sector,        /* Start sector in LBA */
    UINT count            /* Number of sectors to write */
)
{
    DRESULT res;

    switch (pdrv) {
    case DEV_FLASH:
#if DISK_USE_EXTERNAL_SPI_FLASH
        res = spi_flash_disk_write(buff, sector, count);
#else
        res = soc_flash_disk_write(buff, sector, count);
#endif
        return res;
    }

    return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,        /* Physical drive nmuber (0..) */
    BYTE cmd,        /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
    DRESULT res;

    switch (pdrv) {
    case DEV_FLASH:
#if DISK_USE_EXTERNAL_SPI_FLASH
        res = spi_flash_disk_ioctl(cmd, buff);
#else
        res = soc_flash_disk_ioctl(cmd, buff);
#endif
        return res;
    }

    return RES_PARERR;
}

// Get Time from RTC
// This function can be override by Application
__weak DWORD get_fattime(void)
{
    // e.g. 2025-2-20 18:30:58
    #define FF_YEAR     2025
    #define FF_MONTH    2
    #define FF_MDAY     20
    #define FF_HOUR     18
    #define FF_MINUTE   30
    #define FF_SECOND   58

    return (FF_YEAR - 1980) << 25 | FF_MONTH << 21 | FF_MDAY << 16 | FF_HOUR << 11 | FF_MINUTE << 5 | (FF_SECOND / 2) << 0;
}
