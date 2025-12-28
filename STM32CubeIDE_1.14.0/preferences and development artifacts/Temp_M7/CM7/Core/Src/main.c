/* USER CODE BEGIN Header */
/* ================================================================
 * LABEL: TEMPERATURE SENSOR
 * Target  : STM32H745
 * Module  : ADC3 – Internal Temperature Sensor (TSENSE)
 * ================================================================ */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : STM32H745 ADC3 internal temperature sensor example
  *
  * This example demonstrates:
  * - ADC3 continuous conversion mode
  * - Internal temperature sensor (TSENSE) usage
  * - 16-bit ADC resolution matched to factory calibration values
  * - ADC clock derived from PLL2P (20 MHz) via CubeMX
  * - Long sampling time (810.5 cycles ≈ 40.5 µs) required for TSENSE
  * - Junction temperature measurement (not ambient temperature)
  *
  * Key notes:
  * - ADC is started once; conversions run continuously in hardware
  * - EOC flag is polled to read updated samples
  * - Factory calibration values are used directly (no shifting needed)
  * - Temperature reflects MCU die temperature under load/debug
  *
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* DUAL_CORE_BOOT_SYNC_SEQUENCE: Define for dual core boot synchronization    */
/*                             demonstration code based on hardware semaphore */
/* This define is present in both CM7/CM4 projects                            */
/* To comment when developping/debugging on a single core                     */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* ===================== CALIBRATION DOMAIN =====================
   Factory TS_CAL values are 16-bit right-aligned.
   ADC resolution = 16-bit → same numeric domain.
   No shifting required.
   If ADC were 12-bit → calibration must be >> 4.
   ============================================================ */

/* ================= FACTORY CALIBRATION ================= */
#define TS_CAL1_ADDR   ((uint16_t*)0x1FF1E820)   /* ADC raw @ 30 °C */
#define TS_CAL2_ADDR   ((uint16_t*)0x1FF1E840)   /* ADC raw @ 110 °C */
#define TS_CAL1_TEMP   30.0f
#define TS_CAL2_TEMP   110.0f
//#define TS_CAL1_RAW   ((*TS_CAL1_ADDR) >> 4) //// needed when 12 bit resolution is neccessary
//#define TS_CAL2_RAW   ((*TS_CAL2_ADDR) >> 4)

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc3;

/* USER CODE BEGIN PV */
/* ================= DEBUG VARIABLES ================= */
volatile uint16_t adc_raw_dbg;
volatile float  temperature_c_dbg;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_ADC3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* ---- HERE. ONLY HERE. ---- */
 // ADC123_COMMON->CCR |= ADC_CCR_TSEN | ADC_CCR_VREFEN;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */

  /* ---- CONTINUOUS MODE: START ONCE ---- */
  HAL_ADC_Start(&hadc3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  //if (HAL_ADC_PollForConversion(&hadc3, HAL_MAX_DELAY) == HAL_OK) /// not needed
	  if (__HAL_ADC_GET_FLAG(&hadc3, ADC_FLAG_EOC))
	          {
	              adc_raw_dbg = HAL_ADC_GetValue(&hadc3);
	              /* ---- RAW ADC → TEMPERATURE (RM FORMULA) ---- */
/*/// needed when 12 bits is needed shifting 4 bits to the right from 16 to 12 , basically bit division
	              temperature_c_dbg =
	                  ((float)(adc_raw_dbg - TS_CAL1_RAW) *
	                  (TS_CAL2_TEMP - TS_CAL1_TEMP)) /
	                  ((float)(TS_CAL2_RAW - TS_CAL1_RAW)) +
	                  TS_CAL1_TEMP;
*/
	              temperature_c_dbg =
	                          ((float)(adc_raw_dbg - *TS_CAL1_ADDR) *
	                          (TS_CAL2_TEMP - TS_CAL1_TEMP)) /
	                          ((float)(*TS_CAL2_ADDR - *TS_CAL1_ADDR)) +
	                          TS_CAL1_TEMP;

	          }

	          HAL_Delay(100);   /* 10 Hz update — sensor is slow */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
/* ===================== SYSTEM CLOCK (CubeMX) =====================
   HSI = 64 MHz
   PLL1: M=4, N=10, R=2 → SYSCLK = 80 MHz
   D1CPRE = /1 → CPU = 80 MHz
   HPRE   = /4 → HCLK = 20 MHz
   APB1/2/3/4 = /1 → PCLK = 20 MHz
   ================================================================ */
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  /* ===================== ADC CLOCK (CubeMX) =====================
     ADC clock source = PLL2P
     PLL2: M=4, N=20 → VCO = 320 MHz
     PLL2P division → 20 MHz to ADC kernel
     ADC kernel clock = 20 MHz (as shown in CubeMX)
     ============================================================= */

  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /* ===================== ADC SAMPLING TIME =====================
     ADC kernel clock = 20 MHz → Tclk = 50 ns
     Sampling = 810.5 cycles
     Ts = 810.5 × 50 ns ≈ 40.5 µs
     TSENSE min requirement ≥ 10 µs → safely met according to Datasheet
     which mentions minimum of 9 µs!!
     https://www.st.com/resource/en/datasheet/stm32h745zi.pdf page 196/249
     ============================================================ */

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection    = RCC_ADCCLKSOURCE_PLL2; //ADC clock source

  /* PLL2 = 20 MHz example */
  PeriphClkInit.PLL2.PLL2M = 4;
  PeriphClkInit.PLL2.PLL2N = 20;
  PeriphClkInit.PLL2.PLL2P = 2;
  PeriphClkInit.PLL2.PLL2Q = 2;
  PeriphClkInit.PLL2.PLL2R = 2;

  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

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
  hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc3.Init.OversamplingMode = DISABLE;
  hadc3.Init.Oversampling.Ratio = 1;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;

  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
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
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
