#include "ex_flash.h"


static void spi_write_1byte(uint8_t dat)
{
    HAL_SPI_Transmit(&EXFLASH_SPI_H, &dat, 1, 100);
}

static uint8_t spi_read_1byte(void)
{
    uint8_t re;
    HAL_SPI_Receive(&EXFLASH_SPI_H, &re, 1, 100);
    return re;
}

static void spi_cs_enable(void)
{
    HAL_GPIO_WritePin(EXFLASH_CS_PORT, EXFLASH_CS_PIN, GPIO_PIN_RESET);
}

static void spi_cs_disable(void)
{
    HAL_GPIO_WritePin(EXFLASH_CS_PORT, EXFLASH_CS_PIN, GPIO_PIN_SET);
}

void m25p16_write_enable(void)
{
    spi_cs_enable();
    spi_write_1byte(M25P16_WRITE_ENABLE);
    spi_cs_disable();
}

void m25p16_write_disenable(void) 
{
    spi_cs_enable();
    spi_write_1byte(M25P16_WRTTE_DISABLE);
    spi_cs_disable();
}

// 读取状态寄存器
uint8_t m25p16_read_sta_reg(void) 
{
    uint8_t status;
    spi_cs_enable();
    spi_write_1byte(M25P16_READ_STA_REG);
    status = spi_read_1byte();
    spi_cs_disable();
    return status;
}

// 等待Flash空闲
void m25p16_waitfor_free(void) {
    while ((m25p16_read_sta_reg() & M25P16_FREE_FLAG) == SET);
}

// 读取数据
void m25p16_read_data(uint32_t addr, uint8_t* data, uint32_t size) 
{
    uint16_t i;
    
    spi_cs_enable();
    spi_write_1byte(M25P16_READ_BYTES);
    spi_write_1byte((addr & 0xff0000) >>16);
    spi_write_1byte((addr & 0x00ff00) >> 8);
    spi_write_1byte(addr & 0xff);

    for(i=0;i<size;i++)
    {
        data[i] = spi_read_1byte();
    }
    
    spi_cs_disable();
}

// 写入页面
void m25p16_write_page(uint32_t addr, uint8_t* data, uint32_t size) 
{
    uint16_t i;

    m25p16_write_enable();

    spi_cs_enable();
    spi_write_1byte(M25P16_PAGE_PROGRAM);
    spi_write_1byte((addr & 0xff0000) >>16);
    spi_write_1byte((addr & 0x00ff00) >> 8);
    spi_write_1byte(addr & 0xff);

    for(i=0;i<size;i++)
    {
        spi_write_1byte(data[i]);
    }
    spi_cs_disable();

    m25p16_write_disenable();
    m25p16_waitfor_free();
}

// 擦除扇区
void m25p16_sector_erase(uint32_t addr, uint16_t cnt) 
{
    uint16_t i;
    uint32_t tmp = addr;

    for(i=0;i<cnt;i++)
    {
        tmp = addr + 0x10000 * i;
        m25p16_write_enable();

        spi_cs_enable();
        spi_write_1byte(M25P16_SECTOR_ERASE);
        spi_write_1byte((addr & 0xff0000) >>16);
        spi_write_1byte((addr & 0x00ff00) >> 8);
        spi_write_1byte(addr & 0xff);
        spi_cs_disable();
    
        m25p16_write_disenable();
        m25p16_waitfor_free();
       
    }
    
}

/**
 * @brief 擦除整片
 * 
 * @param addr 
 */
void m25p16_chip_erase(uint32_t addr) 
{
    uint16_t i;

    m25p16_write_enable();

    spi_cs_enable();
    spi_write_1byte(M25P16_BULK_ERASE);
    spi_write_1byte((addr & 0xff0000) >>16);
    spi_write_1byte((addr & 0x00ff00) >> 8);
    spi_write_1byte(addr & 0xff);
    spi_cs_disable();
    
    m25p16_write_disenable();

    m25p16_waitfor_free();
}


