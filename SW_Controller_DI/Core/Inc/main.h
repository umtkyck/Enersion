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
#define MCU_DI0_Pin GPIO_PIN_12
#define MCU_DI0_GPIO_Port GPIOF
#define MCU_DI1_Pin GPIO_PIN_13
#define MCU_DI1_GPIO_Port GPIOF
#define MCU_DI2_Pin GPIO_PIN_14
#define MCU_DI2_GPIO_Port GPIOF
#define MCU_DI3_Pin GPIO_PIN_15
#define MCU_DI3_GPIO_Port GPIOF
#define MCU_DI4_Pin GPIO_PIN_0
#define MCU_DI4_GPIO_Port GPIOG
#define MCU_DI5_Pin GPIO_PIN_1
#define MCU_DI5_GPIO_Port GPIOG
#define MCU_DI6_Pin GPIO_PIN_7
#define MCU_DI6_GPIO_Port GPIOE
#define MCU_DI7_Pin GPIO_PIN_8
#define MCU_DI7_GPIO_Port GPIOE
#define MCU_DI8_Pin GPIO_PIN_9
#define MCU_DI8_GPIO_Port GPIOE
#define MCU_DI9_Pin GPIO_PIN_10
#define MCU_DI9_GPIO_Port GPIOE
#define MCU_DI10_Pin GPIO_PIN_11
#define MCU_DI10_GPIO_Port GPIOE
#define MCU_DI11_Pin GPIO_PIN_12
#define MCU_DI11_GPIO_Port GPIOE
#define MCU_DI12_Pin GPIO_PIN_13
#define MCU_DI12_GPIO_Port GPIOE
#define MCU_DI13_Pin GPIO_PIN_14
#define MCU_DI13_GPIO_Port GPIOE
#define MCU_DI14_Pin GPIO_PIN_15
#define MCU_DI14_GPIO_Port GPIOE
#define MCU_DI15_Pin GPIO_PIN_10
#define MCU_DI15_GPIO_Port GPIOB
#define MCU_DI16_Pin GPIO_PIN_11
#define MCU_DI16_GPIO_Port GPIOB
#define MCU_DI17_Pin GPIO_PIN_12
#define MCU_DI17_GPIO_Port GPIOB
#define MCU_DI18_Pin GPIO_PIN_13
#define MCU_DI18_GPIO_Port GPIOB
#define MCU_DI19_Pin GPIO_PIN_8
#define MCU_DI19_GPIO_Port GPIOD
#define MCU_DI20_Pin GPIO_PIN_9
#define MCU_DI20_GPIO_Port GPIOD
#define MCU_DI21_Pin GPIO_PIN_10
#define MCU_DI21_GPIO_Port GPIOD
#define MCU_DI22_Pin GPIO_PIN_11
#define MCU_DI22_GPIO_Port GPIOD
#define MCU_DI23_Pin GPIO_PIN_12
#define MCU_DI23_GPIO_Port GPIOD
#define MCU_DI24_Pin GPIO_PIN_13
#define MCU_DI24_GPIO_Port GPIOD
#define MCU_DI25_Pin GPIO_PIN_14
#define MCU_DI25_GPIO_Port GPIOD
#define MCU_DI26_Pin GPIO_PIN_15
#define MCU_DI26_GPIO_Port GPIOD
#define MCU_DI27_Pin GPIO_PIN_2
#define MCU_DI27_GPIO_Port GPIOG
#define MCU_DI28_Pin GPIO_PIN_3
#define MCU_DI28_GPIO_Port GPIOG
#define MCU_DI29_Pin GPIO_PIN_4
#define MCU_DI29_GPIO_Port GPIOG
#define MCU_DI30_Pin GPIO_PIN_5
#define MCU_DI30_GPIO_Port GPIOG
#define MCU_DI31_Pin GPIO_PIN_6
#define MCU_DI31_GPIO_Port GPIOG
#define MCU_DI32_Pin GPIO_PIN_7
#define MCU_DI32_GPIO_Port GPIOG
#define MCU_DI33_Pin GPIO_PIN_8
#define MCU_DI33_GPIO_Port GPIOG
#define MCU_DI34_Pin GPIO_PIN_6
#define MCU_DI34_GPIO_Port GPIOC
#define MCU_DI35_Pin GPIO_PIN_7
#define MCU_DI35_GPIO_Port GPIOC
#define MCU_DI36_Pin GPIO_PIN_8
#define MCU_DI36_GPIO_Port GPIOC
#define MCU_DI37_Pin GPIO_PIN_9
#define MCU_DI37_GPIO_Port GPIOC
#define MCU_DI38_Pin GPIO_PIN_8
#define MCU_DI38_GPIO_Port GPIOA
#define MCU_DI39_Pin GPIO_PIN_9
#define MCU_DI39_GPIO_Port GPIOA
#define MCU_DI40_Pin GPIO_PIN_15
#define MCU_DI40_GPIO_Port GPIOA
#define MCU_DI41_Pin GPIO_PIN_10
#define MCU_DI41_GPIO_Port GPIOC
#define MCU_DI42_Pin GPIO_PIN_11
#define MCU_DI42_GPIO_Port GPIOC
#define MCU_DI43_Pin GPIO_PIN_12
#define MCU_DI43_GPIO_Port GPIOC
#define MCU_DI44_Pin GPIO_PIN_0
#define MCU_DI44_GPIO_Port GPIOD
#define LED_RUN_DIO_Pin GPIO_PIN_1
#define LED_RUN_DIO_GPIO_Port GPIOD
#define LED_ERR_DIO_Pin GPIO_PIN_2
#define LED_ERR_DIO_GPIO_Port GPIOD
#define MCU_DI45_Pin GPIO_PIN_3
#define MCU_DI45_GPIO_Port GPIOD
#define RS485_DI_COM_Pin GPIO_PIN_4
#define RS485_DI_COM_GPIO_Port GPIOD
#define RS485_DI_TX_Pin GPIO_PIN_5
#define RS485_DI_TX_GPIO_Port GPIOD
#define RS485_DI_RX_Pin GPIO_PIN_6
#define RS485_DI_RX_GPIO_Port GPIOD
#define MCU_DI46_Pin GPIO_PIN_7
#define MCU_DI46_GPIO_Port GPIOD
#define MCU_DI47_Pin GPIO_PIN_9
#define MCU_DI47_GPIO_Port GPIOG
#define MCU_DI48_Pin GPIO_PIN_10
#define MCU_DI48_GPIO_Port GPIOG
#define MCU_DI49_Pin GPIO_PIN_11
#define MCU_DI49_GPIO_Port GPIOG
#define MCU_DI50_Pin GPIO_PIN_12
#define MCU_DI50_GPIO_Port GPIOG
#define MCU_DI51_Pin GPIO_PIN_13
#define MCU_DI51_GPIO_Port GPIOG
#define MCU_DI52_Pin GPIO_PIN_14
#define MCU_DI52_GPIO_Port GPIOG
#define MCU_DI53_Pin GPIO_PIN_15
#define MCU_DI53_GPIO_Port GPIOG
#define MCU_DI54_Pin GPIO_PIN_3
#define MCU_DI54_GPIO_Port GPIOB
#define MCU_DI55_Pin GPIO_PIN_4
#define MCU_DI55_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
