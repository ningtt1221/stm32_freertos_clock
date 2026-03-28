/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

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
#define Led2_Pin GPIO_PIN_0
#define Led2_GPIO_Port GPIOC
#define Led1_Pin GPIO_PIN_1
#define Led1_GPIO_Port GPIOC
#define ADC_Pin GPIO_PIN_0
#define ADC_GPIO_Port GPIOA
#define BEEP_Pin GPIO_PIN_1
#define BEEP_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define Led0_Pin GPIO_PIN_0
#define Led0_GPIO_Port GPIOB
#define Key1_Pin GPIO_PIN_10
#define Key1_GPIO_Port GPIOB
#define SPI1_DC_Pin GPIO_PIN_7
#define SPI1_DC_GPIO_Port GPIOC
#define Key0_Pin GPIO_PIN_8
#define Key0_GPIO_Port GPIOA
#define Led3_Pin GPIO_PIN_3
#define Led3_GPIO_Port GPIOB
#define Key2_Pin GPIO_PIN_4
#define Key2_GPIO_Port GPIOB
#define Key3_Pin GPIO_PIN_5
#define Key3_GPIO_Port GPIOB
#define SPI1_RES_Pin GPIO_PIN_6
#define SPI1_RES_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
