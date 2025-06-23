#include "ff.h" /* Obtains integer types */
#include "diskio.h"
#include "string.h"
#include "PanSeries.h"
#include "utility.h"

#pragma diag_suppress 177

#if CONFIG_OS_EN
#include "FreeRTOS.h"
#else
#error "OS Should be enabled here!"
#endif

#if DEBUG_EN
#define FF_LOG(fmt, ...)    SYS_PRINT("[FF] " fmt, ##__VA_ARGS__)
#define FF_LOG_DIRECT(...)  SYS_PRINT(##__VA_ARGS__)
#else
#define FF_LOG(...)
#define FF_LOG_DIRECT(...)
#endif

#define INTERNAL_FLASH_DISK_START_ADDR      (CONFIG_FLASH_PARTITION_USER_CUSTOM_ADDR)
#define INTERNAL_FLASH_DISK_SIZE            (CONFIG_FLASH_PARTITION_USER_CUSTOM_SIZE)
// The Flash disk sector size for FatFs should be in range of [FF_MIN_SS, FF_MAX_SS], see detail in ffconf.h
// but as the soc flash is not large enough for bigger sector size, so here we fix this size to 512 (strongly recommend)!
#define INTERNAL_FLASH_DISK_SECTOR_SIZE     512
#define INTERNAL_FLASH_DISK_SECTOR_COUNT    (INTERNAL_FLASH_DISK_SIZE / INTERNAL_FLASH_DISK_SECTOR_SIZE)

// All PAN107x SoC supports 4KB Sector Erase, and some of them also support smaller erase size (e.g. 256B Page Erase)
#ifndef INTERNAL_FLASH_ERASE_SIZE
#define INTERNAL_FLASH_ERASE_SIZE           4096
#endif

#if INTERNAL_FLASH_DISK_START_ADDR % 4096
#error "Internal flash disk start address should be 4KB aligned!"
#endif

#if INTERNAL_FLASH_DISK_SIZE % 4096
#error "Internal flash disk size should be multiple of 4096 (4KB)!"
#endif

#if (INTERNAL_FLASH_ERASE_SIZE != 256) && (INTERNAL_FLASH_ERASE_SIZE != 4096)
#error "Unsupported INTERNAL_FLASH_ERASE_SIZE!"
#endif

#if (INTERNAL_FLASH_ERASE_SIZE > INTERNAL_FLASH_DISK_SECTOR_SIZE) && (INTERNAL_FLASH_ERASE_SIZE % INTERNAL_FLASH_DISK_SECTOR_SIZE != 0)
#error "Internal flash erase size should be multiple of flash disk sector size!"
#endif

#if (INTERNAL_FLASH_DISK_SECTOR_SIZE > INTERNAL_FLASH_ERASE_SIZE) && (INTERNAL_FLASH_DISK_SECTOR_SIZE % INTERNAL_FLASH_ERASE_SIZE != 0)
#error "Internal flash disk sector size should be multiple of flash erase size!"
#endif

static BYTE *fd_buf = NULL;
static uint32_t buffered_flash_sector_addr = 0xFFFFFFFF;

int soc_flash_memory_read(const uint8_t *buf, uint32_t flash_addr, size_t nbytes)
{
#if INTERNAL_FLASH_ERASE_SIZE <= INTERNAL_FLASH_DISK_SECTOR_SIZE
    FMC_ReadStream(FLCTL, flash_addr, CMD_DREAD, (uint8_t *)buf, nbytes);
#else
    if (buffered_flash_sector_addr == 0xFFFFFFFF) {
        FF_LOG("%s(): tp0 Read Flash!\n", __FUNCTION__);
        FMC_ReadStream(FLCTL, flash_addr, CMD_DREAD, (uint8_t *)buf, nbytes);
    } else {
        if (fd_buf == NULL) {
            FF_LOG("%s(): Error, fd_buf is NULL!\n", __FUNCTION__);
            return 1;
        }
        if (flash_addr + nbytes <= buffered_flash_sector_addr) {
            FF_LOG("%s(): tp1 Read Flash!\n", __FUNCTION__);
            FMC_ReadStream(FLCTL, flash_addr, CMD_DREAD, (uint8_t *)buf, nbytes);
        } else if ((flash_addr < buffered_flash_sector_addr) && (flash_addr + nbytes > buffered_flash_sector_addr) && (flash_addr + nbytes <= buffered_flash_sector_addr + INTERNAL_FLASH_ERASE_SIZE)) {
            FF_LOG("%s(): tp2 Read Flash and Buffer!\n", __FUNCTION__);
            FMC_ReadStream(FLCTL, flash_addr, CMD_DREAD, (uint8_t *)buf, buffered_flash_sector_addr - flash_addr);
            memcpy((void *)(buf + buffered_flash_sector_addr - flash_addr), fd_buf, flash_addr + nbytes - buffered_flash_sector_addr);
        } else if ((flash_addr < buffered_flash_sector_addr) && (flash_addr + nbytes > buffered_flash_sector_addr) && (flash_addr + nbytes > buffered_flash_sector_addr + INTERNAL_FLASH_ERASE_SIZE)) {
            FF_LOG("%s(): tp3 Read Flash and Buffer!\n", __FUNCTION__);
            FMC_ReadStream(FLCTL, flash_addr, CMD_DREAD, (uint8_t *)buf, buffered_flash_sector_addr - flash_addr);
            memcpy((void *)(buf + buffered_flash_sector_addr - flash_addr), fd_buf, INTERNAL_FLASH_ERASE_SIZE);
            FMC_ReadStream(FLCTL, buffered_flash_sector_addr + INTERNAL_FLASH_ERASE_SIZE, CMD_DREAD, (uint8_t *)(buf + buffered_flash_sector_addr - flash_addr + INTERNAL_FLASH_ERASE_SIZE), flash_addr + nbytes - buffered_flash_sector_addr - INTERNAL_FLASH_ERASE_SIZE);
        } else if ((flash_addr >= buffered_flash_sector_addr) && (flash_addr < buffered_flash_sector_addr + INTERNAL_FLASH_ERASE_SIZE) && (flash_addr + nbytes <= buffered_flash_sector_addr + INTERNAL_FLASH_ERASE_SIZE)) {
            FF_LOG("%s(): tp4 Read Buffer!\n", __FUNCTION__);
            memcpy((void *)buf, fd_buf + flash_addr - buffered_flash_sector_addr, nbytes);
        } else if ((flash_addr >= buffered_flash_sector_addr) && (flash_addr < buffered_flash_sector_addr + INTERNAL_FLASH_ERASE_SIZE) && (flash_addr + nbytes > buffered_flash_sector_addr + INTERNAL_FLASH_ERASE_SIZE)) {
            FF_LOG("%s(): tp5 Read Flash and Buffer!\n", __FUNCTION__);
            memcpy((void *)buf, fd_buf + flash_addr - buffered_flash_sector_addr, buffered_flash_sector_addr + INTERNAL_FLASH_ERASE_SIZE - flash_addr);
            FMC_ReadStream(FLCTL, buffered_flash_sector_addr + INTERNAL_FLASH_ERASE_SIZE, CMD_DREAD, (uint8_t *)(buf + buffered_flash_sector_addr + INTERNAL_FLASH_ERASE_SIZE - flash_addr), flash_addr + nbytes - buffered_flash_sector_addr - INTERNAL_FLASH_ERASE_SIZE);
        } else if (flash_addr >= buffered_flash_sector_addr + INTERNAL_FLASH_ERASE_SIZE) {
            FF_LOG("%s(): tp6 Read Flash!\n", __FUNCTION__);
            FMC_ReadStream(FLCTL, flash_addr, CMD_DREAD, (uint8_t *)buf, nbytes);
        }
    }
#endif

    return 0;
}

static void FMC_EraseWrapper(FLCTL_T *fmc, uint32_t addr, uint32_t len)
{
    extern int FMC_Erase(FLCTL_T *fmc,unsigned int Addr,unsigned char cmd);

    for (size_t i = addr; i < addr + len; i += INTERNAL_FLASH_ERASE_SIZE) {
#if INTERNAL_FLASH_ERASE_SIZE == 256
        FMC_Erase(FLCTL, i, CMD_ERASE_PAGE);
#elif INTERNAL_FLASH_ERASE_SIZE == 4096
        FMC_Erase(FLCTL, i, CMD_ERASE_SECTOR);
#else
#error "Invalid INTERNAL_FLASH_ERASE_SIZE!"
#endif
    }
}

int soc_flash_memory_write(const uint8_t *buf, uint32_t flash_addr, size_t nbytes)
{
#if INTERNAL_FLASH_ERASE_SIZE <= INTERNAL_FLASH_DISK_SECTOR_SIZE
    // Both flash_addr adn nbytes should be multiple of INTERNAL_FLASH_ERASE_SIZE
    SYS_ASSERT((flash_addr % INTERNAL_FLASH_ERASE_SIZE) == 0);
    SYS_ASSERT((nbytes % INTERNAL_FLASH_ERASE_SIZE) == 0);

    // Erase and write flash
    FMC_EraseWrapper(FLCTL, flash_addr, nbytes);
    FMC_WriteStream(FLCTL, flash_addr, (uint8_t *)buf, nbytes);
#else
    uint32_t offset_of_first_sector = flash_addr % INTERNAL_FLASH_ERASE_SIZE;
    uint32_t write_sz_in_first_sector = (nbytes < (INTERNAL_FLASH_ERASE_SIZE - offset_of_first_sector)) ? nbytes : (INTERNAL_FLASH_ERASE_SIZE - offset_of_first_sector);
    uint32_t first_sector_start_addr = flash_addr - offset_of_first_sector;
    uint32_t offset_of_last_sector = (flash_addr + nbytes) % INTERNAL_FLASH_ERASE_SIZE;
    uint32_t write_sz_in_last_sector = (nbytes > write_sz_in_first_sector) ? offset_of_last_sector : 0;
    uint32_t last_sector_start_addr = (flash_addr + nbytes) - offset_of_last_sector;

    size_t middle_sector_num = (last_sector_start_addr > first_sector_start_addr) ? ((last_sector_start_addr - first_sector_start_addr) / INTERNAL_FLASH_ERASE_SIZE - 1) : 0;

    const uint8_t *tmp_buf = buf;
    uint32_t tmp_flash_addr = flash_addr;

    // 1. Write first sector
    // Allocate memory buffer and store current flash erase sector in it if fd_buf is NULL
    if (fd_buf == NULL) {
        fd_buf = (BYTE *)pvPortMalloc(sizeof(BYTE) * INTERNAL_FLASH_ERASE_SIZE);
        if (fd_buf == NULL) {
            FF_LOG("%s(): Failed to allocate memory from os heap!\n", __FUNCTION__);
            return 1;
        }
        // Read the whole current flash sector to fd_buf
        FMC_ReadStream(FLCTL, first_sector_start_addr, CMD_DREAD, (uint8_t *)fd_buf, INTERNAL_FLASH_ERASE_SIZE);
        // Record current buffered flash sector address
        buffered_flash_sector_addr = first_sector_start_addr;
        FF_LOG("%s(): First buffer data, cluster offset = %d!\n", __FUNCTION__, (buffered_flash_sector_addr - INTERNAL_FLASH_DISK_START_ADDR) / 512);
    }

    if (buffered_flash_sector_addr == 0xFFFFFFFF) {
        FF_LOG("%s(): Failed to record current buffered flash sector address!\n", __FUNCTION__);
        return 1;
    }

    if (buffered_flash_sector_addr != first_sector_start_addr) {
        // Erase previous buffered flash sector
        FMC_EraseWrapper(FLCTL, buffered_flash_sector_addr, INTERNAL_FLASH_ERASE_SIZE);
        // Write previous buffered data to flash
        FMC_WriteStream(FLCTL, buffered_flash_sector_addr, (uint8_t *)fd_buf, INTERNAL_FLASH_ERASE_SIZE);
        // Buffer the new flash sector
        FMC_ReadStream(FLCTL, first_sector_start_addr, CMD_DREAD, (uint8_t *)fd_buf, INTERNAL_FLASH_ERASE_SIZE);
        // Record current buffered flash sector address
        buffered_flash_sector_addr = first_sector_start_addr;
        FF_LOG("%s(): Re-buffer data, cluster offset = %d!\n", __FUNCTION__, (buffered_flash_sector_addr - INTERNAL_FLASH_DISK_START_ADDR) / 512);
    }
    FF_LOG("%s(): Write Buffer!\n", __FUNCTION__);
    // Write buffer instead of write flash directly
    memcpy(fd_buf + offset_of_first_sector, tmp_buf, write_sz_in_first_sector);

    tmp_flash_addr += write_sz_in_first_sector;
    tmp_buf += write_sz_in_first_sector;

    // 2. Write middle sectors
    while (middle_sector_num--) {
        FF_LOG("%s(): Erase and Write Flash Middle Sectors, addr: 0x%x, size: 0x%x!\n", __FUNCTION__, tmp_flash_addr, INTERNAL_FLASH_ERASE_SIZE);
        // Erase previous buffered flash sector
        FMC_EraseWrapper(FLCTL, tmp_flash_addr, INTERNAL_FLASH_ERASE_SIZE);
        // Write previous buffered data to flash
        FMC_WriteStream(FLCTL, tmp_flash_addr, (uint8_t *)tmp_buf, INTERNAL_FLASH_ERASE_SIZE);

        tmp_flash_addr += INTERNAL_FLASH_ERASE_SIZE;
        tmp_buf += INTERNAL_FLASH_ERASE_SIZE;
    }

    // 3. Write last sector
    // FIXME: Here we shouldn't use the same fd_buf and buffered_flash_sector_addr to
    //        store buffered data info. Need to create new ones.
    if (write_sz_in_last_sector) {
        FF_LOG("%s(): Write Last Sector!\n", __FUNCTION__);
        // Allocate 4KB memory buffer and store current 4KB flash sector in it if fd_buf is NULL
        if (fd_buf == NULL) {
            fd_buf = (BYTE *)pvPortMalloc(sizeof(BYTE) * INTERNAL_FLASH_ERASE_SIZE);
            if (fd_buf == NULL) {
                FF_LOG("%s(): Failed to allocate memory from os heap!\n", __FUNCTION__);
                return 1;
            }
            // Read the whole current flash sector to fd_buf
            FMC_ReadStream(FLCTL, last_sector_start_addr, CMD_DREAD, (uint8_t *)fd_buf, INTERNAL_FLASH_ERASE_SIZE);
            // Record current buffered flash sector address
            buffered_flash_sector_addr = last_sector_start_addr;
        }

        if (buffered_flash_sector_addr == 0xFFFFFFFF) {
            FF_LOG("%s(): Failed to record current buffered flash sector address!\n", __FUNCTION__);
            return 1;
        }

        if (buffered_flash_sector_addr != last_sector_start_addr) {
            // Erase previous buffered flash sector
            FMC_EraseWrapper(FLCTL, buffered_flash_sector_addr, INTERNAL_FLASH_ERASE_SIZE);
            // Write previous buffered data to flash
            FMC_WriteStream(FLCTL, buffered_flash_sector_addr, (uint8_t *)fd_buf, INTERNAL_FLASH_ERASE_SIZE);
            // Buffer the new flash sector
            FMC_ReadStream(FLCTL, last_sector_start_addr, CMD_DREAD, (uint8_t *)fd_buf, INTERNAL_FLASH_ERASE_SIZE);
            // Record current buffered flash sector address
            buffered_flash_sector_addr = last_sector_start_addr;
        }
        // Write buffer instead of write flash directly
        memcpy(fd_buf, tmp_buf, write_sz_in_last_sector);
    }
#endif

    return 0;
}

void soc_flash_memory_write_sync(void)
{
#if INTERNAL_FLASH_ERASE_SIZE <= INTERNAL_FLASH_DISK_SECTOR_SIZE
    // Do nothing here
#else
    if (fd_buf != NULL) {
        FF_LOG("%s(): Write buffered data to flash, cluster offset = %d!\n", __FUNCTION__, (buffered_flash_sector_addr - INTERNAL_FLASH_DISK_START_ADDR) / 512);
        // Erase previous buffered flash sector
        FMC_EraseWrapper(FLCTL, buffered_flash_sector_addr, INTERNAL_FLASH_ERASE_SIZE);
        // Write previous buffered data to flash
        FMC_WriteStream(FLCTL, buffered_flash_sector_addr, (uint8_t *)fd_buf, INTERNAL_FLASH_ERASE_SIZE);
        // Reset current buffered flash sector address
        buffered_flash_sector_addr = 0xFFFFFFFF;
        // Free fd_buf and reset its value to NULL
        vPortFree(fd_buf);
        fd_buf = NULL;
    }
#endif
}

DRESULT soc_flash_disk_status(void)
{
    FF_LOG("%s() in..\n", __FUNCTION__);

    return RES_OK;
}

DRESULT soc_flash_disk_initialize(void)
{
    FF_LOG("%s() in..\n", __FUNCTION__);

    return RES_OK;
}

DRESULT soc_flash_disk_read(BYTE *buff, LBA_t sector, UINT count)
{
    uint32_t key;

    FF_LOG("%s(): Sector Offset = %d, Sector Count = %d\n", __FUNCTION__, sector, count);

    key = irq_lock();

    if (soc_flash_memory_read(buff, INTERNAL_FLASH_DISK_START_ADDR + sector * INTERNAL_FLASH_DISK_SECTOR_SIZE, count * INTERNAL_FLASH_DISK_SECTOR_SIZE)) {
        irq_unlock(key);
        return RES_ERROR;
    }

    for (int i = 0; i < count * INTERNAL_FLASH_DISK_SECTOR_SIZE; i++) {
        FF_LOG_DIRECT("0x%02x ", *(uint8_t*)(buff + i));
        if ((i + 1) % 16 == 0) {
            FF_LOG_DIRECT("\n");
        }
    }
    FF_LOG_DIRECT("\n");

    irq_unlock(key);

    return RES_OK;
}

DRESULT soc_flash_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
    uint32_t key;

    FF_LOG("%s(): Sector Offset = %d, Sector Count = %d\n", __FUNCTION__, sector, count);

    key = irq_lock();

    if (soc_flash_memory_write(buff, INTERNAL_FLASH_DISK_START_ADDR + sector * INTERNAL_FLASH_DISK_SECTOR_SIZE, count * INTERNAL_FLASH_DISK_SECTOR_SIZE)) {
        irq_unlock(key);
        return RES_ERROR;
    }

    for (int i = 0; i < count * INTERNAL_FLASH_DISK_SECTOR_SIZE; i++) {
        FF_LOG_DIRECT("0x%02x ", *(uint8_t*)(buff + i));
        if ((i + 1) % 16 == 0) {
            FF_LOG_DIRECT("\n");
        }
    }
    FF_LOG_DIRECT("\n");

    irq_unlock(key);

    return RES_OK;
}

DRESULT soc_flash_disk_ioctl(BYTE cmd, void *buff)
{
    DRESULT result = RES_OK;

//    FF_LOG("%s(): cmd = %d\n", __FUNCTION__, cmd);

    switch (cmd) {
        /* Make sure that no pending write process */
        case CTRL_SYNC:
            FF_LOG("%s(): CTRL_SYNC\n", __FUNCTION__);
            soc_flash_memory_write_sync();
            result = RES_OK;
            break;
        /* Get R/W sector size (WORD) */
        case GET_SECTOR_SIZE:
            *(WORD *)buff = INTERNAL_FLASH_DISK_SECTOR_SIZE;
            FF_LOG("%s(): GET_SECTOR_SIZE: Sector Size = %d\n", __FUNCTION__, *(WORD *)buff);
            result = RES_OK;
            break;
        /* Get erase block size in unit of sector (DWORD) */
        case GET_BLOCK_SIZE:
            *(DWORD *)buff = INTERNAL_FLASH_ERASE_SIZE / INTERNAL_FLASH_DISK_SECTOR_SIZE;
            FF_LOG("%s(): GET_BLOCK_SIZE: Erase Block Size = %d\n", __FUNCTION__, *(DWORD *)buff);
            result = RES_OK;
            break;
        /* Get number of sectors on the disk (DWORD) */
        case GET_SECTOR_COUNT:
            *(DWORD *)buff = INTERNAL_FLASH_DISK_SECTOR_COUNT;
            FF_LOG("%s(): GET_SECTOR_COUNT: Sector Count = %d\n", __FUNCTION__, *(DWORD *)buff);
            result = RES_OK;
            break;
#if FF_USE_TRIM == 1
        #define START_LBA   (*(DWORD *)buff)
        #define END_LBA     (*((DWORD *)buff + 1))
        /* Informs the data on the block of sectors can be erased. */
        case CTRL_TRIM:
            FF_LOG("%s(): CTRL_TRIM: Start LBA = %d, End LBA = %d\n", __FUNCTION__, *(DWORD *)buff, *((DWORD *)buff + 1));
            if (((START_LBA * INTERNAL_FLASH_DISK_SECTOR_SIZE) % 4096) || (((END_LBA + 1) * INTERNAL_FLASH_DISK_SECTOR_SIZE) % 4096)) {
                result = RES_PARERR;
                break;
            }
            __disable_irq();
            FMC_EraseCodeArea(FLCTL, INTERNAL_FLASH_DISK_START_ADDR + START_LBA * INTERNAL_FLASH_DISK_SECTOR_SIZE, (END_LBA - START_LBA + 1) * INTERNAL_FLASH_DISK_SECTOR_SIZE);
            __enable_irq();
            break;
#endif
        default:
            FF_LOG("%s(): Unrecognized cmd: %d\n", __FUNCTION__, cmd);
            result = RES_PARERR;
            break;
    }

    return result;
}
