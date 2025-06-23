#include "ff.h" /* Obtains integer types */
#include "diskio.h"
#include "string.h"
#include "sfud.h"

#pragma diag_suppress 177

#if CONFIG_OS_EN
#include "FreeRTOS.h"
#else
#error "OS Should be enabled here!"
#endif

#if FF_USE_TRIM == 1
#error "The FF_USE_TRIM feature is not supported here!"
#endif

#if DEBUG_EN
#define FF_LOG(fmt, ...)    SYS_PRINT("[FF] " fmt, ##__VA_ARGS__)
#else
#define FF_LOG(...)
#endif

#ifndef EXTERNAL_FLASH_DISK_START_ADDR
#define EXTERNAL_FLASH_DISK_START_ADDR      (0x0)       // Start Address of SPI Flash for Disk Use (Default: 0x0)
#endif
#ifndef EXTERNAL_FLASH_DISK_SIZE
#define EXTERNAL_FLASH_DISK_SIZE            (0x100000)  // Size of SPI Flash for Disk Use (Default: 1MB)
#endif

// The Flash disk sector size for FatFs should be in range of [FF_MIN_SS, FF_MAX_SS], see detail in ffconf.h
// but as the bigger sector size requires bigger sram buffer, so here we fix this size to 512 (strongly recommend)!
#define EXTERNAL_FLASH_DISK_SECTOR_SIZE     512
#define EXTERNAL_FLASH_DISK_SECTOR_COUNT    (EXTERNAL_FLASH_DISK_SIZE / EXTERNAL_FLASH_DISK_SECTOR_SIZE)

// All SPI flash supports 4KB Sector Erase, and some of them also support smaller erase size (e.g. 256B Page Erase)
#ifndef EXTERNAL_FLASH_ERASE_SIZE
#define EXTERNAL_FLASH_ERASE_SIZE           4096
#endif

#if EXTERNAL_FLASH_DISK_START_ADDR % 4096
#error "External flash disk start address should be 4KB aligned!"
#endif

#if EXTERNAL_FLASH_DISK_SIZE % 4096
#error "External flash disk size should be multiple of 4096 (4KB)!"
#endif

#if (EXTERNAL_FLASH_ERASE_SIZE > EXTERNAL_FLASH_DISK_SECTOR_SIZE) && (EXTERNAL_FLASH_ERASE_SIZE % EXTERNAL_FLASH_DISK_SECTOR_SIZE != 0)
#error "Internal flash erase size should be multiple of flash disk sector size!"
#endif

#if (EXTERNAL_FLASH_DISK_SECTOR_SIZE > EXTERNAL_FLASH_ERASE_SIZE) && (EXTERNAL_FLASH_DISK_SECTOR_SIZE % EXTERNAL_FLASH_ERASE_SIZE != 0)
#error "Internal flash disk sector size should be multiple of flash erase size!"
#endif

static BYTE *fd_buf = NULL;
static uint32_t buffered_flash_sector_addr = 0xFFFFFFFF;
static sfud_flash *flash_dev;

int spi_flash_memory_read(const uint8_t *buf, uint32_t flash_addr, size_t nbytes)
{
#if EXTERNAL_FLASH_ERASE_SIZE <= EXTERNAL_FLASH_DISK_SECTOR_SIZE
    sfud_read(flash_dev, flash_addr, nbytes, (uint8_t *)buf);
#else
    if (buffered_flash_sector_addr == 0xFFFFFFFF) {
        sfud_read(flash_dev, flash_addr, nbytes, (uint8_t *)buf);
    } else {
        if (fd_buf == NULL) {
            FF_LOG("%s(): Error, fd_buf is NULL!\n", __FUNCTION__);
            return 1;
        }
        if (flash_addr + nbytes <= buffered_flash_sector_addr) {
            sfud_read(flash_dev, flash_addr, nbytes, (uint8_t *)buf);
        } else if ((flash_addr < buffered_flash_sector_addr) && (flash_addr + nbytes > buffered_flash_sector_addr) && (flash_addr + nbytes <= buffered_flash_sector_addr + EXTERNAL_FLASH_ERASE_SIZE)) {
            sfud_read(flash_dev, flash_addr, buffered_flash_sector_addr - flash_addr, (uint8_t *)buf);
            memcpy((void *)(buf + buffered_flash_sector_addr - flash_addr), fd_buf, flash_addr + nbytes - buffered_flash_sector_addr);
        } else if ((flash_addr < buffered_flash_sector_addr) && (flash_addr + nbytes > buffered_flash_sector_addr) && (flash_addr + nbytes > buffered_flash_sector_addr + EXTERNAL_FLASH_ERASE_SIZE)) {
            sfud_read(flash_dev, flash_addr, buffered_flash_sector_addr - flash_addr, (uint8_t *)buf);
            memcpy((void *)(buf + buffered_flash_sector_addr - flash_addr), fd_buf, EXTERNAL_FLASH_ERASE_SIZE);
            sfud_read(flash_dev, buffered_flash_sector_addr + EXTERNAL_FLASH_ERASE_SIZE, flash_addr + nbytes - buffered_flash_sector_addr - EXTERNAL_FLASH_ERASE_SIZE, (uint8_t *)(buf + buffered_flash_sector_addr - flash_addr + EXTERNAL_FLASH_ERASE_SIZE));
        } else if ((flash_addr >= buffered_flash_sector_addr) && (flash_addr < buffered_flash_sector_addr + EXTERNAL_FLASH_ERASE_SIZE) && (flash_addr + nbytes <= buffered_flash_sector_addr + EXTERNAL_FLASH_ERASE_SIZE)) {
            memcpy((void *)buf, fd_buf + flash_addr - buffered_flash_sector_addr, nbytes);
        } else if ((flash_addr >= buffered_flash_sector_addr) && (flash_addr < buffered_flash_sector_addr + EXTERNAL_FLASH_ERASE_SIZE) && (flash_addr + nbytes > buffered_flash_sector_addr + EXTERNAL_FLASH_ERASE_SIZE)) {
            memcpy((void *)buf, fd_buf + flash_addr - buffered_flash_sector_addr, buffered_flash_sector_addr + EXTERNAL_FLASH_ERASE_SIZE - flash_addr);
            sfud_read(flash_dev, buffered_flash_sector_addr + EXTERNAL_FLASH_ERASE_SIZE, flash_addr + nbytes - buffered_flash_sector_addr - EXTERNAL_FLASH_ERASE_SIZE, (uint8_t *)(buf + buffered_flash_sector_addr + EXTERNAL_FLASH_ERASE_SIZE - flash_addr));
        } else if (flash_addr >= buffered_flash_sector_addr + EXTERNAL_FLASH_ERASE_SIZE) {
            sfud_read(flash_dev, flash_addr, nbytes, (uint8_t *)buf);
        }
    }
#endif

    return 0;
}

int spi_flash_memory_write(const uint8_t *buf, uint32_t flash_addr, size_t nbytes)
{
#if EXTERNAL_FLASH_ERASE_SIZE <= EXTERNAL_FLASH_DISK_SECTOR_SIZE
    // Both flash_addr adn nbytes should be multiple of EXTERNAL_FLASH_ERASE_SIZE
    SYS_ASSERT((flash_addr % EXTERNAL_FLASH_ERASE_SIZE) == 0);
    SYS_ASSERT((nbytes % EXTERNAL_FLASH_ERASE_SIZE) == 0);

    // Erase and write flash
#if EXTERNAL_FLASH_ERASE_SIZE == 256
    sfud_page_erase(flash_dev, flash_addr, nbytes);
#else
    if (flash_dev->chip.erase_gran != EXTERNAL_FLASH_ERASE_SIZE) {
        FF_LOG("SFUD flash erase size should be exactly equal with EXTERNAL_FLASH_ERASE_SIZE!\n");
        return 1;
    }
    sfud_erase(flash_dev, flash_addr, nbytes);
#endif
    sfud_write(flash_dev, flash_addr, nbytes, (uint8_t *)buf);
#else
    uint32_t offset_of_first_sector = flash_addr % EXTERNAL_FLASH_ERASE_SIZE;
    uint32_t write_sz_in_first_sector = (nbytes < (EXTERNAL_FLASH_ERASE_SIZE - offset_of_first_sector)) ? nbytes : (EXTERNAL_FLASH_ERASE_SIZE - offset_of_first_sector);
    uint32_t first_sector_start_addr = flash_addr - offset_of_first_sector;
    uint32_t offset_of_last_sector = (flash_addr + nbytes) % EXTERNAL_FLASH_ERASE_SIZE;
    uint32_t write_sz_in_last_sector = (nbytes > write_sz_in_first_sector) ? offset_of_last_sector : 0;
    uint32_t last_sector_start_addr = (flash_addr + nbytes) - offset_of_last_sector;

    size_t middle_sector_num = (last_sector_start_addr > first_sector_start_addr) ? ((last_sector_start_addr - first_sector_start_addr) / EXTERNAL_FLASH_ERASE_SIZE - 1) : 0;

    const uint8_t *tmp_buf = buf;
    uint32_t tmp_flash_addr = flash_addr;

    // 1. Write first sector
    // Allocate memory buffer and store current flash sector in it if fd_buf is NULL
    if (fd_buf == NULL) {
        fd_buf = (BYTE *)pvPortMalloc(sizeof(BYTE) * EXTERNAL_FLASH_ERASE_SIZE);
        if (fd_buf == NULL) {
            FF_LOG("%s(): Failed to allocate memory from os heap!\n", __FUNCTION__);
            return 1;
        }
        // Read the whole current flash sector to fd_buf
        sfud_read(flash_dev, first_sector_start_addr, EXTERNAL_FLASH_ERASE_SIZE, (uint8_t *)fd_buf);
        // Record current buffered flash sector address
        buffered_flash_sector_addr = first_sector_start_addr;
    }

    if (buffered_flash_sector_addr == 0xFFFFFFFF) {
        FF_LOG("%s(): Failed to record current buffered flash sector address!\n", __FUNCTION__);
        return 1;
    }

    if (buffered_flash_sector_addr != first_sector_start_addr) {
        // Erase previous buffered flash sector
        sfud_erase(flash_dev, buffered_flash_sector_addr, EXTERNAL_FLASH_ERASE_SIZE);
        // Write previous buffered data to flash
        sfud_write(flash_dev, buffered_flash_sector_addr, EXTERNAL_FLASH_ERASE_SIZE, (uint8_t *)fd_buf);
        // Buffer the new flash sector
        sfud_read(flash_dev, first_sector_start_addr, EXTERNAL_FLASH_ERASE_SIZE, (uint8_t *)fd_buf);
        // Record current buffered flash sector address
        buffered_flash_sector_addr = first_sector_start_addr;
    }
    // Write buffer instead of write flash directly
    memcpy(fd_buf + offset_of_first_sector, tmp_buf, write_sz_in_first_sector);

    tmp_flash_addr += write_sz_in_first_sector;
    tmp_buf += write_sz_in_first_sector;

    // 2. Write middle sectors
    while (middle_sector_num--) {
        // Erase previous buffered flash sector
        sfud_erase(flash_dev, buffered_flash_sector_addr, EXTERNAL_FLASH_ERASE_SIZE);
        // Write previous buffered data to flash
        sfud_write(flash_dev, tmp_flash_addr, EXTERNAL_FLASH_ERASE_SIZE, (uint8_t *)tmp_buf);

        tmp_flash_addr += EXTERNAL_FLASH_ERASE_SIZE;
        tmp_buf += EXTERNAL_FLASH_ERASE_SIZE;
    }

    // 3. Write last sector
    // FIXME: Here we shouldn't use the same fd_buf and buffered_flash_sector_addr to
    //        store buffered data info. Need to create new ones.
    if (write_sz_in_last_sector) {
        // Allocate 4KB memory buffer and store current 4KB flash sector in it if fd_buf is NULL
        if (fd_buf == NULL) {
            fd_buf = (BYTE *)pvPortMalloc(sizeof(BYTE) * EXTERNAL_FLASH_ERASE_SIZE);
            if (fd_buf == NULL) {
                FF_LOG("%s(): Failed to allocate memory from os heap!\n", __FUNCTION__);
                return 1;
            }
            // Read the whole current flash sector to fd_buf
            sfud_read(flash_dev, first_sector_start_addr, EXTERNAL_FLASH_ERASE_SIZE, (uint8_t *)fd_buf);
            // Record current buffered flash sector address
            buffered_flash_sector_addr = last_sector_start_addr;
        }

        if (buffered_flash_sector_addr == 0xFFFFFFFF) {
            FF_LOG("%s(): Failed to record current buffered flash sector address!\n", __FUNCTION__);
            return 1;
        }

        if (buffered_flash_sector_addr != last_sector_start_addr) {
            // Erase previous buffered flash sector
            sfud_erase(flash_dev, buffered_flash_sector_addr, EXTERNAL_FLASH_ERASE_SIZE);
            // Write previous buffered data to flash
            sfud_write(flash_dev, buffered_flash_sector_addr, EXTERNAL_FLASH_ERASE_SIZE, (uint8_t *)fd_buf);
            // Buffer the new flash sector
            sfud_read(flash_dev, last_sector_start_addr, EXTERNAL_FLASH_ERASE_SIZE, (uint8_t *)fd_buf);
            // Record current buffered flash sector address
            buffered_flash_sector_addr = last_sector_start_addr;
        }
        // Write buffer instead of write flash directly
        memcpy(fd_buf, tmp_buf, write_sz_in_last_sector);
    }
#endif

    return 0;
}

void spi_flash_memory_write_sync(void)
{
#if EXTERNAL_FLASH_ERASE_SIZE <= EXTERNAL_FLASH_DISK_SECTOR_SIZE
    // Do nothing here
#else
    if (fd_buf != NULL) {
        // Erase previous buffered flash sector
        sfud_erase(flash_dev, buffered_flash_sector_addr, EXTERNAL_FLASH_ERASE_SIZE);
        // Write previous buffered data to flash
        sfud_write(flash_dev, buffered_flash_sector_addr, EXTERNAL_FLASH_ERASE_SIZE, (uint8_t *)fd_buf);
        // Reset current buffered flash sector address
        buffered_flash_sector_addr = 0xFFFFFFFF;
        // Free fd_buf and reset its value to NULL
        vPortFree(fd_buf);
        fd_buf = NULL;
    }
#endif
}

DRESULT spi_flash_disk_status(void)
{
    FF_LOG("%s() in..\n", __FUNCTION__);

    return RES_OK;
}

DRESULT spi_flash_disk_initialize(void)
{
    FF_LOG("%s() in..\n", __FUNCTION__);

    // Init SFUD driver
    if (sfud_init() != SFUD_SUCCESS) {
        FF_LOG("SFUD init fail!\n");
        return RES_ERROR;
    }

    // Store the device handle for later use
    flash_dev = sfud_get_device(0);

#if EXTERNAL_FLASH_ERASE_SIZE > EXTERNAL_FLASH_DISK_SECTOR_SIZE
    if (flash_dev->chip.erase_gran != EXTERNAL_FLASH_ERASE_SIZE) {
        FF_LOG("SFUD flash erase size should be exactly equal with EXTERNAL_FLASH_ERASE_SIZE!\n");
        return RES_ERROR;
    }
#endif

    return RES_OK;
}

DRESULT spi_flash_disk_read(BYTE *buff, LBA_t sector, UINT count)
{
    FF_LOG("%s(): Sector Offset = %d, Sector Count = %d\n", __FUNCTION__, sector, count);

    if (spi_flash_memory_read(buff, EXTERNAL_FLASH_DISK_START_ADDR + sector * EXTERNAL_FLASH_DISK_SECTOR_SIZE, count * EXTERNAL_FLASH_DISK_SECTOR_SIZE)) {
        return RES_ERROR;
    }

    return RES_OK;
}

DRESULT spi_flash_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
    FF_LOG("%s(): Sector Offset = %d, Sector Count = %d\n", __FUNCTION__, sector, count);

    if (spi_flash_memory_write(buff, EXTERNAL_FLASH_DISK_START_ADDR + sector * EXTERNAL_FLASH_DISK_SECTOR_SIZE, count * EXTERNAL_FLASH_DISK_SECTOR_SIZE)) {
        return RES_ERROR;
    }

    return RES_OK;
}

DRESULT spi_flash_disk_ioctl(BYTE cmd, void *buff)
{
    DRESULT result = RES_OK;

//    FF_LOG("%s(): cmd = %d\n", __FUNCTION__, cmd);

    switch (cmd) {
        /* Make sure that no pending write process */
        case CTRL_SYNC:
            FF_LOG("%s(): CTRL_SYNC\n", __FUNCTION__);
            spi_flash_memory_write_sync();
            result = RES_OK;
            break;
        /* Get R/W sector size (WORD) */
        case GET_SECTOR_SIZE:
            *(WORD *)buff = EXTERNAL_FLASH_DISK_SECTOR_SIZE;
            FF_LOG("%s(): GET_SECTOR_SIZE: Sector Size = %d\n", __FUNCTION__, *(WORD *)buff);
            result = RES_OK;
            break;
        /* Get erase block size in unit of sector (DWORD) */
        case GET_BLOCK_SIZE:
            *(DWORD *)buff = EXTERNAL_FLASH_ERASE_SIZE / EXTERNAL_FLASH_DISK_SECTOR_SIZE;
            FF_LOG("%s(): GET_BLOCK_SIZE: Erase Block Size = %d\n", __FUNCTION__, *(DWORD *)buff);
            result = RES_OK;
            break;
        /* Get number of sectors on the disk (DWORD) */
        case GET_SECTOR_COUNT:
            *(DWORD *)buff = EXTERNAL_FLASH_DISK_SECTOR_COUNT;
            FF_LOG("%s(): GET_SECTOR_COUNT: Sector Count = %d\n", __FUNCTION__, *(DWORD *)buff);
            result = RES_OK;
            break;
#if FF_USE_TRIM == 1
        #define START_LBA   (*(DWORD *)buff)
        #define END_LBA     (*((DWORD *)buff + 1))
        /* Informs the data on the block of sectors can be erased. */
        case CTRL_TRIM:
            // TBD
            break;
#endif
        default:
            FF_LOG("%s(): Unrecognized cmd: %d\n", __FUNCTION__, cmd);
            result = RES_PARERR;
            break;
    }

    return result;
}
