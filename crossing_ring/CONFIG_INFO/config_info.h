#ifndef __CONFIG_INFO_H
#define __CONFIG_INFO_H

#include "stm32f1xx_hal.h"

#define HC_DISTANCE_LIMT

typedef struct{
    uint8_t device_height;      // 通行（待检）设备高度
    uint8_t recog_flag;         // 1， 开启id识别过滤； 0，关闭~
    uint16_t record_high;       // 历史最高纪录
    

}CONFIG_INFO_T;

#endif

