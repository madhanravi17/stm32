/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32h7xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{

  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
  * @brief ADC MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(hadc->Instance==ADC3)
  {
    /* USER CODE BEGIN ADC3_MspInit 0 */
	  /* ============================================================
	   * STM32H7 – PLL2 → ADC clock calculation
	   * ============================================================
	   *
	   * General formula:
	   *
	   *   fVCO = fIN / PLL2M * PLL2N
	   *   fADC = fVCO / PLL2P        (when RCC_ADCCLKSOURCE_PLL2)
	   *
	   * Where:
	   *   fIN  = HSI = 64 MHz
	   *   ADC clock source = PLL2P
	   */

	  /* ------------------------------------------------------------
	   * CASE 1: ADC = 20 MHz
	   * ------------------------------------------------------------ */
	  /*
	  PLL2M = 4;
	  PLL2N = 10;
	  PLL2P = 8;
	  */
	  /*
	   * fVCO = 64 / 4 * 10 = 160 MHz
	   * fADC = 160 / 8    = 20 MHz
	   */


	  /* ------------------------------------------------------------
	   * CASE 2: ADC = 80 MHz
	   * ------------------------------------------------------------ */
	  /*
	  PLL2M = 4;
	  PLL2N = 10;
	  PLL2P = 2;
	  */
	  /*
	   * fVCO = 64 / 4 * 10 = 160 MHz
	   * fADC = 160 / 2    = 80 MHz
	   */


	  /* ------------------------------------------------------------
	   * Key rule (do not forget):
	   *
	   *  - Same M and N  → same VCO
	   *  - PLL2P alone selects ADC speed
	   *  - ADC clock = VCO / PLL2P
	   * ------------------------------------------------------------ */

    /* USER CODE END ADC3_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInitStruct.PLL2.PLL2M = 4;
    PeriphClkInitStruct.PLL2.PLL2N = 10;
    PeriphClkInitStruct.PLL2.PLL2P = 8;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 2;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_ADC3_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**ADC3 GPIO Configuration
    PC3_C     ------> ADC3_INP1
    */
    HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC3, SYSCFG_SWITCH_PC3_OPEN);

    /* USER CODE BEGIN ADC3_MspInit 1 */

    /* USER CODE END ADC3_MspInit 1 */

  }

}

/**
  * @brief ADC MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC3)
  {
    /* USER CODE BEGIN ADC3_MspDeInit 0 */

    /* USER CODE END ADC3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC3_CLK_DISABLE();

    /* USER CODE BEGIN ADC3_MspDeInit 1 */

    /* USER CODE END ADC3_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
