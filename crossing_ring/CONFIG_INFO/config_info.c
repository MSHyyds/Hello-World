#include "config_info.h"
#include "debug_log.h"
#include "tim.h"

CONFIG_INFO_T config_info_1;
uint16_t timeout_cnt=0;

void config_info_init(CONFIG_INFO_T* info)
{
    info->device_height = DEFAULT_DEVICE_H;
    info->uav_height    = DEFAULT_UAV_H;
    info->recog_flag    = id_recog_closed;
    info->connc_flag    = 1;
    info->sleep_color   = CYAN;
    info->wait_cross    = RED;
    info->cross_color   = GREEN;
    info->max_score     = 0;
    info->curr_score    = 0;
}

void print_config_info(CONFIG_INFO_T info)
{
    mylog("device height: %.2f cm\r\n", info.device_height);
    mylog("uav height   : %.2f cm\r\n", info.uav_height);
    mylog("id record    : %d points\r\n", info.max_score);
    mylog("id recog     : %s \r\n", info.recog_flag ? "open" : "closed");
    

}




void status_time_init(void)
{
  timeout_cnt = 0;
  HAL_TIM_Base_Start_IT(&TIME_COUNT_TIM);               // 开启定时器
}

/**
 * @brief 状态计时控制
 * 
 * @param sta 1，开启计时； 0，关闭计时
 */
void status_time_ctrl(uint8_t sta)
{
    switch(sta)
    {
        case 0:
            HAL_TIM_Base_Stop_IT(&TIME_COUNT_TIM);
            break;

        case 1:
            HAL_TIM_Base_Start_IT(&TIME_COUNT_TIM);
            break;
    }
}
/**
 * @brief timer overtime interrupt service function
 * 
 * @param htim 
 */
void status_time_over_isr(TIM_HandleTypeDef *htim)
{
  static uint16_t i;

  if(htim->Instance == TIME_COUNT_TIM.Instance)
  {
    i++;

    if(i % 50 ==  0)
    {

    }
    if(i % 100 == 0)
    {

    }
    if(i % 200 == 0)
    {

    }
    if(i % 500 == 0)
    {

    }
    if(i % 1000 == 0)
    {
        i=0;
        timeout_cnt++;
    }
  }
}


uint16_t get_timeout_cnt_val(void)
{
    return timeout_cnt;
}


void get_current_score(CONFIG_INFO_T info, uint8_t* data)
{
    data[0] = info.curr_score & 0xff;
    data[1] = (info.curr_score >> 8) & 0xff;
}


