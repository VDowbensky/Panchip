#include "mtb_kvstore.h"
#include "PANSeries.h"

//--------------------------------------------------------------------------------------------------
// bd_read_size
//--------------------------------------------------------------------------------------------------
static uint32_t bd_read_size(void* context, uint32_t addr)
{
    CY_UNUSED_PARAMETER(context);
    CY_UNUSED_PARAMETER(addr);
    return 1;
}
 
 
//--------------------------------------------------------------------------------------------------
// bd_program_size
//--------------------------------------------------------------------------------------------------
static uint32_t bd_program_size(void* context, uint32_t addr)
{
    CY_UNUSED_PARAMETER(context);
    return PAGE_SIZE;
}
 
 
//--------------------------------------------------------------------------------------------------
// bd_erase_size
//--------------------------------------------------------------------------------------------------
static uint32_t bd_erase_size(void* context, uint32_t addr)
{
    CY_UNUSED_PARAMETER(context);
    return SECTOR_SIZE;
}
 
 
//--------------------------------------------------------------------------------------------------
// bd_read
//--------------------------------------------------------------------------------------------------
static cy_rslt_t bd_read(void* context, uint32_t addr, uint32_t length, uint8_t* buf)
{
    CY_UNUSED_PARAMETER(context);
    
    return FMC_ReadStream(FLCTL, addr, CMD_DREAD, buf, length);
}
 
//--------------------------------------------------------------------------------------------------
// bd_program
//--------------------------------------------------------------------------------------------------
static cy_rslt_t bd_program(void* context, uint32_t addr, uint32_t length, const uint8_t* buf)
{
    CY_UNUSED_PARAMETER(context);
    return FMC_WriteStream(FLCTL, addr, (unsigned char*)buf, length);
}

//--------------------------------------------------------------------------------------------------
// bd_erase
//--------------------------------------------------------------------------------------------------
static cy_rslt_t bd_erase(void* context, uint32_t addr, uint32_t length)
{
    CY_UNUSED_PARAMETER(context);
    return FMC_EraseCodeArea(FLCTL, addr, length);
}
 
static mtb_kvstore_bd_t block_device =
{
    .read         = bd_read,
    .program      = bd_program,
    .erase        = bd_erase,
    .read_size    = bd_read_size,
    .program_size = bd_program_size,
    .erase_size   = bd_erase_size,
    .context      = NULL
};


/** 
 * KV stroe object
 */
mtb_kvstore_t mtb_obj;
static bool kv_inited = false;

cy_rslt_t app_kv_write(const char* key, const uint8_t* data, uint32_t size)
{
    return mtb_kvstore_write(&mtb_obj, key, data, size);
}

cy_rslt_t app_kv_read(const char* key, uint8_t* data, uint32_t* size)
{
    return mtb_kvstore_read(&mtb_obj, key, data, size);
}

cy_rslt_t app_kv_delete(const char* key)
{
    return mtb_kvstore_delete(&mtb_obj, key);
}

cy_rslt_t app_kv_key_exists(const char *key)
{
    return mtb_kvstore_key_exists(&mtb_obj, key);
}

void kv_init(void)
{
    if(kv_inited){
        return;
    }

    uint32_t length = CONFIG_FLASH_PARTITION_KVSTORE_SIZE;
    uint32_t start_addr = CONFIG_FLASH_PARTITION_KVSTORE_ADDR; 
    
    cy_rslt_t result = mtb_kvstore_init(&mtb_obj, start_addr, length, &block_device);
    
    CY_ASSERT(result == CY_RSLT_SUCCESS);   

    kv_inited = true;
}
