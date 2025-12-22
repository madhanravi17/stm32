/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
TIM_HandleTypeDef htim10;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM10_Init(void);
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
	//	uint16_t timer_val;
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
	MX_USART2_UART_Init();
	MX_TIM10_Init();
	/* USER CODE BEGIN 2 */

	HAL_TIM_Base_Start(&htim10);
	//	timer_val = __HAL_TIM_GET_COUNTER(&htim10);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */
		if (__HAL_TIM_GET_COUNTER(&htim10) == 0)
		{
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
			HAL_Delay(1E3);
			/*
			 when timer overflows to zero ,the duration is
			 only for 1ms so to visualize the trigger a delay
			 of 1s is used
			 Time  = period / timer clock frequency , where timer clock frequency  = 84 Mhz /  (prescaler = 84000 = 10 khz?
			 ChatGPT said:
			 ChatGPT
			 Yes, that's correct! Here's how the formula works:

			 Formula:
			 Time (Overflow Period)
			 =
			 Period
			 Timer Clock Frequency
			 Time (Overflow Period)=
			 Timer Clock Frequency
			 Period
			 ​


			 Where:

			 Timer Clock Frequency is the frequency at which the timer increments its counter.
			 Period is the value up to which the timer counts before resetting and triggering an event.
			 Breaking it down:
			 Step 1: Calculate the Timer Clock Frequency
			 If the timer clock frequency (which is typically derived from the system clock, 84 MHz in this case) is divided by the prescaler, the new clock frequency for the timer is:

			 Timer Clock Frequency
			 =
			 84
			  MHz
			 Prescaler
			 Timer Clock Frequency=
			 Prescaler
			 84 MHz
			 ​

			 With the prescaler set to 84000, the calculation becomes:

			 Timer Clock Frequency
			 =
			 84
			  MHz
			 84000
			 =
			 1
			  kHz
			 Timer Clock Frequency=
			 84000
			 84 MHz
			 ​
			 =1 kHz
			 So, the timer increments its counter every 1 millisecond (since 1 kHz = 1000 Hz).

			 Step 2: Calculate the Overflow Time
			 Now, you can use the formula to calculate the time it takes for the timer to overflow:

			 Overflow Time
			 =
			 Period
			 Timer Clock Frequency
			 Overflow Time=
			 Timer Clock Frequency
			 Period
			 ​

			 For a period of 65535, and a timer clock frequency of 1 kHz (from step 1):

			 Overflow Time
			 =
			 65535
			 1000
			  Hz
			 =
			 65.535
			  seconds
			 Overflow Time=
			 1000 Hz
			 65535
			 ​
			 =65.535 seconds
			 Conclusion:
			 With a prescaler of 84000, your timer clock frequency is 1 kHz.
			 With a period of 65535, the timer will overflow approximately every 65.535 seconds.





			 You said:
			 if prescaler is 8400 then time of overflow would be 6.5535 seconds. To make it exactly one second of time overflow then prescaler should be 65535 to attain 1s
			 ChatGPT said:
			 ChatGPT
			 You're on the right track! Let's break it down step by step to clarify:

			 Given Information:
			 Timer Clock Frequency Calculation:
			 Timer Clock Frequency
			 =
			 System Clock Frequency
			 Prescaler
			 Timer Clock Frequency=
			 Prescaler
			 System Clock Frequency
			 ​

			 System Clock Frequency = 84 MHz (or 84,000,000 Hz)
			 Step 1: Calculate the Timer Clock Frequency with Different Prescaler Values
			 Case 1: Prescaler = 8400
			 Timer Clock Frequency
			 =
			 84
			 ,
			 000
			 ,
			 000
			  Hz
			 8400
			 =
			 10
			 ,
			 000
			  Hz
			 (
			 or 
			 10
			  kHz
			 )
			 Timer Clock Frequency=
			 8400
			 84,000,000 Hz
			 ​
			 =10,000 Hz(or 10 kHz)
			 Step 2: Calculate Overflow Time
			 For a period of 65535:

			 Overflow Time
			 =
			 Period
			 Timer Clock Frequency
			 =
			 65535
			 10
			 ,
			 000
			  Hz
			 =
			 6.5535
			  seconds
			 Overflow Time=
			 Timer Clock Frequency
			 Period
			 ​
			 =
			 10,000 Hz
			 65535
			 ​
			 =6.5535 seconds
			 Case 2: Make Overflow Time Exactly 1 Second
			 To achieve an overflow time of 1 second:

			 1
			  second
			 =
			 65535
			 Timer Clock Frequency
			 1 second=
			 Timer Clock Frequency
			 65535
			 ​

			 Thus, you rearrange to find the required Timer Clock Frequency:

			 Timer Clock Frequency
			 =
			 65535
			  Hz
			 Timer Clock Frequency=65535 Hz
			 Step 3: Calculate the Required Prescaler
			 Now, to find the required prescaler:

			 Prescaler
			 =
			 84
			 ,
			 000
			 ,
			 000
			  Hz
			 65535
			  Hz
			 ≈
			 1283.33
			 Prescaler=
			 65535 Hz
			 84,000,000 Hz
			 ​
			 ≈1283.33
			 Conclusion
			 If you want an overflow time of 6.5535 seconds, a prescaler of 8400 is correct.
			 To achieve an overflow time of 1 second, you'd need a prescaler of approximately 1283 (you'd typically round this value to the nearest integer, which would be 1283 or 1284 based on the exact needs of your application).
			 So, your observation is close, but the specific value of the prescaler needed for exactly 1 second of overflow is around 1283, not 65535. 65535 is the maximum period value, which would give you the longest possible overflow time depending on the timer clock frequency.
			 To determine whether setting the timer period to 60,000 and the prescaler to 1,400 will yield a timer frequency of 1 Hz, we need to understand how the timer works.

Timer Configuration
Clock Frequency: The timer frequency is derived from the timer's input clock frequency. Assuming a typical clock frequency of 84 MHz (common for STM32 microcontrollers), the timer frequency without any prescaler would be:

𝑓
𝑡
𝑖
𝑚
𝑒
𝑟
=
𝑓
𝑐
𝑙
𝑜
𝑐
𝑘
𝑃
𝑟
𝑒
𝑠
𝑐
𝑎
𝑙
𝑒
𝑟
f
timer
​
 =
Prescaler
f
clock
​

​

Setting the Prescaler: With a prescaler of 1,400:

𝑓
𝑡
𝑖
𝑚
𝑒
𝑟
=
84
,
000
,
000
 Hz
1
,
400
≈
60
,
000
 Hz
f
timer
​
 =
1,400
84,000,000 Hz
​
 ≈60,000 Hz
Setting the Period: The timer counts up to the period value. If you set the period to 60,000, the timer will overflow and trigger an interrupt or reset every 60,000 counts.

Calculating the Timer Frequency
To find the output frequency, we can use the following formula:

𝑓
𝑜
𝑢
𝑡
𝑝
𝑢
𝑡
=
𝑓
𝑡
𝑖
𝑚
𝑒
𝑟
𝑃
𝑒
𝑟
𝑖
𝑜
𝑑
f
output
​
 =
Period
f
timer
​

​

In this case:

𝑓
𝑜
𝑢
𝑡
𝑝
𝑢
𝑡
=
60
,
000
 Hz
60
,
000
=
1
 Hz
f
output
​
 =
60,000
60,000 Hz
​
 =1 Hz
Limitations of Timer
The STM32 timer can count up to 65,535 in 16-bit mode. Setting a period of 60,000 is valid since it is less than the maximum value.

Conclusion
Yes, it is possible to achieve a timer frequency of 1 Hz by setting the period to 60,000 and the prescaler to 1,400, as this configuration stays within the bounds of a 16-bit timer. The timer will successfully count up to 60,000 and overflow, generating an event at 1 Hz.*/
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		}
		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief TIM10 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM10_Init(void)
{

	/* USER CODE BEGIN TIM10_Init 0 */

	/* USER CODE END TIM10_Init 0 */

	/* USER CODE BEGIN TIM10_Init 1 */

	/* USER CODE END TIM10_Init 1 */
	htim10.Instance = TIM10;
	htim10.Init.Prescaler = 8400 - 1;
	htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim10.Init.Period = 65535;
	htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM10_Init 2 */

	/* USER CODE END TIM10_Init 2 */
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : LD2_Pin */
	GPIO_InitStruct.Pin = LD2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
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
