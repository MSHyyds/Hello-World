#ifndef __TM1628A_H
#define __TM1628A_H

#include "stm32f1xx_hal.h"
#include "main.h"

/**
 * @brief command instruction
 * 
 * first byte:
 *              B7   B6         CMD
 *              0    0          show mode command setting
 *              0    1          data command setting
 *              1    0          show control command setting
 *              1    1          address command setting
 * 
 */

// show mode command setting

#define BIT4_13P    0x00   // 4位13段
#define BIT5_12P    0x01   // 5位12段
#define BIT6_11P    0x02
#define BIT7_10P    0x03



// data command setting

#define WRETE_DATA  0x40    // 写数据 
#define READ_DATA   0x42    // 读数据
#define ADDR_ACC    0x40    // 地址自增模式
#define ADDR_FIXED  0x44    // 固定地址模式
#define NORMAL_MODE 0x40    // 
#define TEST_MODE   0x48

#define PULSE_WIDTH_1_16    0x80
#define PULSE_WIDTH_2_16    0x81
#define PULSE_WIDTH_4_16    0x82
#define PULSE_WIDTH_10_16   0x83
#define PULSE_WIDTH_11_16   0x84
#define PULSE_WIDTH_12_16   0x85
#define PULSE_WIDTH_13_16   0x86
#define PULSE_WIDTH_14_16   0x87
#define SHOW_ON             0x88
#define SHOW_OFF            0x80

// data command seting 
#define TM1628A_DIO_HIGH()   HAL_GPIO_WritePin(TM_DIO_GPIO_Port, TM_DIO_Pin, GPIO_PIN_SET)
#define TM1628A_DIO_LOW()    HAL_GPIO_WritePin(TM_DIO_GPIO_Port, TM_DIO_Pin, GPIO_PIN_RESET)
#define TM1628A_CLK_HIGH()   HAL_GPIO_WritePin(TM_CLK_GPIO_Port, TM_CLK_Pin, GPIO_PIN_SET)
#define TM1628A_CLK_LOW()    HAL_GPIO_WritePin(TM_CLK_GPIO_Port, TM_CLK_Pin, GPIO_PIN_RESET)
#define TM1628A_STB_HIGH()   HAL_GPIO_WritePin(TM_STB_GPIO_Port, TM_STB_Pin, GPIO_PIN_SET)
#define TM1628A_STB_LOW()    HAL_GPIO_WritePin(TM_STB_GPIO_Port, TM_STB_Pin, GPIO_PIN_RESET)

// show address
#define CXH(ch)     (0xC0 +ch)


// show digit
#define ZERO    0x3F    // 0
#define ONE     0x06    // 1
#define TWO     0x5B    // 2
#define THREE   0x4F    // 3
#define FOUR    0x66    // 4
#define FIVE    0x6D    // 5
#define SIX     0x7D    // 6
#define SEVEN   0x07    // 7
#define EIGHT   0x7F    // 8
#define NINE    0x6F    // 9 
#define POINT   0x80    // .
#define CLK_PT  0x01    // :
#define ALL_PT  0xFF    // full
#define NONE    0x00    // none
#define LOG_E   0x79    // E
#define LOG_BE  0x48    // =

void tm1628a_init(uint8_t mode);
void tm1628a_clear_op(void);
void tm1628a_show_count(uint16_t num, uint8_t dp_pos, uint16_t delay_ms, uint8_t mode);
void tm1628a_show_code(uint8_t code, uint16_t delay_ms, uint8_t mode);



#endif


