#ifndef __WS2812B_H
#define __WS2812B_H

#include "stm32f1xx_hal.h"

#define WS_TIM         htim1
#define WS_TIM_CH1     TIM_CHANNEL_1
#define WS_TIM_CH2     TIM_CHANNEL_4

#define LED_TOTAL_CNT     100
#define CODE_1      (64)
#define CODE_0      (25)

// #define CODE_1      (58)
// #define CODE_0      (25)


#define BUFF_LEN    ((LED_TOTAL_CNT*24)+24)

#define CHANGE_VAL 0.01f


typedef struct{
    uint8_t G;
    uint8_t R;
    uint8_t B;
}COLOR_T;

typedef struct{
    float h;
    float s;
    float v;
}HSV_T;

extern const COLOR_T RED;      // 红色
extern const COLOR_T GREEN;    // 绿色
extern const COLOR_T BLUE;     // 蓝色

extern const COLOR_T YELLOW ;  // 黄色
extern const COLOR_T CYAN;     // 青色
extern const COLOR_T MAGENTA;  // 品红

extern const COLOR_T BLACK;    // 黑色
extern const COLOR_T WHITE;    // 白色

extern const COLOR_T SILVER;   // 银色
extern const COLOR_T SKY;      // 天蓝色
extern const COLOR_T MEDIUM;   // 松石绿色
extern const COLOR_T PURPLE;   // 紫色
extern const COLOR_T JACINTH;  // 橘红色
extern const COLOR_T INDIGO;   // 靛蓝色
extern const COLOR_T GOLD;     // 金色

extern  COLOR_T COLOR_FUL[14];
extern  COLOR_T RED_BLUE[2];
extern uint32_t color_raw[BUFF_LEN];

extern int mylog(char *format, ...);

void ws_reset(void);
void get_sigle_color(uint32_t* buf, COLOR_T color);
void ws_light_shine(uint32_t* buf, uint16_t cnt, COLOR_T color);
void ws_light_place(uint32_t *buf, uint16_t begin, uint16_t cnt, COLOR_T color, uint8_t dir);
void ws_light_inc(uint32_t *buf, uint16_t begin, uint16_t cnt, COLOR_T color, uint8_t dir);
void ws_light_dec(uint32_t *buf, uint16_t begin, uint16_t cnt, COLOR_T color, uint8_t dir);

void breatheLED(COLOR_T color, uint16_t begin, uint16_t cnt, uint16_t ms);
void waterfall_light(COLOR_T color, uint16_t begin, uint16_t end, uint16_t cnt, uint16_t ms, uint8_t dir);
void colorful_flow(COLOR_T color_arr[], uint16_t arr_len, uint16_t begin, uint16_t end, uint16_t cnt, uint16_t ms, uint8_t dir);
void array_shift(COLOR_T arr[], uint16_t len, uint8_t bit, uint8_t dir);
void array_shift_plus(uint32_t* arr, uint16_t len, uint8_t bit, uint8_t dir);
void meteor_trail(COLOR_T color, uint16_t begin, uint16_t end, uint16_t cnt, uint16_t ms, uint8_t dir);
COLOR_T bright_var(COLOR_T color, uint8_t type, float val);

#endif



