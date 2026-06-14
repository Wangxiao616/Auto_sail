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
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define SG90_Pin GPIO_PIN_1
#define SG90_GPIO_Port GPIOA
#define IRM_1_Pin GPIO_PIN_4
#define IRM_1_GPIO_Port GPIOA
#define IRM_1_EXTI_IRQn EXTI4_IRQn
#define IRM_2_Pin GPIO_PIN_5
#define IRM_2_GPIO_Port GPIOA
#define IRM_2_EXTI_IRQn EXTI9_5_IRQn
#define IRM_3_Pin GPIO_PIN_6
#define IRM_3_GPIO_Port GPIOA
#define IRM_3_EXTI_IRQn EXTI9_5_IRQn
#define IRM_4_Pin GPIO_PIN_7
#define IRM_4_GPIO_Port GPIOA
#define IRM_4_EXTI_IRQn EXTI9_5_IRQn
#define IRM_5_Pin GPIO_PIN_8
#define IRM_5_GPIO_Port GPIOA
#define IRM_5_EXTI_IRQn EXTI9_5_IRQn
#define IRM_6_Pin GPIO_PIN_9
#define IRM_6_GPIO_Port GPIOA
#define IRM_6_EXTI_IRQn EXTI9_5_IRQn
#define IRM_7_Pin GPIO_PIN_10
#define IRM_7_GPIO_Port GPIOA
#define IRM_7_EXTI_IRQn EXTI15_10_IRQn
#define IRM_8_Pin GPIO_PIN_11
#define IRM_8_GPIO_Port GPIOA
#define IRM_8_EXTI_IRQn EXTI15_10_IRQn
#define IRM_9_Pin GPIO_PIN_12
#define IRM_9_GPIO_Port GPIOA
#define IRM_9_EXTI_IRQn EXTI15_10_IRQn
#define DRV8833_IN1_Pin GPIO_PIN_6
#define DRV8833_IN1_GPIO_Port GPIOB
#define DRV8833_IN2_Pin GPIO_PIN_7
#define DRV8833_IN2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
