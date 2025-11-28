/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32h5xx_hal.h"

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
#define EXP12_Pin GPIO_PIN_13
#define EXP12_GPIO_Port GPIOC
#define EXP10_Pin GPIO_PIN_14
#define EXP10_GPIO_Port GPIOC
#define EXP11_Pin GPIO_PIN_15
#define EXP11_GPIO_Port GPIOC
#define EN_20V_Pin GPIO_PIN_0
#define EN_20V_GPIO_Port GPIOC
#define EN_12V_Pin GPIO_PIN_1
#define EN_12V_GPIO_Port GPIOC
#define EN_IR_Pin GPIO_PIN_2
#define EN_IR_GPIO_Port GPIOC
#define EN_WHITE_Pin GPIO_PIN_3
#define EN_WHITE_GPIO_Port GPIOC
#define CPLD_NSS2_Pin GPIO_PIN_3
#define CPLD_NSS2_GPIO_Port GPIOA
#define CPLD_NSS1_Pin GPIO_PIN_4
#define CPLD_NSS1_GPIO_Port GPIOA
#define TERM_EN_Pin GPIO_PIN_4
#define TERM_EN_GPIO_Port GPIOC
#define STAT1_Pin GPIO_PIN_5
#define STAT1_GPIO_Port GPIOC
#define CPLD_RSTn_Pin GPIO_PIN_0
#define CPLD_RSTn_GPIO_Port GPIOB
#define CPLD_DONE_Pin GPIO_PIN_1
#define CPLD_DONE_GPIO_Port GPIOB
#define EXP2_Pin GPIO_PIN_2
#define EXP2_GPIO_Port GPIOB
#define EXP7_Pin GPIO_PIN_13
#define EXP7_GPIO_Port GPIOB
#define STAT2_Pin GPIO_PIN_6
#define STAT2_GPIO_Port GPIOC
#define MPP_EN_Pin GPIO_PIN_7
#define MPP_EN_GPIO_Port GPIOC
#define EXP8_Pin GPIO_PIN_8
#define EXP8_GPIO_Port GPIOC
#define EXP9_Pin GPIO_PIN_9
#define EXP9_GPIO_Port GPIOC
#define USOUND_SYNC0_Pin GPIO_PIN_8
#define USOUND_SYNC0_GPIO_Port GPIOA
#define USOUND_SYNC1_Pin GPIO_PIN_9
#define USOUND_SYNC1_GPIO_Port GPIOA
#define USOUND_SYNC2_Pin GPIO_PIN_10
#define USOUND_SYNC2_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_15
#define LED1_GPIO_Port GPIOA
#define PIR4_Pin GPIO_PIN_10
#define PIR4_GPIO_Port GPIOC
#define PIR3_Pin GPIO_PIN_11
#define PIR3_GPIO_Port GPIOC
#define PIR2_Pin GPIO_PIN_12
#define PIR2_GPIO_Port GPIOC
#define PIR1_Pin GPIO_PIN_2
#define PIR1_GPIO_Port GPIOD
#define EXP3_Pin GPIO_PIN_3
#define EXP3_GPIO_Port GPIOB
#define EXP4_Pin GPIO_PIN_4
#define EXP4_GPIO_Port GPIOB
#define EXP5_Pin GPIO_PIN_5
#define EXP5_GPIO_Port GPIOB
#define EXP6_Pin GPIO_PIN_8
#define EXP6_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
