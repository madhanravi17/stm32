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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

FDCAN_HandleTypeDef hfdcan2;

/* USER CODE BEGIN PV */
/* ================= DEBUG ================= */
volatile uint32_t rx_id_dbg;
volatile uint8_t rx_len_dbg;     // 8 bits for DLC ranges from  0 to 15
volatile uint8_t rx_data_dbg[2]; /* DLC = 2 bytes */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FDCAN2_Init(void);
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

  /* MCU Configuration--------------------------------------------------------*/

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
  MX_FDCAN2_Init();
  /* USER CODE BEGIN 2 */
  /* Accept all standard IDs */
  FDCAN_FilterTypeDef filter = {0};
  filter.IdType = FDCAN_STANDARD_ID;
  filter.FilterIndex = 0;
  filter.FilterType = FDCAN_FILTER_MASK;
  filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  filter.FilterID1 = 0x000;
  filter.FilterID2 = 0x000;
  HAL_FDCAN_ConfigFilter(&hfdcan2, &filter);

  HAL_FDCAN_Start(&hfdcan2);

  /* TX header */
  FDCAN_TxHeaderTypeDef txh = {0};
  txh.Identifier = 0x123;
  txh.IdType = FDCAN_STANDARD_ID;
  txh.TxFrameType = FDCAN_DATA_FRAME;
  txh.DataLength = FDCAN_DLC_BYTES_2;
  txh.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  txh.BitRateSwitch = FDCAN_BRS_OFF;
  txh.FDFormat = FDCAN_CLASSIC_CAN;
  txh.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  txh.MessageMarker = 0;

  uint8_t txd[2] = {0x1, 0x2}; /* 2 bytes data => 0x11 = 1, 0x22 = 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    while (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan2) == 0) {
    }

    /* Enqueue frame */
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &txh, txd);

    /* Wait for RX */
    while (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan2, FDCAN_RX_FIFO0) == 0) {
      // wait for filling
    }
    /* Read RX */
    FDCAN_RxHeaderTypeDef rxh;
    HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &rxh, (uint8_t*) rx_data_dbg);

    rx_id_dbg = rxh.Identifier;
    // DLC from 9 to 15 means 12 to 64 bytes (see RM)
    rx_len_dbg = rxh.DataLength; // HAL provides raw DLC (not byte count)
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

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
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief FDCAN2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_FDCAN2_Init(void) {

  /* USER CODE BEGIN FDCAN2_Init 0 */
  /* ============ 500 kbit/s Bit Timing Derivation ============
   *
   * Step 1: FDCAN Kernel Clock
   *   fCAN = 80 MHz (from Clock Configuration)
   *
   * Step 2: Prescaler
   *   Prescaler = 10
   *
   * Step 3: Time Quantum (tq)
   *   tq = Prescaler / fCAN
   *   tq = 10 / 80,000,000 Hz = 0.125 µs = 125 ns
   *
   * Step 4: Bit Segments (in TQ)
   *   Sync Segment   = 1 TQ  (fixed, always 1)
   *   Time Segment 1 = 12 TQ (Prop_Seg + Phase_Seg1)
   *   Time Segment 2 = 3 TQ  (Phase_Seg2)
   *
   * Step 5: Total Time Quanta per Bit
   *   Total TQ = Sync + TS1 + TS2 = 1 + 12 + 3 = 16 TQ
   *
   * Step 6: Bit Time
   *   Bit Time = Total TQ × tq = 16 × 125 ns = 2000 ns = 2 µs
   *
   * Step 7: Nominal Bit Rate
   *   Bit Rate = 1 / Bit Time = 1 / 2 µs = 500 kbit/s
   *
   * Step 8: Sample Point
   *   Sample Point = (Sync + TS1) / Total TQ = (1 + 12) / 16 = 81.25%
   *
   * Step 9: Frame Time Estimate (Standard CAN, 11-bit ID, 2 data bytes)
   *   Min frame bits ≈ 47 bits (no stuff bits)
   *   Max frame bits ≈ 80 bits (worst-case bit stuffing)
   *   Frame time ≈ 47–80 bits × 2 µs = 94–160 µs
   */
  /* USER CODE END FDCAN2_Init 0 */

  /* USER CODE BEGIN FDCAN2_Init 1 */

  /* USER CODE END FDCAN2_Init 1 */
  hfdcan2.Instance = FDCAN2;
  hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan2.Init.Mode = FDCAN_MODE_EXTERNAL_LOOPBACK;
  hfdcan2.Init.AutoRetransmission =
      DISABLE; // for loopback testing , no Ack, so no retries of frame
               // transmission
  hfdcan2.Init.TransmitPause = DISABLE;
  hfdcan2.Init.ProtocolException = DISABLE;
  hfdcan2.Init.NominalPrescaler = 10;
  hfdcan2.Init.NominalSyncJumpWidth = 1;
  hfdcan2.Init.NominalTimeSeg1 = 12;
  hfdcan2.Init.NominalTimeSeg2 = 3;
  hfdcan2.Init.DataPrescaler = 1;
  hfdcan2.Init.DataSyncJumpWidth = 1;
  hfdcan2.Init.DataTimeSeg1 = 1;
  hfdcan2.Init.DataTimeSeg2 = 1;
  hfdcan2.Init.MessageRAMOffset = 0;
  hfdcan2.Init.StdFiltersNbr = 1; // 1 filter
  hfdcan2.Init.ExtFiltersNbr = 0;
  hfdcan2.Init.RxFifo0ElmtsNbr = 1;                  // 1 element
  hfdcan2.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8; // 8 bytes per element
  hfdcan2.Init.RxFifo1ElmtsNbr = 0;
  hfdcan2.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan2.Init.RxBuffersNbr = 0;
  hfdcan2.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan2.Init.TxEventsNbr = 32;
  hfdcan2.Init.TxBuffersNbr = 0; // TX FIFO mode: buffers disabled
  hfdcan2.Init.TxFifoQueueElmtsNbr =
      32; // CubeMX enforces full 32-slot TX RAM allocation
  hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan2.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN2_Init 2 */

  /* USER CODE END FDCAN2_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
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
void assert_failed(uint8_t* file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
