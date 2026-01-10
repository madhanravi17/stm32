/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32h7xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*
 * =============================================================================
 * INTERRUPT SERVICE ROUTINES (ISR) - I2C WORKFLOW
 * =============================================================================
 *
 * IRQ Handler Chain:
 *   Hardware Event → NVIC → IRQHandler → HAL_I2C_XX_IRQHandler → User Callback
 *
 * I2C1 (Master) Events:
 *   I2C1_EV_IRQHandler → HAL_I2C_EV_IRQHandler(&hi2c1)
 *     → HAL_I2C_MasterTxCpltCallback (after TX complete)
 *     → HAL_I2C_MasterRxCpltCallback (after RX complete)
 *
 * I2C4 (Slave) Events:
 *   I2C4_EV_IRQHandler → HAL_I2C_EV_IRQHandler(&hi2c4)
 *     → HAL_I2C_SlaveRxCpltCallback (after receiving from master)
 *     → HAL_I2C_SlaveTxCpltCallback (after transmitting to master)
 *
 * Error Handling:
 *   I2C1_ER_IRQHandler → HAL_I2C_ER_IRQHandler(&hi2c1) → HAL_I2C_ErrorCallback
 *   I2C4_ER_IRQHandler → HAL_I2C_ER_IRQHandler(&hi2c4) → HAL_I2C_ErrorCallback
 *
 * CRITICAL REMINDER:
 *   - ISRs must be fast and non-blocking
 *   - Never use HAL_Delay() in ISR context (causes deadlock)
 *   - SysTick_Handler has equal/lower priority - HAL_Delay waits forever
 *
 * =============================================================================
 */
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c4;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void) {
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1) {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void) {
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void) {
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void) {
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void) {
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void) {
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void) {
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void) {
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void) {
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles I2C1 event interrupt.
  * @note  I2C1 is configured as MASTER
  * @note  Triggers: TX complete, RX complete, address sent, etc.
  * @note  HAL handler dispatches to appropriate callback in main.c
  */
void I2C1_EV_IRQHandler(void) {
  /* USER CODE BEGIN I2C1_EV_IRQn 0 */
  /* Master event: TX/RX complete, NACK, etc. */
  /* USER CODE END I2C1_EV_IRQn 0 */
  HAL_I2C_EV_IRQHandler(&hi2c1); /* → MasterTxCpltCallback or MasterRxCpltCallback */
  /* USER CODE BEGIN I2C1_EV_IRQn 1 */

  /* USER CODE END I2C1_EV_IRQn 1 */
}

/**
  * @brief This function handles I2C1 error interrupt.
  * @note  I2C1 is configured as MASTER
  * @note  Triggers: Bus error, arbitration lost, ACK failure, overrun, timeout
  */
void I2C1_ER_IRQHandler(void) {
  /* USER CODE BEGIN I2C1_ER_IRQn 0 */
  /* Master error: NACK, bus error, arbitration lost, etc. */
  /* USER CODE END I2C1_ER_IRQn 0 */
  HAL_I2C_ER_IRQHandler(&hi2c1); /* → HAL_I2C_ErrorCallback if implemented */
  /* USER CODE BEGIN I2C1_ER_IRQn 1 */

  /* USER CODE END I2C1_ER_IRQn 1 */
}

/**
  * @brief This function handles I2C4 event interrupt.
  * @note  I2C4 is configured as SLAVE (address: 0x02)
  * @note  Triggers: Address match, TX complete, RX complete, STOP detected
  * @note  HAL handler dispatches to appropriate callback in main.c
  */
void I2C4_EV_IRQHandler(void) {
  /* USER CODE BEGIN I2C4_EV_IRQn 0 */
  /* Slave event: address match, RX/TX complete, STOP, etc. */
  /* USER CODE END I2C4_EV_IRQn 0 */
  HAL_I2C_EV_IRQHandler(&hi2c4); /* → SlaveRxCpltCallback or SlaveTxCpltCallback */
  /* USER CODE BEGIN I2C4_EV_IRQn 1 */

  /* USER CODE END I2C4_EV_IRQn 1 */
}

/**
  * @brief This function handles I2C4 error interrupt.
  * @note  I2C4 is configured as SLAVE (address: 0x02)
  * @note  Triggers: Bus error, ACK failure, overrun, timeout
  */
void I2C4_ER_IRQHandler(void) {
  /* USER CODE BEGIN I2C4_ER_IRQn 0 */
  /* Slave error: bus error, overrun, etc. */
  /* USER CODE END I2C4_ER_IRQn 0 */
  HAL_I2C_ER_IRQHandler(&hi2c4); /* → HAL_I2C_ErrorCallback if implemented */
  /* USER CODE BEGIN I2C4_ER_IRQn 1 */

  /* USER CODE END I2C4_ER_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
