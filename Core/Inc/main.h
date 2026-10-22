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
#include <stdint.h>
#include <math.h>

#include "time.h"
#include "chinch.h"
#include "knob.h"
#include "signal.h"
#include "ui.h"
#include "switch.h"

#include "st7789.h"
#include "ui_helpers.h"
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
#define Encoder_A_Pin GPIO_PIN_6
#define Encoder_A_GPIO_Port GPIOA
#define Encoder_B_Pin GPIO_PIN_7
#define Encoder_B_GPIO_Port GPIOA
#define Knob_button_Pin GPIO_PIN_0
#define Knob_button_GPIO_Port GPIOB
#define PC_Switch_Pin GPIO_PIN_12
#define PC_Switch_GPIO_Port GPIOB
#define BT_Switch_Pin GPIO_PIN_13
#define BT_Switch_GPIO_Port GPIOB
#define Base_Switch_Pin GPIO_PIN_14
#define Base_Switch_GPIO_Port GPIOB
#define Chinch_Pin GPIO_PIN_15
#define Chinch_GPIO_Port GPIOB
#define TFT_SCK_Pin GPIO_PIN_3
#define TFT_SCK_GPIO_Port GPIOB
#define TFT_MOSI_Pin GPIO_PIN_5
#define TFT_MOSI_GPIO_Port GPIOB
#define TFT_CS_Pin GPIO_PIN_6
#define TFT_CS_GPIO_Port GPIOB
#define TFT_RST_Pin GPIO_PIN_7
#define TFT_RST_GPIO_Port GPIOB
#define TFT_DC_Pin GPIO_PIN_8
#define TFT_DC_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
