#ifndef __FLASH_USE_H
#define __FLASH_USE_H

#include "stm32f1xx_hal.h"

#define VERSION_INFO

#ifdef VERSION_INFO
    #pragma messages("create date: 2024-11-29 by meng")
    #pragma messages("software version: V1.0.0 ")
#endif

#define FLASH_SIZE 64

#if FLASH_SIZE < 256
    #define SECTOR_SIZE 1024
#else
    #define SECTOR_SIZE 2048
#endif

#define USER_DATA_PAGE_SIZE  2
#define USER_DATA_START_ADDR ((FLASH_BASE + 1024 * FLASH_SIZE) - (USER_DATA_PAGE_SIZE * SECTOR_SIZE))
#define OTA_FLAG_ADDRESS    0x0800F000  // Flash 最后 4KB 存储升级标志
#define OTA_FLAG_SET        0xa5a5a5a5  // OTA 更新标志
#define OTA_FLAG_CLEAR      0xffffffff  // 未设置OTA

#define BOOTLOADER_SIZE     16
#define FLASH_APP_START     0x08008000    // APP 固件起始地址    flash | app | boot = 64k | 
#define OTA_MAX_SIZE       ((FLASH_SIZE - BOOTLOADER_SIZE) * 1024)


#pragma pack(4)
typedef struct _key_val_t{
    char *key;
    unsigned int val;
}key_val_t;
#pragma pack()



uint16_t flash_read_halfword(uint32_t addr);
uint32_t flash_read_word(uint32_t addr);
void flash_read_multiple_datas(uint32_t start_addr, uint16_t *buffer, uint16_t count);
void flash_write_multiple_datas(uint32_t start_addr, uint16_t *buffer, uint16_t count);

uint8_t set_ota_update(void);
uint8_t check_ota_flag(void);

#endif
