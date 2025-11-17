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
#include "stm32h7xx_hal.h"

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
#define MCU_DO0_Pin GPIO_PIN_0
#define MCU_DO0_GPIO_Port GPIOF
#define MCU_DO1_Pin GPIO_PIN_1
#define MCU_DO1_GPIO_Port GPIOF
#define MCU_DO2_Pin GPIO_PIN_2
#define MCU_DO2_GPIO_Port GPIOF
#define MCU_DO3_Pin GPIO_PIN_3
#define MCU_DO3_GPIO_Port GPIOF
#define MCU_DO4_Pin GPIO_PIN_4
#define MCU_DO4_GPIO_Port GPIOF
#define MCU_DO5_Pin GPIO_PIN_5
#define MCU_DO5_GPIO_Port GPIOF
#define MCU_DO6_Pin GPIO_PIN_6
#define MCU_DO6_GPIO_Port GPIOF
#define MCU_DO7_Pin GPIO_PIN_7
#define MCU_DO7_GPIO_Port GPIOF
#define MCU_DO8_Pin GPIO_PIN_8
#define MCU_DO8_GPIO_Port GPIOF
#define MCU_DO9_Pin GPIO_PIN_9
#define MCU_DO9_GPIO_Port GPIOF
#define MCU_DO10_Pin GPIO_PIN_10
#define MCU_DO10_GPIO_Port GPIOF
#define MCU_DO11_Pin GPIO_PIN_0
#define MCU_DO11_GPIO_Port GPIOC
#define MCU_DO12_Pin GPIO_PIN_1
#define MCU_DO12_GPIO_Port GPIOC
#define MCU_DO13_Pin GPIO_PIN_2
#define MCU_DO13_GPIO_Port GPIOC
#define MCU_DO14_Pin GPIO_PIN_3
#define MCU_DO14_GPIO_Port GPIOC
#define MCU_DO15_Pin GPIO_PIN_0
#define MCU_DO15_GPIO_Port GPIOA
#define MCU_DO16_Pin GPIO_PIN_1
#define MCU_DO16_GPIO_Port GPIOA
#define MCU_DO17_Pin GPIO_PIN_2
#define MCU_DO17_GPIO_Port GPIOA
#define MCU_DO18_Pin GPIO_PIN_3
#define MCU_DO18_GPIO_Port GPIOA
#define MCU_DO19_Pin GPIO_PIN_4
#define MCU_DO19_GPIO_Port GPIOA
#define MCU_DO20_Pin GPIO_PIN_5
#define MCU_DO20_GPIO_Port GPIOA
#define MCU_DO21_Pin GPIO_PIN_6
#define MCU_DO21_GPIO_Port GPIOA
#define MCU_DO22_Pin GPIO_PIN_7
#define MCU_DO22_GPIO_Port GPIOA
#define MCU_DO23_Pin GPIO_PIN_4
#define MCU_DO23_GPIO_Port GPIOC
#define MCU_DO24_Pin GPIO_PIN_5
#define MCU_DO24_GPIO_Port GPIOC
#define MCU_DO25_Pin GPIO_PIN_0
#define MCU_DO25_GPIO_Port GPIOB
#define MCU_DO26_Pin GPIO_PIN_1
#define MCU_DO26_GPIO_Port GPIOB
#define MCU_DO27_Pin GPIO_PIN_2
#define MCU_DO27_GPIO_Port GPIOB
#define MCU_DO28_Pin GPIO_PIN_11
#define MCU_DO28_GPIO_Port GPIOF
#define MCU_DO29_Pin GPIO_PIN_12
#define MCU_DO29_GPIO_Port GPIOF
#define MCU_DO30_Pin GPIO_PIN_15
#define MCU_DO30_GPIO_Port GPIOE
#define MCU_DO31_Pin GPIO_PIN_10
#define MCU_DO31_GPIO_Port GPIOB
#define MCU_DO32_Pin GPIO_PIN_11
#define MCU_DO32_GPIO_Port GPIOB
#define MCU_DO33_Pin GPIO_PIN_12
#define MCU_DO33_GPIO_Port GPIOB
#define MCU_DO34_Pin GPIO_PIN_13
#define MCU_DO34_GPIO_Port GPIOB
#define MCU_DO35_Pin GPIO_PIN_8
#define MCU_DO35_GPIO_Port GPIOD
#define MCU_DO36_Pin GPIO_PIN_9
#define MCU_DO36_GPIO_Port GPIOD
#define MCU_DO37_Pin GPIO_PIN_10
#define MCU_DO37_GPIO_Port GPIOD
#define MCU_DO38_Pin GPIO_PIN_11
#define MCU_DO38_GPIO_Port GPIOD
#define MCU_DO39_Pin GPIO_PIN_12
#define MCU_DO39_GPIO_Port GPIOD
#define MCU_DO40_Pin GPIO_PIN_13
#define MCU_DO40_GPIO_Port GPIOD
#define MCU_DO41_Pin GPIO_PIN_14
#define MCU_DO41_GPIO_Port GPIOD
#define MCU_DO42_Pin GPIO_PIN_15
#define MCU_DO42_GPIO_Port GPIOD
#define MCU_DO43_Pin GPIO_PIN_2
#define MCU_DO43_GPIO_Port GPIOG
#define MCU_DO44_Pin GPIO_PIN_3
#define MCU_DO44_GPIO_Port GPIOG
#define MCU_DO45_Pin GPIO_PIN_4
#define MCU_DO45_GPIO_Port GPIOG
#define MCU_DO46_Pin GPIO_PIN_5
#define MCU_DO46_GPIO_Port GPIOG
#define MCU_DO47_Pin GPIO_PIN_6
#define MCU_DO47_GPIO_Port GPIOG
#define MCU_DO48_Pin GPIO_PIN_7
#define MCU_DO48_GPIO_Port GPIOG
#define MCU_DO49_Pin GPIO_PIN_8
#define MCU_DO49_GPIO_Port GPIOG
#define MCU_DO50_Pin GPIO_PIN_6
#define MCU_DO50_GPIO_Port GPIOC
#define MCU_DO51_Pin GPIO_PIN_7
#define MCU_DO51_GPIO_Port GPIOC
#define MCU_DO52_Pin GPIO_PIN_8
#define MCU_DO52_GPIO_Port GPIOC
#define MCU_DO53_Pin GPIO_PIN_9
#define MCU_DO53_GPIO_Port GPIOC
#define MCU_DO54_Pin GPIO_PIN_8
#define MCU_DO54_GPIO_Port GPIOA
#define MCU_DO55_Pin GPIO_PIN_9
#define MCU_DO55_GPIO_Port GPIOA
#define LED_RUN_OUT_Pin GPIO_PIN_1
#define LED_RUN_OUT_GPIO_Port GPIOD
#define LED_ERR_OUT_Pin GPIO_PIN_2
#define LED_ERR_OUT_GPIO_Port GPIOD
#define RS485_COM_OUT_Pin GPIO_PIN_4
#define RS485_COM_OUT_GPIO_Port GPIOD
#define RS485_TX_OUT_Pin GPIO_PIN_5
#define RS485_TX_OUT_GPIO_Port GPIOD
#define RS485_RX_OUT_Pin GPIO_PIN_6
#define RS485_RX_OUT_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
