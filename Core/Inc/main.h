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
#define POWER_BUTTON_PIN_Pin GPIO_PIN_1
#define POWER_BUTTON_PIN_GPIO_Port GPIOA
#define DIODE_POWER_Pin GPIO_PIN_1
#define DIODE_POWER_GPIO_Port GPIOB
#define Y4_Pin GPIO_PIN_10
#define Y4_GPIO_Port GPIOB
#define GAU_Pin GPIO_PIN_11
#define GAU_GPIO_Port GPIOB
#define Y0_Pin GPIO_PIN_12
#define Y0_GPIO_Port GPIOB
#define Y1_Pin GPIO_PIN_13
#define Y1_GPIO_Port GPIOB
#define Y2_Pin GPIO_PIN_14
#define Y2_GPIO_Port GPIOB
#define Y3_Pin GPIO_PIN_15
#define Y3_GPIO_Port GPIOB
#define GAL_Pin GPIO_PIN_8
#define GAL_GPIO_Port GPIOA
#define Y5_Pin GPIO_PIN_15
#define Y5_GPIO_Port GPIOA
#define Y6_Pin GPIO_PIN_3
#define Y6_GPIO_Port GPIOB
#define Y7_Pin GPIO_PIN_4
#define Y7_GPIO_Port GPIOB
#define CCLR_Pin GPIO_PIN_8
#define CCLR_GPIO_Port GPIOB
#define RCLK_Pin GPIO_PIN_9
#define RCLK_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
