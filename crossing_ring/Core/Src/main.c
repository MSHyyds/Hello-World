/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "debug_log.h"
#include "gpio_ctrl.h"
#include "wifi_b35.h"
#include "tm1628a.h"
#include "ex_flash.h"
#include "string.h"
#include "ir.h"
#include "hc_sr04.h"
#include "ws2812b.h"
#include "config_info.h"
#include "longing.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

enum system_state{
  sys_wait_check,
  sys_checking,
  sys_reco_succ,
  sys_reco_fail,
  sys_wait_rst

};

uint8_t test_buf[200];

uint8_t current_state = sys_wait_check;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  

  longing_protocol_t send_info;

  float distance[HC_HEAD_NUM];
  

  uint8_t ir_id[IR_HEAD_NUM][4];

  uint8_t flash_test_data[] ="hello world Meng.........";
  uint8_t flash_recv_data[30];

  uint16_t crossing_cnt = 0;
  uint16_t local_addr,send_len,cnt;

  uint8_t re_data[10];

  float batt_val;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  ir_init();
  hc_init();
  

  ringbuffer_init(&ringbuffer, rx_buff, 200);
  HAL_UART_Receive_IT(&huart2, &rx_data, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  emlog(LOG_INFO, "enter main\r\n");

  // get local addr
  local_addr = get_dip_switch_addr();

  // get device mode
  get_device_mode();

  // digital tube self-check
  // tm1628a_init(0);
  

  /* extern flash test */
  m25p16_sector_erase(0,1);
  m25p16_read_data(0, flash_recv_data, sizeof(flash_test_data));
  emlog(LOG_DEBUG, "read flash datas: %s", flash_recv_data);
  m25p16_write_page(0, flash_test_data, sizeof(flash_test_data));
  m25p16_read_data(0, flash_recv_data, sizeof(flash_test_data));
  emlog(LOG_DEBUG, "after write, read flash datas: %s", flash_recv_data);
  


 
  config_device_mode(DHCP_MODE, TCP_MODE, CLIENT_MODE);
  mylog("config info:\r\n");
  query_config_info();
  
  mylog("into main.\r\n");
  

  /* 灯带测试 */
  // breatheLED(GREEN, 0, LED_TOTAL_CNT, 100);
  // ws_light_inc(NULL, 0, LED_TOTAL_CNT, GREEN, 1);
  // ws_light_shine(NULL, 50, CYAN);
  ws_light_shine(NULL, LED_TOTAL_CNT, BLACK);

  while (1)
  {
    // set_buzz(BUZZ_SINGLE, 2000);
    // tm1628a_show_count(num,0,100,0);
    // num++;
    // tm1628a_show_code(1, 100, 0);
    

    /* 红外遥控接收测试 */
    // ir_read(ir_id, IR_HEAD_NUM);
    
    //   for(i=0;i<4;i++)
    //   {
    //     emlog(LOG_DEBUG, "CH: %d, %02x-%02x-%02x-%02x\r\n", i, ir_id[i][0], ir_id[i][1], ir_id[i][2], ir_id[i][3]);
    //     memset(ir_id[i], 0, 4);
    //   }
    
    
    
    /* 超声波测距测�?????? */

    hc_read(distance, HC_HEAD_NUM);
    mylog("left distance: %.2f cm --- %.2f m, right distance: %.2f cm --- %.2f m\r\n", distance[0],(distance[0] / 100.0), distance[1], (distance[1] / 100.0));
    // memset(distance, 0, 2);

    /* 灯带测试 */
    
    // breatheLED(YELLOW, 0, LED_TOTAL_CNT, 50);


    /* 电池电压测试 */
    // batt_val = adc_get_value();
    // emlog(LOG_DEBUG, "batt val: %.2fV \r\n", batt_val);

    /* 上传数据到上行端点 */
    // distance[0] = 8.16;
    // distance[1] = 9.86;

    send_info.type = FRAME_SEND;
    send_info.target_addr = 0xffee;
    send_info.local_addr = local_addr;
    send_info.cmd_seq = 1;
    send_info.cmd = ORHC;
    
    float_to_uint8_t(distance, 2, re_data);
    emlog(LOG_DEBUG, "0x%02x-0x%02x-0x%02x-0x%02x \r\n",re_data[0], re_data[1], re_data[2], re_data[3]);
    emlog(LOG_DEBUG, "0x%02x-0x%02x-0x%02x-0x%02x \r\n",re_data[4], re_data[5], re_data[6], re_data[7]);

    send_info.data_len = filler_frame_data(re_data, 2, 4, send_info.data);

    
   

    /* 串口2接收测试 */
    if(rx_complt)
    {
  
    cnt = get_used_len(&ringbuffer);
    get_free_len(&ringbuffer);
    read_bytes(&ringbuffer, test_buf, cnt);
    emlog(LOG_DEBUG, "cnt= %d.\r\n", cnt);
    
    if(cnt!=0)
    {
      emlog(LOG_DEBUG, "recv: %s", test_buf);
      
     
      memset(test_buf, 0, sizeof(test_buf));

    }
    rx_complt = 0;
   
  }
  send_len = pack_frame(&send_info, test_buf);
  send_to_wifi(test_buf, send_len);
  memset(distance, 0, 2);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
   

  //   switch(current_state)
  //   {
  //     case sys_wait_check:    // 检测
        
  //       tm1628a_show_count(crossing_cnt,0,100,1);
  //       ws_light_shine(NULL, LED_TOTAL_CNT, RED);
       
  //       hc_read(distance, HC_HEAD_NUM);
  //       // mylog("left distance: %.2f cm --- %.2f m, right distance: %.2f cm --- %.2f m\r\n", distance[0],(distance[0] / 100.0), distance[1], (distance[1] / 100.0));
        
        

  //       if(distance[0] <=3.0f || distance[1] <= 3.0f)
  //       {
  //         current_state = sys_checking;
  //       }

  //       memset(distance, 0, HC_HEAD_NUM);
  //       memset(distance2, 0, HC_HEAD_NUM);
  //       break;
      
  //     case sys_checking:
  //       ir_read(ir_id, IR_HEAD_NUM);
  //       // if(ir_id[1][0]==0x00 && ir_id[1][1]==0xef && ir_id[1][2]==0x02 && ir_id[1][3]==0xfc)
  //       if(1)
  //       {
  //         current_state = sys_reco_succ;
  //         for(i=0;i<4;i++)
  //         {
  //           emlog(LOG_DEBUG, "CH: %d, %02x-%02x-%02x-%02x\r\n", i, ir_id[i][0], ir_id[i][1], ir_id[i][2], ir_id[i][3]);
  //         }
          
  //       }
  //       else
  //       {
  //         current_state = sys_reco_fail;
  //       }
  //       break;

  //     case sys_reco_succ: 
  //       set_buzz(BUZZ_SINGLE, 1);
  //       ws_light_shine(NULL, LED_TOTAL_CNT, GREEN);
  //       tm1628a_show_count(crossing_cnt++,0,100,0);
        
  //       current_state = sys_wait_rst;
  //       break;

  //     case sys_reco_fail:
  //       current_state = sys_checking;
  //       ws_light_shine(NULL, LED_TOTAL_CNT, BLUE);
  //       break;

  //     case sys_wait_rst:
        
  //       current_state = sys_wait_check;
  //       // ws_light_shine(NULL, LED_TOTAL_CNT, RED);
  //       batt_val = adc_get_value();
  //       if(batt_val <= LOW_BAT_WARNING);    // 低压报警
  //       {
  //         // ws_light_shine(NULL, LED_TOTAL_CNT, BLACK);
  //         // set_buzz(BUZZ_THREE, 1000);
  //         // tm1628a_show_code(1, 100, 0);
  //         // ws_light_shine(NULL, 3, RED);
  //         emlog(LOG_DEBUG, "low battarry\r\n");
  //       }

  //   }


  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  hc_tim_ic_isr(&HC_TIM);
  ir_tim_ic_isr(&IR_TIM);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
  hc_tim_over_isr(&HC_TIM);
  ir_tim_over_isr(&IR_TIM);
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
