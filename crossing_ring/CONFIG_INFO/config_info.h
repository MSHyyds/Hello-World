#ifndef __CONFIG_INFO_H
#define __CONFIG_INFO_H

// #include "stm32f1xx_hal.h"
#include "ws2812b.h"


#define DEFAULT_DEVICE_H    40.0f
#define DEFAULT_UAV_H       20.0f
#define DEFAULT_FULL_H      3.95f

#define TIME_COUNT_TIM  htim3

enum ID_RECOG_SW{
  id_recog_closed = 0,
  id_recog_open  
};

#pragma pack(4)
typedef struct{
    uint8_t recog_flag;         // 开启id识别过滤: 0，关闭; 1,开启
    uint8_t connc_flag;         // wifi状态：sta or ap; tcp or udp
    // uint8_t dev_adrr;           // 设备地址
    float device_height;        // 设备高度
    float uav_height;           // 无人机高度
    uint16_t max_score;         // 最高分数
    uint16_t curr_score;        // 当前分数
    COLOR_T wait_cross;         // 等待色
    COLOR_T cross_color;        // 穿过色
    COLOR_T sleep_color;        // 休眠色
}CONFIG_INFO_T;
#pragma pack()

extern CONFIG_INFO_T config_info_1;
extern uint16_t timeout_cnt;

void config_info_init(CONFIG_INFO_T* info);
void print_config_info(CONFIG_INFO_T info);
void get_current_score(CONFIG_INFO_T info, uint8_t* data);
void status_time_init(void);
void status_time_over_isr(TIM_HandleTypeDef *htim);
void status_time_ctrl(uint8_t sta);

#endif

