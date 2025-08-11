/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADDR4_Pin GPIO_PIN_13
#define ADDR4_GPIO_Port GPIOC
#define SEL_MODE_Pin GPIO_PIN_14
#define SEL_MODE_GPIO_Port GPIOC
#define TRIG_Pin GPIO_PIN_15
#define TRIG_GPIO_Port GPIOC
#define ECHO1_Pin GPIO_PIN_0
#define ECHO1_GPIO_Port GPIOA
#define ECHO2_Pin GPIO_PIN_1
#define ECHO2_GPIO_Port GPIOA
#define SPI_CS_Pin GPIO_PIN_4
#define SPI_CS_GPIO_Port GPIOA
#define BUZZ_Pin GPIO_PIN_1
#define BUZZ_GPIO_Port GPIOB
#define E2_SCL_Pin GPIO_PIN_10
#define E2_SCL_GPIO_Port GPIOB
#define E2_SDA_Pin GPIO_PIN_11
#define E2_SDA_GPIO_Port GPIOB
#define ADDR0_Pin GPIO_PIN_12
#define ADDR0_GPIO_Port GPIOB
#define ADDR1_Pin GPIO_PIN_13
#define ADDR1_GPIO_Port GPIOB
#define ADDR2_Pin GPIO_PIN_14
#define ADDR2_GPIO_Port GPIOB
#define ADDR3_Pin GPIO_PIN_15
#define ADDR3_GPIO_Port GPIOB
#define WS1_Pin GPIO_PIN_8
#define WS1_GPIO_Port GPIOA
#define WS2_Pin GPIO_PIN_11
#define WS2_GPIO_Port GPIOA
#define WIFI_ES0_Pin GPIO_PIN_12
#define WIFI_ES0_GPIO_Port GPIOA
#define WIFI_RST_Pin GPIO_PIN_15
#define WIFI_RST_GPIO_Port GPIOA
#define TM_DIO_Pin GPIO_PIN_3
#define TM_DIO_GPIO_Port GPIOB
#define TM_CLK_Pin GPIO_PIN_4
#define TM_CLK_GPIO_Port GPIOB
#define TM_STB_Pin GPIO_PIN_5
#define TM_STB_GPIO_Port GPIOB
#define IR_IC1_Pin GPIO_PIN_6
#define IR_IC1_GPIO_Port GPIOB
#define IR_IC2_Pin GPIO_PIN_7
#define IR_IC2_GPIO_Port GPIOB
#define IR_IC3_Pin GPIO_PIN_8
#define IR_IC3_GPIO_Port GPIOB
#define IR_IC4_Pin GPIO_PIN_9
#define IR_IC4_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
