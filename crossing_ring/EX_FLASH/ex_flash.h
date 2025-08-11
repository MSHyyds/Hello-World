#ifndef __EX_FLASH
#define __EX_FLASH

#include "stm32f1xx_hal.h"
#include "spi.h"



/**
 * @brief M25P16 Flash 已停产
 * single have 32 sectors, 1 sector have 256 pages, 1 page have 256 Bytes
 * total 2 MB : 2 * 1024 * 1024 bytes
 * 1 sector   : 256 * 256 / 1024 = 64 KB
 * 
 */

#define EXFLASH_SECTOR(ch)      ((uint32_t)ch*65536)      // 第ch块的起始地址
#define APP_UPDATE_START_ADDR   EXFLASH_SECTOR(0)         // 64 * 4 = 256 KB
#define APP_BACKUP_START_ADDR   EXFLASH_SECTOR(4)          
#define APP_PARAM_START_ADDR    EXFLASH_SECTOR(31)        // 64 KB


#define EXFLASH_CS_PORT         SPI_CS_GPIO_Port 
#define EXFLASH_CS_PIN          SPI_CS_Pin
#define EXFLASH_SPI_H           hspi1

#define M25P16_WRITE_ENABLE     0x06
#define M25P16_WRTTE_DISABLE    0x04
#define M25P16_READ_ID          0x9f  // 0x9e
#define M25P16_READ_STA_REG     0x05
#define M25P16_WRITE_STA_REG    0x01
#define M25P16_READ_BYTES       0x03
#define M25P16_READ_BYTES_HS    0x0b
#define M25P16_PAGE_PROGRAM     0x02
#define M25P16_SECTOR_ERASE     0xd8
#define M25P16_BULK_ERASE       0xc7
#define M25P16_DEEP_PWRDW       0xb9
#define M25P16_DEEP_WKUP        0xab

#define M25P16_FREE_FLAG        0x01 



void m25p16_write_enable(void);
void m25p16_write_disenable(void);
uint8_t m25p16_read_sta_reg(void);
void m25p16_waitfor_free(void);

void m25p16_read_data(uint32_t addr, uint8_t* data, uint32_t size);
void m25p16_write_page(uint32_t addr, uint8_t* data, uint32_t size);
void m25p16_sector_erase(uint32_t addr, uint16_t cnt);
void m25p16_chip_erase(uint32_t addr);


#endif


