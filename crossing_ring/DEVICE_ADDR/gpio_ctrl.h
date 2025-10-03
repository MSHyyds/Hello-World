#ifndef _GPIO_CTRL_H
#define _GPIO_CTRL_H

#include "main.h"

#define ADC_BATT            hadc1
#define ADC_SAMPLE_CNT      10
#define LOW_BAT_WARNING     3.60

enum BUZZ_MODE{
    BUZZ_ON = 0,
    BUZZ_OFF,
    BUZZ_SINGLE,
    BUZZ_THREE
};


uint16_t get_dip_switch_addr(void);
void set_buzz(uint8_t mode, uint16_t ms);
uint8_t get_device_mode(void);
float adc_get_value(void);

#endif

