/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/*
 * =============================================================================
 * I2C MASTER-SLAVE LOOPBACK WORKFLOW (Same MCU, Two I2C Peripherals)
 * =============================================================================
 *
 * Hardware Setup:
 *   - I2C1 (Master): PB8 (SCL), PB9 (SDA)
 *   - I2C4 (Slave):  PF14 (SCL), PF15 (SDA)
 *   - Physical wire connection required between I2C1 and I2C4 pins
 *   - External pull-up resistors (4.7kΩ) on SCL and SDA lines
 *
 * Communication Flow:
 *   1. Master TX  → Slave RX  (Master sends "Hello from CM7 Master!")
 *   2. [20ms delay via flag - safe in main loop]
 *   3. Master RX  ← Slave TX  (Slave responds with acknowledgment)
 *   4. [100ms delay via flag - safe in main loop]
 *   5. Repeat from step 1
 *
 * Interrupt Chain (IRQ → HAL Handler → Callback):
 *   I2C1_EV_IRQHandler → HAL_I2C_EV_IRQHandler → HAL_I2C_MasterTxCpltCallback
 *   I2C1_EV_IRQHandler → HAL_I2C_EV_IRQHandler → HAL_I2C_MasterRxCpltCallback
 *   I2C4_EV_IRQHandler → HAL_I2C_EV_IRQHandler → HAL_I2C_SlaveRxCpltCallback
 *   I2C4_EV_IRQHandler → HAL_I2C_EV_IRQHandler → HAL_I2C_SlaveTxCpltCallback
 *
 * CRITICAL: ISR Delay Violation
 *   - HAL_Delay() CANNOT be used inside ISR/callbacks
 *   - HAL_Delay() relies on SysTick interrupt (lower/equal priority)
 *   - Using HAL_Delay() in ISR causes system hang (deadlock)
 *   - Solution: Use flags and handle delays in main loop
 *
 * =============================================================================
 */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* 0x01 = 0b00000001 << 1 = 0x02, for 7-bit addressing mode
   LSB last 8th bit is R/W bit
   The slave address used for I2C communication 
*/
#define I2C_Slave_ADDRESS 0x01 << 1
/* 0x02 = 0b00000010 << 1 = 0x04, for 7-bit addressing mode
   LSB last 8th bit is R/W bit
   The master address used for I2C communication 
   when slave communicates back to master 
*/
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c4;

/* USER CODE BEGIN PV */
uint8_t txMData[] = "Hello from CM7 Master!";
/* sizeof(txMData) must be equal to sizeof(rxSData)
therefore sizeof(txMData) == 23 so number of bytes transferred is 23
sizeof(txMData) / sizeof(txMData[0]) = 23 / 1 = 23 bytes
so shortcut sizeof(txMData) is the same as number of bytes transferred = 23
------
buffer to store data received by slave during I2C master communication
*/
/* String length WITHOUT null terminator for I2C transmission
 * sizeof(txMData) = 23 (includes '\0')
 * sizeof(txMData) - 1 = 22 (actual string length, excludes '\0')
 * This prevents NUL character from appearing in logic analyzer decode
 */
#define TX_MASTER_LEN (sizeof(txMData) - 1) // 22 bytes, no NUL

uint8_t rxSData[sizeof(txMData)] = {""};
#define RX_SLAVE_LEN TX_MASTER_LEN // Must match what master sends

uint8_t txSData[] = "Hello from CM7 Master! Positive response from slave!";
#define TX_SLAVE_LEN (sizeof(txSData) - 1) // 52 bytes, no NUL

uint8_t rxMData[sizeof(txSData)] = {""};
#define RX_MASTER_LEN TX_SLAVE_LEN // Must match what slave sends

volatile uint8_t masterTxReady = 0; // Flag to trigger next master TX from main loop
volatile uint8_t masterRxReady = 0; // Flag to trigger master RX after TX complete (20ms delay)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C4_Init(void);
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

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C4_Init();
  /* USER CODE BEGIN 2 */
  /* Arm slave receiver first (must be ready before master transmits)
   * Using RX_SLAVE_LEN to exclude null terminator from transfer */
  HAL_I2C_Slave_Receive_IT(&hi2c4, rxSData, RX_SLAVE_LEN);

  /* Start first master transmission
   * Using TX_MASTER_LEN to exclude null terminator (no NUL in decode) */
  HAL_I2C_Master_Transmit_IT(&hi2c1, I2C_Slave_ADDRESS, txMData, TX_MASTER_LEN);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* Handle Master RX trigger (20ms delay after TX complete)
     * This delay gives the slave time to prepare its response data */
    if (masterRxReady) {
      masterRxReady = 0;
      HAL_Delay(1); // 1ms delay between Master TX complete and Master RX start
      HAL_I2C_Master_Receive_IT(&hi2c1, I2C_Slave_ADDRESS, rxMData, RX_MASTER_LEN);
    }

    /* Handle next Master TX trigger (100ms delay between communication cycles)
     * This delay is the inter-cycle gap before starting next transmission */
    if (masterTxReady) {
      masterTxReady = 0;
      HAL_Delay(100); // 100ms delay between communication cycles
      HAL_I2C_Master_Transmit_IT(&hi2c1, I2C_Slave_ADDRESS, txMData, TX_MASTER_LEN);
    }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void) {

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00707CBB; //100 Khz
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */
}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_Init(void) {

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x00707CBB;
  hi2c4.Init.OwnAddress1 = I2C_Slave_ADDRESS;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK) {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */
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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/*
 * =============================================================================
 * I2C CALLBACK FUNCTIONS
 * =============================================================================
 * These callbacks are invoked by HAL from within the IRQ handlers.
 * They execute in interrupt context - keep them short and non-blocking!
 *
 * WARNING: Never use HAL_Delay() or blocking functions in callbacks!
 * =============================================================================
 */

/**
 * @brief  Master Transmit Complete Callback
 * @note   Called when I2C1 (Master) finishes transmitting data to slave
 * @note   Sets flag to trigger Master RX from main loop (with 20ms delay)
 * @param  hi2c: I2C handle pointer
 */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* hi2c) {
  if (hi2c->Instance == I2C1) {
    /* Set flag to trigger Master Receive from main loop
     * DO NOT call HAL_I2C_Master_Receive_IT directly here if delay is needed
     * The 20ms delay will be handled safely in the main loop */
    masterRxReady = 1;
  }
}

/**
 * @brief  Master Receive Complete Callback
 * @note   Called when I2C1 (Master) finishes receiving data from slave
 * @note   rxMData buffer now contains slave's response
 * @note   Sets flag to trigger next TX cycle from main loop (with 100ms delay)
 * @param  hi2c: I2C handle pointer
 */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c) {
  if (hi2c->Instance == I2C1) {
    /* rxMData now contains: "Hello from CM7 Master! Positive response from slave!"
     * Set flag to trigger next Master Transmit from main loop
     * CRITICAL: HAL_Delay() CANNOT be used here - it would cause deadlock!
     * The 100ms inter-cycle delay will be handled safely in the main loop */
    masterTxReady = 1;
  }
}

/**
 * @brief  Slave Receive Complete Callback
 * @note   Called when I2C4 (Slave) finishes receiving data from master
 * @note   rxSData buffer now contains master's message
 * @note   Immediately arms slave TX to prepare response for master's read request
 * @param  hi2c: I2C handle pointer
 */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef* hi2c) {
  if (hi2c->Instance == I2C4) {
    /* rxSData now contains: "Hello from CM7 Master!"
     * Arm slave transmitter - when master initiates a read, slave will respond
     * Using TX_SLAVE_LEN to exclude null terminator (no NUL in decode) */
    HAL_I2C_Slave_Transmit_IT(&hi2c4, txSData, TX_SLAVE_LEN);
  }
}

/**
 * @brief  Slave Transmit Complete Callback
 * @note   Called when I2C4 (Slave) finishes transmitting response to master
 * @note   Re-arms slave receiver for next incoming message from master
 * @param  hi2c: I2C handle pointer
 */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef* hi2c) {
  if (hi2c->Instance == I2C4) {
    /* Slave finished sending response to master
     * Re-arm slave receiver to be ready for next master transmission
     * Using RX_SLAVE_LEN to match expected incoming data length */
    HAL_I2C_Slave_Receive_IT(&hi2c4, rxSData, RX_SLAVE_LEN);
  }
}
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
