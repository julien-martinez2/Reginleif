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
#include "stm32g0xx_hal.h"

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
#define E_2_B_Pin GPIO_PIN_14
#define E_2_B_GPIO_Port GPIOC
#define E_2_B_EXTI_IRQn EXTI4_15_IRQn
#define SPI1_CS_2_Pin GPIO_PIN_15
#define SPI1_CS_2_GPIO_Port GPIOC
#define SPI1_CS_3_Pin GPIO_PIN_2
#define SPI1_CS_3_GPIO_Port GPIOF
#define E_3_B_Pin GPIO_PIN_0
#define E_3_B_GPIO_Port GPIOA
#define E_3_B_EXTI_IRQn EXTI0_1_IRQn
#define NFAULT_Pin GPIO_PIN_2
#define NFAULT_GPIO_Port GPIOA
#define NFAULT_EXTI_IRQn EXTI2_3_IRQn
#define E_4_A_Pin GPIO_PIN_7
#define E_4_A_GPIO_Port GPIOA
#define E_3_A_Pin GPIO_PIN_0
#define E_3_A_GPIO_Port GPIOB
#define E_4_B_Pin GPIO_PIN_1
#define E_4_B_GPIO_Port GPIOB
#define E_4_B_EXTI_IRQn EXTI0_1_IRQn
#define SPI1_CS_4_Pin GPIO_PIN_8
#define SPI1_CS_4_GPIO_Port GPIOA
#define SPI1_CS_1_Pin GPIO_PIN_6
#define SPI1_CS_1_GPIO_Port GPIOC
#define E_1_A_Pin GPIO_PIN_15
#define E_1_A_GPIO_Port GPIOA
#define E_1_B_Pin GPIO_PIN_3
#define E_1_B_GPIO_Port GPIOB
#define E_1_B_EXTI_IRQn EXTI2_3_IRQn
#define E_2_A_Pin GPIO_PIN_4
#define E_2_A_GPIO_Port GPIOB
#define SPI2_CS_Pin GPIO_PIN_5
#define SPI2_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
