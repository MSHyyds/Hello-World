#ifndef __HC_SR04_H
#define __HC_SR04_H

#include "tim.h"
#include "stm32f1xx_hal.h"

/* 0  °C -> 330.45 m/s
 * 20 °C -> 342.62 m/s
 * 40 °C -> 354.85 m/s
*/
#define VOS 340.0

#define HC_HEAD_NUM     2
#define HC_TIM          htim2
// #define HC_TIM htim3
// #define HC_TIM_CHANNE1 TIM_CHANNEL_1 
// #define HC_TIM_CHANNE2 TIM_CHANNEL_2
// #define HC_TIM_CHANNE3 TIM_CHANNEL_3
// #define HC_TIM_CHANNE4 TIM_CHANNEL_4

typedef struct
{
    uint8_t  complet_flag;              // 完成标志
    uint8_t  edge_state;                // 上升沿/下降沿状态标志
    uint16_t tim_over_cnt;              // 定时器溢出次数
    uint32_t t1;                        // 上升沿时间点
    uint32_t t2;                        // 下降沿时间点
    uint32_t high_level_us;             // 高电平持续时间
    float distance;                     // 计算出的距离（cm)
    uint32_t tim_ic_ch;                 // 当前通道
}HC_INFO_T;



// extern HC_INFO_T Hcsr04Info;

void hc_start(void);
void hc_tim_over_isr(TIM_HandleTypeDef *htim);
void hc_tim_ic_isr(TIM_HandleTypeDef* htim);
void hc_read(float* arr, uint8_t len);
void hc_init(void);
float median_filter(float* buf, uint8_t size);

#endif


