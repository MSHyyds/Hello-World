#ifndef __IR_H
#define __IR_H

#include "main.h"
#include "stm32f1xx_hal.h"
#include "tim.h"

#define IR_HEAD_NUM     4       // 红外接收头数量

#define IR_TIM      htim4
// #define IR_TIM htim3
// #define IR_TIM_CHANNE1 TIM_CHANNEL_1
// #define IR_TIM_CHANNE2 TIM_CHANNEL_2
// #define IR_TIM_CHANNE3 TIM_CHANNEL_3
// #define IR_TIM_CHANNE4 TIM_CHANNEL_4



#define IR_US_LOGIC_0 560           // 逻辑0的脉宽值
#define IR_US_LOGIC_1 1640          // 逻辑1的脉宽值
#define IR_US_BOOT_0 9000           // 引导码之9ms脉宽值
#define IR_US_BOOT_1 4500           // 引导码之4.5ms脉宽值
#define IR_us_REPEAT_START 30650     // 重复码9ms
#define IR_us_REPEAT_END 2250        // 重复码560us
#define IR_US_DEVIATION 100	//允许的误差值


typedef struct
{
    uint8_t     rcv_flag;       // 收到标志
    uint8_t     level_flag;     // 电平捕获标志，1：上升沿捕获，0：下降沿捕获
    uint16_t    over_cnt;       // 溢出次数记录
    uint16_t    pluse_width;    // 脉宽
    uint32_t    up_val;         // 上升沿时刻捕获值
    uint32_t    down_val;       // 下降沿时刻捕获值
    uint16_t    bit_buff[128];  // 捕获值缓存区
    uint16_t    bit_cnt;        // 捕获值计数
    uint8_t     change_data[4]; // 经计算后的码值，依次为：用户码、用户反码、数据码、数据反码
    uint8_t     repet_flag;
    uint8_t     save_data[4];
    uint32_t    IR_TIM_CHANNEL;
}IR_INFO_T;

// extern IR_INFO_T ir_info; 



void ir_init(void);
void ir_tim_over_isr(TIM_HandleTypeDef *tim);
void ir_tim_ic_isr(TIM_HandleTypeDef *tim);
uint8_t ir_read(uint8_t (*arr)[4], uint8_t cnt);

#endif

