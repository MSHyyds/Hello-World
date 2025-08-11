#include "hc_sr04.h"
#include "string.h"
#include "main.h"

 HC_INFO_T Hcsr04Info[HC_HEAD_NUM];

 extern int mylog(char *format, ...);


/**
 * @brief delay ns
 * 
 * @param nTimer ns
 */
static void delay_us(uint32_t nTimer)
{
	uint32_t i=0;
	for(i=0;i<nTimer;i++){
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
	}
}


void hc_init()
{
  HAL_TIM_Base_Start_IT(&HC_TIM);               // 开启定时器
	HAL_TIM_IC_Start_IT(&HC_TIM,TIM_CHANNEL_1);   // 开启通道1捕获
	HAL_TIM_IC_Start_IT(&HC_TIM,TIM_CHANNEL_2);   // 开启通道2捕获
}
/**
 * @brief hc start, need to repeat
 * 
 */
void hc_start(void)
{
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    delay_us(15);
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
}

/**
 * @brief timer overtime interrupt service function
 * 
 * @param htim 
 */
void hc_tim_over_isr(TIM_HandleTypeDef *htim)
{
  uint8_t i;

  if(htim->Instance == HC_TIM.Instance)
  {
    for(i=0;i<HC_HEAD_NUM;i++)
    {
      Hcsr04Info[i].tim_over_cnt++;
      
    }
  }
}

/**
 * @brief time input commpare interrupt service function
 * 
 * @param htim 
 */
void hc_tim_ic_isr(TIM_HandleTypeDef* htim)
{
  uint8_t index;
  if(htim->Instance == HC_TIM.Instance)
  {
    switch(htim->Channel)
		{
			case HAL_TIM_ACTIVE_CHANNEL_1:
				index = 0;
				Hcsr04Info[index].tim_ic_ch = TIM_CHANNEL_1;
				break;
			
			case HAL_TIM_ACTIVE_CHANNEL_2:
				index = 1;
				Hcsr04Info[index].tim_ic_ch = TIM_CHANNEL_2;
				break;

			case HAL_TIM_ACTIVE_CHANNEL_3:
				index = 2;
				Hcsr04Info[index].tim_ic_ch = TIM_CHANNEL_3;
				break;

			case HAL_TIM_ACTIVE_CHANNEL_4:
				index = 3;
				Hcsr04Info[index].tim_ic_ch = TIM_CHANNEL_4;
				break;
      default:
        return; 
		}
    if(Hcsr04Info[index].edge_state == 0)      //  捕获上升沿
    {

      // 得到上升沿开始时间T1，并更改输入捕获为下降沿
      Hcsr04Info[index].t1 = HAL_TIM_ReadCapturedValue(htim, Hcsr04Info[index].tim_ic_ch);
      __HAL_TIM_SET_CAPTUREPOLARITY(htim, Hcsr04Info[index].tim_ic_ch, TIM_INPUTCHANNELPOLARITY_FALLING);
      Hcsr04Info[index].tim_over_cnt = 0;  //  定时器溢出计数器清零
      Hcsr04Info[index].edge_state = 1;        //  上升沿、下降沿捕获标志位
    }
    else if(Hcsr04Info[index].edge_state == 1) //  捕获下降沿
    {
      // 捕获下降沿时间T2，并计算高电平时间
      Hcsr04Info[index].t2 = HAL_TIM_ReadCapturedValue(htim, Hcsr04Info[index].tim_ic_ch);
      Hcsr04Info[index].t2 += Hcsr04Info[index].tim_over_cnt * (HC_TIM.Init.Period + 1); //  需要考虑定时器溢出中断
      Hcsr04Info[index].high_level_us = Hcsr04Info[index].t2 - Hcsr04Info[index].t1; //  高电平持续时间 = 下降沿时间点 - 上升沿时间点
      // 计算距离
      Hcsr04Info[index].distance = (Hcsr04Info[index].high_level_us * 0.017); // / 1000000.0) * VOS / 2.0 * 100.0;
      // 重新开启上升沿捕获
      Hcsr04Info[index].edge_state = 0;  //  一次采集完毕，清零
      __HAL_TIM_SET_CAPTUREPOLARITY(htim, Hcsr04Info[index].tim_ic_ch, TIM_INPUTCHANNELPOLARITY_RISING);
      Hcsr04Info[index].complet_flag = 1; // 完成标志置位
    }
  }
}

/**
 * @brief read hc change datas
 * 
 * @return float 
 */
void hc_read(float* arr, uint8_t len)
{
  uint8_t i, cnt;
  float temp[HC_HEAD_NUM][5];
  // uint32_t timeout = 0;

  for(cnt=0;cnt<5;cnt++)
  {
    hc_start();
    HAL_Delay(50);
    for(i=0;i<len;i++)
    {
      if(Hcsr04Info[i].complet_flag == 1)
      {
        // timeout = 0;
        Hcsr04Info[i].complet_flag = 0;  // 完成标志清零

        if(Hcsr04Info[i].distance >= 450)
        {
          temp[i][cnt] = 450.00;
        }
        else
        {
          temp[i][cnt] = Hcsr04Info[i].distance;
        }
        Hcsr04Info[i].distance = 0; // clear
      }
      // timeout++;
      // if(timeout>100000)break;
      
    }
  }
  
  for(i=0;i<len;i++)
  {
    arr[i] = median_filter(temp[i], 5);
  }
 
}


/**
 * @brief 从一个5个元素的数组中获得中位数，进行中值滤波
 * 
 * @param buf     源数据
 * @param size    大小为5
 * @return float  返回类型
 */
float median_filter(float* buf, uint8_t size)
{
    float temp[5];
    memcpy(temp, buf, size * sizeof(float));

    // 简单冒泡排序
    for(uint8_t i = 0; i < size - 1; i++)
    {
        for(uint8_t j = 0; j < size - i - 1; j++)
        {
            if(temp[j] > temp[j+1])
            {
                float t = temp[j];
                temp[j] = temp[j+1];
                temp[j+1] = t;
            }
        }
    }

    return temp[size / 2]; // 返回中位数
}

