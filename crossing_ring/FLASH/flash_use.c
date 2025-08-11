#include "flash_use.h"
#include "stm32f1xx_hal_flash.h"
#include "string.h"

// extern int mylog(char *format, ...);

/* read half word, is 2 bytes */
uint16_t flash_read_halfword(uint32_t addr)
{
    return *(__IO uint16_t*)addr;
}

/* read word, is 4 bytes */
uint32_t flash_read_word(uint32_t addr)
{
    return (*(__IO uint16_t*)addr) + ((*(__IO uint16_t*)addr+2) << 16);
}

/* read count datas from start_addr */
void flash_read_multiple_datas(uint32_t start_addr, uint16_t *buffer, uint16_t count)
{
    uint16_t i;

    if(strlen((const char*)buffer) != 0)
    {
        memset(buffer, 0x0, count);
    }
    for(i = 0; i < count; i++)
    {
      buffer[i] = flash_read_halfword(start_addr + i * 2);
    }
}

void flash_write_multiple_datas(uint32_t start_addr, uint16_t *buffer, uint16_t count)
{
    FLASH_EraseInitTypeDef erase_t;

    uint32_t page_error;
    uint16_t i;

    if(start_addr < FLASH_BASE || ((start_addr + count * 2) >= (FLASH_BASE + 1024 * FLASH_SIZE)))
    {
        // mylog("overstep the flash size \r\n");
        return;
    }
    if((count * 2048) >= (FLASH_BASE + 1024 * USER_DATA_PAGE_SIZE))
    {
        // mylog("USER_DATA space is not enough \r\n");
        return;
    }
    HAL_FLASH_Unlock();
    uint32_t offset_addr = start_addr - FLASH_BASE;
    uint32_t sector_pos  = offset_addr/SECTOR_SIZE;

    uint32_t sector_startaddr = ((start_addr - FLASH_BASE)/SECTOR_SIZE) * SECTOR_SIZE + FLASH_BASE;

    erase_t.PageAddress = sector_startaddr;
    erase_t.TypeErase   = FLASH_TYPEERASE_PAGES;
    if((start_addr - FLASH_BASE) % SECTOR_SIZE == 0)
    {
        erase_t.NbPages     = USER_DATA_PAGE_SIZE;
    }
    else{
        erase_t.NbPages     = USER_DATA_PAGE_SIZE + 1;
    }
    
    if(HAL_FLASHEx_Erase(&erase_t, &page_error) == HAL_OK)
    {
        for(i = 0; i < count; i++)
        {
            if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, start_addr + i * 2, buffer[i]) !=  HAL_OK)
            {
                // mylog("program error \r\n");
                return;
            }
            
        }
        HAL_FLASH_Lock();
    }
}




uint8_t set_ota_update(void) 
{
    volatile uint32_t* tmp = (volatile uint32_t*)OTA_FLAG_ADDRESS;

    HAL_FLASH_Unlock();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, OTA_FLAG_ADDRESS, OTA_FLAG_SET);
    HAL_FLASH_Lock();

    if(*tmp == OTA_FLAG_SET)
    {
        NVIC_SystemReset();
    }

    return 1;
}


uint8_t check_ota_flag(void)
{
    volatile uint32_t* tmp = (volatile uint32_t*)OTA_FLAG_ADDRESS;

    if(*tmp == OTA_FLAG_SET)
    {
        HAL_FLASH_Unlock();
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, OTA_FLAG_ADDRESS, OTA_FLAG_CLEAR);
        HAL_FLASH_Lock();
    }

    if(*tmp == OTA_FLAG_CLEAR)return 0;
    else return 1;
}