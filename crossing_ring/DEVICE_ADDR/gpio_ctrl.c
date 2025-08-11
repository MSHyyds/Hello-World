#include "gpio_ctrl.h"
#include "debug_log.h"

#include "gpio.h"
#include "adc.h"

uint16_t get_dip_switch_addr()
{
    uint16_t res=0;
    res |= HAL_GPIO_ReadPin(ADDR4_GPIO_Port, ADDR4_Pin);
    res <<= 1;
    
    res |= HAL_GPIO_ReadPin(ADDR3_GPIO_Port, ADDR3_Pin);
    res <<= 1;

    res |= HAL_GPIO_ReadPin(ADDR2_GPIO_Port, ADDR2_Pin);
    res <<= 1;

    res |= HAL_GPIO_ReadPin(ADDR1_GPIO_Port, ADDR1_Pin);
    res <<= 1;

    res |= HAL_GPIO_ReadPin(ADDR0_GPIO_Port, ADDR0_Pin);

    emlog(LOG_DEBUG, "get dip switch addr: 0x%02x\r\n", res);

    return res;
}

void set_buzz(uint8_t mode, uint16_t ms)
{
    switch(mode)
    {
        case BUZZ_ON:HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_RESET);break;
        case BUZZ_OFF:HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_SET);break;
        case BUZZ_SINGLE:
        HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_RESET);HAL_Delay(ms);
        HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_SET);HAL_Delay(ms);break;

        case BUZZ_THREE:
        HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_RESET);HAL_Delay(ms);
        HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_SET);HAL_Delay(ms);
        HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_RESET);HAL_Delay(ms);
        HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_SET);HAL_Delay(ms);
        HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_RESET);HAL_Delay(ms);
        HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_SET);HAL_Delay(ms);break;
    }
    
}


uint8_t get_device_mode(void)
{
    uint8_t res =0;
    res = HAL_GPIO_ReadPin(SEL_MODE_GPIO_Port, SEL_MODE_Pin);
    emlog(LOG_DEBUG, "device mode: %s\r\n", res ? "single-player mode" : "on-line mode");
    return res;
}



float adc_get_value(void)
{
	uint16_t adcVal;
    float voltage;
    uint32_t sum = 0;

    for (int i = 0; i < ADC_SAMPLE_CNT; i++)
    {
        HAL_ADC_Start(&ADC_BATT);
        HAL_ADC_PollForConversion(&ADC_BATT, 0xFFFF);
        adcVal = HAL_ADC_GetValue(&ADC_BATT);
        HAL_ADC_Stop(&ADC_BATT);
        sum += adcVal;
        HAL_Delay(2);  // 可选，避免连续读取干扰
    }

    float avg_adc = (float)sum / ADC_SAMPLE_CNT;
    voltage = (avg_adc / 4095.0f) * 6.6f;

    return voltage;
}