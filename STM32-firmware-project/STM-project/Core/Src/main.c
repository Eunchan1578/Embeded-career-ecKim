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
#include <stdio.h>
#include <string.h>
#include "gyro.h"
#include "dht.h"
#include "string.h"
#include "stdio.h"
#include "esp.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define TRUE  1
#define FALSE 0
#define LED_PORT LD2_GPIO_Port
#define LED_PIN LD2_Pin
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

#define ARR_CNT 5
#define CMD_SIZE 50
volatile uint8_t serialBuf[300];
volatile DHT11_TypeDef dht11Data;
volatile int DTH11Flag;
volatile int DTH11Cnt;
volatile int motorFlag;
volatile int motorCnt;
volatile int getFoodFlag=0;
volatile int foodTemp;
volatile int pulse=1400-1;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim11;
/* USER CODE BEGIN PV */
uint8_t rx2char;
extern cb_data_t cb_data;
extern volatile unsigned char rx2Flag;
extern volatile char rx2Data[50];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM11_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
char strBuff[MAX_ESP_COMMAND_LEN];
void MX_GPIO_LED_ON(int flag);
void MX_GPIO_LED_OFF(int flag);
void esp_event(char *);
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
  int ret = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  uint8_t check;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM11_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  ret |= drv_uart_init();
  ret |= drv_esp_init();
  if(ret != 0) Error_Handler();
  AiotClient_Init();
  DHT11_Init();
  dht11Data = DHT11_readData();
  HAL_Delay(1000);
  if (MPU_begin(&hi2c1, AD0_LOW, AFSR_4G, GFSR_2000DPS, 0.98, 0.004) == TRUE)
    {
      HAL_GPIO_WritePin(LED_PORT, LED_PIN, 1);
      HAL_I2C_Mem_Read(&hi2c1, (AD0_LOW<<1), WHO_AM_I, 1, &check, 1, I2C_TIMOUT_MS);
      if (check == WHO_AM_I_9250_ANS)
      {
    	  sprintf((char *)serialBuf, "MPU_9250\r\n");
      }
      else if(check == WHO_AM_I_6050_ANS)
      {
    	  sprintf((char *)serialBuf, "MPU_6250\r\n");
      }
      drv_uart_tx_buffer(serialBuf,strlen((char *)serialBuf));
    }
    else
    {
      sprintf((char *)serialBuf, "ERROR!\r\n");
      drv_uart_tx_buffer(serialBuf,strlen((char *)serialBuf));
      while (1)
      {
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        HAL_Delay(500);
      }
    }

    // Calibrate the IMU
    sprintf((char *)serialBuf, "CALIBRATING...\r\n");
    drv_uart_tx_buffer(serialBuf,strlen((char *)serialBuf));
    MPU_calibrateGyro(&hi2c1, 1500);
    // Start timer and put processor into an efficient low power mode

    if(HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1) != HAL_OK)
    {
  	  Error_Handler();
    }
    printf("ecKim_Start_PWM1\r\n");
    if(HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2) != HAL_OK)
    {
  	  Error_Handler();
    }
    printf("ecKim_Start_PWM2\r\n");
    __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_1,1400-1);
    HAL_TIM_Base_Start_IT(&htim11);
    MX_GPIO_LED_OFF(LD2_Pin);
    sprintf((char *)serialBuf, "[ALLMSG]h_%d%%@t_%d.%d'C\r\n", \
    				dht11Data.rh_byte1, dht11Data.temp_byte1, dht11Data.temp_byte2);
    drv_uart_tx_buffer(serialBuf,strlen((char *)serialBuf));
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(DTH11Flag==1)
	  {
		__HAL_UART_DISABLE_IT(&huart6, UART_IT_RXNE);
		__HAL_UART_DISABLE_IT(&huart2, UART_IT_RXNE);
		HAL_TIM_Base_Stop_IT(&htim11);
		dht11Data = DHT11_readData();
		DTH11Flag=0;
		sprintf((char *)serialBuf, "[ALLMSG]h_%d%%@t_%d.%d'C\r\n", \
				dht11Data.rh_byte1, dht11Data.temp_byte1, dht11Data.temp_byte2);
		drv_uart_tx_buffer(serialBuf,strlen((char *)serialBuf));
		if(getFoodFlag==1)
		{
			if(dht11Data.temp_byte1>foodTemp)
				__HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_2,20000-1);
			else
				__HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_2,1-1);
			if(dht11Data.temp_byte1<foodTemp)
				MX_GPIO_LED_ON(LD2_Pin);
			else
				MX_GPIO_LED_OFF(LD2_Pin);
		}
		HAL_TIM_Base_Start_IT(&htim11);
		UART_Clear_RXNE(&huart6);
		UART_Clear_RXNE(&huart2);
		__HAL_UART_ENABLE_IT(&huart6, UART_IT_RXNE);
		__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
	  }
	  else
	  {
		if(strstr((char *)cb_data.buf,"+IPD") && cb_data.buf[cb_data.length-1] == '\n')
		{
			HAL_TIM_Base_Stop_IT(&htim11);
			strcpy(strBuff,strchr((char *)cb_data.buf,'['));
			memset(cb_data.buf,0x0,sizeof(cb_data.buf));
			cb_data.length = 0;
			esp_event(strBuff);
			HAL_TIM_Base_Start_IT(&htim11);
		}
		if(rx2Flag)
		{
			HAL_TIM_Base_Stop_IT(&htim11);
			sprintf((char *)serialBuf,"[ALLMSG]%s\r\n",(char *)rx2Data);
			esp_send_data(serialBuf);
			rx2Flag =0;
			printf("recv2 : %s\r\n",rx2Data);
			HAL_TIM_Base_Start_IT(&htim11);
		}
		if(motorFlag)
		{
			pulse=pulse+11*(int)attitude.p;
		    __HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_1,pulse);
//			printf("degree : %d\r\n",(int)attitude.p);
		    motorFlag=0;
		}
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
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 20000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 84-1;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 1000-1;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */


/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */


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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);

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

  /*Configure GPIO pin : DHT11_Pin */
  GPIO_InitStruct.Pin = DHT11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void MX_GPIO_LED_ON(int pin)
{
	HAL_GPIO_WritePin(LD2_GPIO_Port, pin, GPIO_PIN_SET);
}
void MX_GPIO_LED_OFF(int pin)
{
	HAL_GPIO_WritePin(LD2_GPIO_Port, pin, GPIO_PIN_RESET);
}

void esp_event(char * recvBuf)
{
  int i=0;
  char * pToken;
  char * pArray[ARR_CNT]={0};
  char sendBuf[MAX_UART_COMMAND_LEN]={0};

	strBuff[strlen(recvBuf)-1] = '\0';	//'\n' cut
  printf("\r\nDebug recv : %s\r\n",recvBuf);

  pToken = strtok(recvBuf,"[@]");
  while(pToken != NULL)
  {
    pArray[i] = pToken;
    if(++i >= ARR_CNT)
      break;
    pToken = strtok(NULL,"[@]");
  }

  if(!strcmp(pArray[1],"LED"))
  {
  	if(!strcmp(pArray[2],"ON"))
  	{
  		MX_GPIO_LED_ON(LD2_Pin);

  	}
		else if(!strcmp(pArray[2],"OFF"))
		{
				MX_GPIO_LED_OFF(LD2_Pin);
		}
		sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
  }
  else if(!strncmp(pArray[1]," New conn",8))
  {
     return;
  }
  else if(!strncmp(pArray[1]," Already log",8))
  {
			esp_client_conn();
      return;
  }
  else if(!strcmp(pArray[1],"FOOD"))
  {
	  foodTemp=atoi(pArray[2]);
	  printf("%d\r\n",foodTemp);
	  getFoodFlag=1;
	  sprintf(sendBuf,"[%s]%s\r\n","ALLMSG","YES");
	  esp_send_data(sendBuf);
      return;
  }
  else if(!strcmp(pArray[1],"SENSOR"))
    {
	  sprintf((char *)sendBuf, "[MGJ_BT]SENSOR@%d@%d\r\n", \
						dht11Data.rh_byte1, dht11Data.temp_byte1);
	  printf("%s\r\n",sendBuf);
  	  esp_send_data(sendBuf);
        return;
    }
  else
      return;

//  esp_send_data(sendBuf);
  printf("Debug send : %s\r\n",sendBuf);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim11)
  {
	 DTH11Cnt++;
	 if(DTH11Cnt==20000)
	 {
		 DTH11Flag=1;
		 DTH11Cnt=0;
	 }
	 motorCnt++;
	 if(motorCnt==500)
	 {
		 motorFlag=1;
		 motorCnt=0;
	 }

    MPU_calcAttitude(&hi2c1);
  }
}
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

#ifdef  USE_FULL_ASSERT
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
