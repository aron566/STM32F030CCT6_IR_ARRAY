/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "stm32f0xx_hal.h"

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
#define TX_TEMP_4_Pin GPIO_PIN_0
#define TX_TEMP_4_GPIO_Port GPIOA
#define RX_TEMP_4_Pin GPIO_PIN_1
#define RX_TEMP_4_GPIO_Port GPIOA
#define TX_TEMP_2_Pin GPIO_PIN_2
#define TX_TEMP_2_GPIO_Port GPIOA
#define RX_TEMP_2_Pin GPIO_PIN_3
#define RX_TEMP_2_GPIO_Port GPIOA
#define TX_SCREEN_Pin GPIO_PIN_4
#define TX_SCREEN_GPIO_Port GPIOA
#define RX_SCREEN_Pin GPIO_PIN_5
#define RX_SCREEN_GPIO_Port GPIOA
#define TX_TEMP_3_Pin GPIO_PIN_10
#define TX_TEMP_3_GPIO_Port GPIOB
#define RX_TEMP_3_Pin GPIO_PIN_11
#define RX_TEMP_3_GPIO_Port GPIOB
#define Relay_NC_Pin GPIO_PIN_12
#define Relay_NC_GPIO_Port GPIOB
#define Relay_NO_Pin GPIO_PIN_13
#define Relay_NO_GPIO_Port GPIOB
#define LED_Red_Pin GPIO_PIN_14
#define LED_Red_GPIO_Port GPIOB
#define LED_Green_Pin GPIO_PIN_15
#define LED_Green_GPIO_Port GPIOB
#define TX_TEMP_1_Pin GPIO_PIN_9
#define TX_TEMP_1_GPIO_Port GPIOA
#define RX_TEMP_1_Pin GPIO_PIN_10
#define RX_TEMP_1_GPIO_Port GPIOA
#define TX_PC_CTL_Pin GPIO_PIN_3
#define TX_PC_CTL_GPIO_Port GPIOB
#define RX_PC_CTL_Pin GPIO_PIN_4
#define RX_PC_CTL_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
