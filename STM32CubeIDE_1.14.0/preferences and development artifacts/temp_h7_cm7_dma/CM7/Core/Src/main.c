/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
 =======================================================================
 * ANALOG TEMPERATURE PROBING VIA DAC — WITH SCOPE MATH RECONSTRUCTION
 *
 * This block documents BOTH sides of the system:
 *   1) Firmware-side DAC encoding
 *   2) Oscilloscope-side math used to recover °C from voltage
 *
 * This closes the loop and proves correctness end-to-end.
 *
 * -----------------------------------------------------------------------
 * HARDWARE / DAC FACTS
 *
 *   VDDA (DAC reference) ≈ 3.3 V
 *   DAC resolution       = 12-bit → 0 … 4095
 *
 *   General DAC equation:
 *     Vout = (DAC_code / 4095) × 3.3 V
 *
 *   Inverse (used in scope math):
 *     DAC_code ≈ (Vout / 3.3) × 4095
 *
 * -----------------------------------------------------------------------
 * DAC OUT1 (PA4) — JUNCTION TEMPERATURE
 *
 * Firmware mapping:
 *   Range : 0 … 66 °C  →  0 … 3.3 V
 *   Gain  : 50 mV / °C
 *
 * Firmware code:
 *   dac_out1 = (temp_equivalent × 4095) / 66;
 *
 * Resulting voltage:
 *   Vout1 = temp_equivalent × (3.3 / 66)
 *         = temp_equivalent × 0.05 V/°C
 *
 * -----------------------------------------------------------------------
 * SCOPE MATH — JUNCTION TEMPERATURE (OUT1)
 *
 * Measured signal:
 *   CH_A = Vout1  (Volts)
 *
 * Math to recover temperature:
 *   JunctionTemp(°C) = Vout1 / 0.05
 *                    = Vout1 × 20
 *
 * PicoScope math channel:
 *   JunctionTemp = A × 20
 *
 * Example:
 *   Measured Vout1 ≈ 2.8 V
 *   JunctionTemp  ≈ 2.8 × 20 = 56 °C
 *
 * Matches debugger value.
 *
 * -----------------------------------------------------------------------
 * DAC OUT2 (PA5) — ROOM TEMPERATURE (OFFSET + GAIN)
 *
 * Concept:
 *   - Remove junction offset (~25 °C)
 *   - Encode room temperature only
 *
 * Firmware steps:
 *   roomtemp = temp_equivalent − 25 °C
 *   clamp roomtemp to 0 … 33 °C
 *
 * Mapping:
 *   0 … 33 °C → 0 … 3.3 V
 *   Gain = 100 mV / °C
 *
 * Firmware code:
 *   dac_out2 = (roomtemp × 4095) / 33;
 *
 * Resulting voltage:
 *   Vout2 = roomtemp × (3.3 / 33)
 *         = roomtemp × 0.1 V/°C
 *
 * -----------------------------------------------------------------------
 * SCOPE MATH — ROOM TEMPERATURE (OUT2)
 *
 * Measured signal:
 *   CH_B = Vout2  (Volts)
 *
 * Math to recover room temperature:
 *   RoomTemp(°C) = Vout2 / 0.1
 *                = Vout2 × 10
 *
 * PicoScope math channel:
 *   RoomTemp = B × 10
 *
 * Example:
 *   Measured Vout2 ≈ 2.8 V
 *   RoomTemp      ≈ 2.8 × 10 = 28 °C
 *
 * Matches debugger value.
 *
 * -----------------------------------------------------------------------
 * CRITICAL MEASUREMENT NOTE (ROOT CAUSE FOUND)
 *
 * Earlier false readings (≈29 V, saturation at 33 °C) were NOT firmware bugs.
 *
 * Cause:
 *   - Probe gain accidentally set to ×2
 *
 * Effect:
 *   - Displayed voltage = actual voltage × 2
 *   - 2.8 V appeared as ~5.6 V
 *   - Math channels became meaningless
 *
 * Correct probe configuration:
 *   - Gain      : ×1
 *   - Coupling  : DC
 *   - Same scale on both channels
 *
 * After correction:
 *   - OUT1 and OUT2 voltages match calculations
 *   - Scope math recovers correct temperatures
 *   - Firmware and hardware fully validated
 *
 * -----------------------------------------------------------------------
 * FINAL STATE
 *
 *   Firmware math  ✔
 *   DAC scaling    ✔
 *   Scope math     ✔
 *   Probe setup    ✔
 *   Physical truth ✔
 *
 * System is behaving exactly as designed.
 * =======================================================================
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc3;
DAC_HandleTypeDef hdac1;

/* USER CODE BEGIN PV */
// HAL_ADC_Start_DMA requires pData as uint32_t*
static volatile uint16_t adc = 0;
static volatile int32_t temp = 0;
static volatile uint16_t dac_temp_voltage = 0;
static volatile uint16_t dac_roomtemp_voltage = 0;
static volatile uint32_t temp_equivalent = 0;
static volatile float roomtemp = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC3_Init(void);
static void MX_DAC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC3_Init();
  MX_DAC1_Init();
  /* USER CODE BEGIN 2 */
  /* ENABLE DAC OUTPUTS */
  HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
  HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
  /* (uint32_t *) used since it was declared as volatile!!
   * read 1 value into adc in circular mode only valid recent value
   */
  HAL_ADC_Start_DMA(&hadc3, (uint32_t *)&adc,
                    1); // read 1 value into adc in circular mode recent values
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* Snapshot DMA-updated value */
    temp = __HAL_ADC_CALC_TEMPERATURE(3300, (int32_t)adc,
                                      ADC_GET_RESOLUTION(&hadc3));
    // saturate temp to 0 to 100 c for voltage equivalent
    if (temp < 0) {
      temp_equivalent = 0;
    } else if (temp > 66) {
      temp_equivalent = 66;
    } else {
      temp_equivalent = temp;
    }
    // HAL_Delay(1E3); // 1 second delay
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* Set DAC output to temperature equivalent voltage */
    dac_temp_voltage = (temp_equivalent * 4095) / 66;
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_temp_voltage);

    /* --- Clamp for 100 mV / °C (0–33 °C → 0–3.3 V) --- */
    roomtemp = (float)temp_equivalent - 25.0f; // manual calibration
    if (roomtemp < 0.0f)
      roomtemp = 0.0f;
    else if (roomtemp > 33.0f)
      roomtemp = 33.0f;
    // 100mV / degC
    dac_roomtemp_voltage = (uint16_t)((roomtemp * 4095.0f) / 33.0f);
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R,
                     dac_roomtemp_voltage);
    /* =======================================================================
     * LINEAR TEMPERATURE → DAC VOLTAGE MAPPING (50 mV / °C)
     *
     * Design choice:
     *   - Use the DAC as an analog probe to visualize temperature on a scope
     *   - Keep the signal linear, monotonic, and within DAC headroom
     *
     * Electrical constraints:
     *   - DAC reference (VDDA) ≈ 3.3 V
     *   - 12-bit DAC → codes 0 … 4095
     *
     * Selected scale:
     *   - 0 … 66 °C  →  0 … 3.3 V
     *   - Slope = 3.3 V / 66 °C ≈ 50 mV / °C
     *
     * Rationale:
     *   - Avoid saturation at normal operating temperatures
     *   - Preserve full DAC resolution across a useful thermal range
     *   - Simple integer math, no floating point
     *
     * -----------------------------------------------------------------------
     * Runtime values (example shown in debugger):
     *   temp            = 54 °C
     *   temp_equivalent = 54 (within clamp range)
     *
     * DAC computation:
     *   dac_code = (temp_equivalent / 66) × 4095
     *            = (54 × 4095) / 66
     *            ≈ 3350
     *
     * Electrical output:
     *   Vout = (3350 / 4095) × 3.3 V ≈ 2.7 V
     *   Which matches:
     *   54 °C × 50 mV/°C = 2.7 V
     *
     * This confirms:
     *   - Correct linear mapping
     *   - Correct clamping
     *   - Correct DAC behavior
     * =======================================================================
     */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
   */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOMEDIUM;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV4;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief ADC3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC3_Init(void) {

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Common config
   */
  hadc3.Instance = ADC3;
  hadc3.Init.Resolution = ADC_RESOLUTION_16B;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = ENABLE;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ConversionDataManagement =
      ADC_CONVERSIONDATA_DMA_CIRCULAR; // DMA mode no callbacks necessary!
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc3.Init.OversamplingMode = DISABLE;
  hadc3.Init.Oversampling.Ratio = 1;
  if (HAL_ADC_Init(&hadc3) != HAL_OK) {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */
}

/**
 * @brief DAC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_DAC1_Init(void) {

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
   */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK) {
    Error_Handler();
  }

  /** DAC channel OUT1 config
   */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK) {
    Error_Handler();
  }

  /** DAC channel OUT2 config
   */
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pins : PD8 PD9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* ================= DMA CALLBACK ================= */
/*
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  if (hadc->Instance == ADC3) {
    adc_ready = 1;
  }
}
*/
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state
   */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
     file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
