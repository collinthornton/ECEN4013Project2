/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <main.h>
#include "fatfs.h"
#include "usb_device.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include "UART.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdarg.h"
#include <math.h>
#include <mpu6050.h>

#define SD_TIMER_HANDLE htim6
#define BLE_TIMER_HANDLE htim7
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern Disk_drvTypeDef  disk;

uint8_t usbBuffer[16];

enum SDSTATE {
	DISCONNECTED,
	CONNECTED,
	MOUNTED,
	FILE_OPEN
};
SDSTATE sdState;
volatile uint8_t sdCount=0, sdOverlap=15;

typedef struct {
	uint16_t UTC_Hour;
	uint16_t UTC_Min;
	uint16_t UTC_Sec;
	uint16_t UTC_MicroSec;

	float	Latitude;
	double  LatitudeDecimal;
	char	NS_Indicator;

	float	Longitude;
	double 	LongitudeDecimal;
	char	EW_Indicator;

	uint16_t	PositionFixIndicator;
	uint16_t	SatellitesUsed;
	float	HDOP;
	float	MSL_Altitude;
	char	MSL_Units;
	float	Geoid_Separation;
	char	Geoid_Units;

	uint16_t	AgeofDiffCorr;
	char	DiffRefStationID[4];
	char	CheckSum[2];


} GPGGA;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
UART gps(&huart1), ble(&huart2);
MPU6050_t mpu6050;

uint32_t adcValue[2];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
/* USER CODE BEGIN PFP */
double convertDegMinToDecDeg (float degMin);
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
  GPGGA gpsData;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  MX_DMA_Init();
  MX_USB_DEVICE_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc, adcValue, 2);

  // Setup UART devices at 9600 baud
  ble.init(USART2, 9600, 1);
  gps.init(USART1, 9600, 1);

  MPU6050_Init(&hi2c1);

  memset(&gpsData,0,sizeof(gpsData));
  gpsData.LatitudeDecimal = 0.0;
  gpsData.LongitudeDecimal = 0.0;
  if(gpsData.NS_Indicator==0)
	  gpsData.NS_Indicator='-';
  if(gpsData.EW_Indicator==0)
	  gpsData.EW_Indicator='-';
  if(gpsData.Geoid_Units==0)
	  gpsData.Geoid_Units='-';
  if(gpsData.MSL_Units==0)
	  gpsData.MSL_Units='-';

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t buff[1024] = {0};
  char msg[100] = {0};

  uint32_t elapsedTime = 0;
  uint32_t prevTime = HAL_GetTick();

  HAL_GPIO_WritePin(BLE_EN_GPIO_Port, BLE_EN_Pin, GPIO_PIN_SET);

  HAL_Delay(1000);


  FATFS FatFS;

  FIL fil;
  FRESULT fres;


  // Check SD card detect pin and attemp to mount/dismount file system accordingly
  GPIO_PinState sdValue = HAL_GPIO_ReadPin(SD_DETECT_GPIO_Port, SD_DETECT_Pin);

  if(sdValue == GPIO_PIN_SET) {
	 if(sdState == FILE_OPEN) {
		 f_close(&fil);
		 sdState = MOUNTED;
	 }
	 if(sdState == MOUNTED) {
		 f_mount(NULL, "", 1);
	 }

	 disk.is_initialized[0] = 0;
	 sdState = DISCONNECTED;
	}
	else if(sdValue == GPIO_PIN_RESET) {
	  fres = f_mount(&FatFS, "", 1);
	  if(fres != FR_OK) {
		  sprintf((char*)buff, "f_mount error (%i)\r\n", fres);
		  CDC_Transmit_FS(buff, strlen((char*)buff));
		  ble.sendData(buff, strlen((char*)buff));
		  sdState = CONNECTED;
	  }
	  else {
		  sdState = MOUNTED;
	  }
  }


  // Setup timer IRQ
  HAL_TIM_Base_Start_IT(&SD_TIMER_HANDLE);
  HAL_TIM_Base_Start_IT(&BLE_TIMER_HANDLE);


  // Make CSV header
  sprintf((char*)buff,
    "UTC Time, Loop Time, Lat (Deg), NS, Long. (Deg), EW, MSL Alt., Units, Sats., ax (m/s^2), ay (m/s^2), az (m/s^2), gx (deg/s), gy (deg/s), gz (deg/s)\r\n"
  );

  // Transmit CSV header over USB and BLE
  CDC_Transmit_FS(buff, strlen((char*)buff));
  ble.sendData(buff, strlen((char*)buff), 50000);


  // Attempt to open file and write CSV header to SD card
  if(sdState == MOUNTED) {
	  fres = f_open(&fil, "log.csv", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
	  if(fres != FR_OK) {
		  sprintf((char*)buff, "f_open error (%i)\r\n", fres);
		  CDC_Transmit_FS(buff, strlen((char*)buff));
		  ble.sendData(buff, strlen((char*)buff));
	  }
	  else {
		  sdState = FILE_OPEN;
		  UINT bytesWrote;

		  if(f_write(&fil, buff, strlen((char*)buff), &bytesWrote) != FR_OK) {
			  sprintf((char*)buff, "f_write error (%i)\r\n", fres);
			  CDC_Transmit_FS(buff, strlen((char*)buff));
			  ble.sendData(buff, strlen((char*)buff));
			  //while(1);
		  }
		  	  f_close(&fil);
		  	  sdState = MOUNTED;
	  }
  }


  while (1)
  {

	  // Get a line from the GPS buffer if available
	  if(gps.readLine(buff, sizeof(buff))) {

		  // Check for the GNGGA message
		  char *str = strstr((char*)buff, "$GNGGA,");
		  if(str!=NULL)
		  {
			  // Parse if available
			  memset(&gpsData,0,sizeof(gpsData));
			  sscanf(str,"$GNGGA,%2hu%2hu%2hu.%3hu,%f,%c,%f,%c,%hu,%hu,%f,%f,%c,%f,%c,,*%2s",
					  &gpsData.UTC_Hour,&gpsData.UTC_Min,&gpsData.UTC_Sec,&gpsData.UTC_MicroSec,&gpsData.Latitude,
					  &gpsData.NS_Indicator,&gpsData.Longitude,&gpsData.EW_Indicator,&gpsData.PositionFixIndicator,
					  &gpsData.SatellitesUsed,&gpsData.HDOP,&gpsData.MSL_Altitude,&gpsData.MSL_Units,&gpsData.Geoid_Separation,
					  &gpsData.Geoid_Units,gpsData.CheckSum);

			  if(gpsData.NS_Indicator==0)
				  gpsData.NS_Indicator='-';
			  if(gpsData.EW_Indicator==0)
				  gpsData.EW_Indicator='-';
			  if(gpsData.Geoid_Units==0)
				  gpsData.Geoid_Units='-';
			  if(gpsData.MSL_Units==0)
				  gpsData.MSL_Units='-';

			  gpsData.LatitudeDecimal=convertDegMinToDecDeg(gpsData.Latitude);
			  gpsData.LongitudeDecimal=convertDegMinToDecDeg(gpsData.Longitude);

			  // Set status LED
			  if(gpsData.SatellitesUsed > 3) HAL_GPIO_WritePin(GPS_LED_G_GPIO_Port, GPS_LED_G_Pin, GPIO_PIN_SET);
			  else HAL_GPIO_WritePin(GPS_LED_G_GPIO_Port, GPS_LED_G_Pin, GPIO_PIN_RESET);

			  memset(buff, '\0', sizeof(buff));
		  }
	  }


	 // Check value of SD chip detect pin and mount/dismount file system as necessary
	 GPIO_PinState sdValue = HAL_GPIO_ReadPin(SD_DETECT_GPIO_Port, SD_DETECT_Pin);

	 if(sdValue == GPIO_PIN_SET) {
		 if(sdState == FILE_OPEN) {
			 f_close(&fil);
			 sdState = MOUNTED;
		 }
		 if(sdState == MOUNTED) {
			 f_mount(NULL, "", 1);
		 }

		 disk.is_initialized[0] = 0;
		 sdState = DISCONNECTED;
	 }
	 else if(sdValue == GPIO_PIN_RESET && sdState == DISCONNECTED) {

		  fres = f_mount(&FatFS, "", 1);
		  if(fres != FR_OK) {
			  sprintf((char*)buff, "f_mount error (%i)\r\n", fres);
			  CDC_Transmit_FS(buff, strlen((char*)buff));
			  ble.sendData(buff, strlen((char*)buff));
			  sdState = CONNECTED;
		  }
		  else {
			  sdState = MOUNTED;
		  }
	 }


	  // Update IMU
	  MPU6050_Read_All(&hi2c1, &mpu6050);


	  // Format output string
	  sprintf((char*)buff,
			  "%hu:%hu:%hu,%ld,%.5f,%c,%.5f,%c,%.1f,%c,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\r\n",

			  gpsData.UTC_Hour, gpsData.UTC_Min, gpsData.UTC_Sec, elapsedTime,
			  gpsData.LatitudeDecimal, gpsData.NS_Indicator, gpsData.LongitudeDecimal, gpsData.EW_Indicator,
			  gpsData.MSL_Altitude, gpsData.MSL_Units, gpsData.SatellitesUsed,
	 	mpu6050.Ax, mpu6050.Ay, mpu6050.Az,
		mpu6050.Gx, mpu6050.Gy, mpu6050.Gz
	  );


	  // Transmit output string via USB and BLE
	  CDC_Transmit_FS(buff, strlen((char*)buff));
	  ble.sendData(buff, strlen((char*)buff), 50000);


	  // Attempt to open file and write output string to SD card
	  if(sdState == MOUNTED) {
		  fres = f_open(&fil, "log.csv", FA_OPEN_APPEND | FA_WRITE | FA_READ);
		  if(fres != FR_OK) {
			  sprintf((char*)buff, "f_open error (%i)\r\n", fres);
			  CDC_Transmit_FS(buff, strlen((char*)buff));
			  ble.sendData(buff, strlen((char*)buff));
		  }
		  else {
			  sdState = FILE_OPEN;

			  UINT bytesWrote;
			  fres = f_write(&fil, buff, strlen((char*)buff), &bytesWrote);
			  if(fres != FR_OK) {
				  sprintf((char*)buff, "f_write error (%i)\r\n", fres);
				  CDC_Transmit_FS(buff, strlen((char*)buff));
				  ble.sendData(buff, strlen((char*)buff));
			  }
			  f_close(&fil);

			  sdState = MOUNTED;
		  }
	  }

	  // Compute elapsed time
	  elapsedTime = HAL_GetTick() - prevTime;
	  prevTime = HAL_GetTick();

	  HAL_Delay(900);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  f_mount(NULL, "", 0);
  /* USER CODE END 3 */
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM11) {
    	HAL_IncTick();
    }
    else if(htim->Instance == BLE_TIMER_HANDLE.Instance) {
    	GPIO_PinState bleState = HAL_GPIO_ReadPin(BLE_STATUS_GPIO_Port, BLE_STATUS_Pin);
		if(bleState == GPIO_PIN_RESET) {
			HAL_GPIO_TogglePin(BLE_LED_G_GPIO_Port, BLE_LED_G_Pin);
		}
		else {
			HAL_GPIO_WritePin(BLE_LED_G_GPIO_Port, BLE_LED_G_Pin, GPIO_PIN_RESET);
		}
	}
	else if(htim->Instance == SD_TIMER_HANDLE.Instance) {
		switch(sdState) {
		case DISCONNECTED:
			HAL_GPIO_WritePin(SD_LED_G_GPIO_Port, SD_LED_G_Pin, GPIO_PIN_RESET);
			sdCount = 0;
			break;

		case CONNECTED:
			HAL_GPIO_WritePin(SD_LED_G_GPIO_Port, SD_LED_G_Pin, GPIO_PIN_SET);
			sdCount = 0;
			break;

		case MOUNTED:
			if(++sdCount >= sdOverlap) {
				HAL_GPIO_TogglePin(SD_LED_G_GPIO_Port, SD_LED_G_Pin);
				sdCount = 0;
			}
			break;

		case FILE_OPEN:
			HAL_GPIO_TogglePin(SD_LED_G_GPIO_Port, SD_LED_G_Pin);
			sdCount = 0;
			break;
		}
	}
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc.Init.LowPowerAutoWait = ADC_AUTOWAIT_DISABLE;
  hadc.Init.LowPowerAutoPowerOff = ADC_AUTOPOWEROFF_DISABLE;
  hadc.Init.ChannelsBank = ADC_CHANNELS_BANK_A;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.NbrOfConversion = 2;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = ENABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_4CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

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
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}


/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 20;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 65535;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 120;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 65535;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}
/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, USB_LED_G_Pin|BLE_LED_G_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, PWR_LED_G_Pin|PWR_LED_R_Pin|GPS_LED_G_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SPI_SD_CS_Pin|MLX_TRIG_Pin|SD_LED_G_Pin|BLE_STATUS_Pin
                          |BLE_EN_Pin|PWR_LED_B_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : USB_LED_G_Pin BLE_LED_G_Pin */
  GPIO_InitStruct.Pin = USB_LED_G_Pin|BLE_LED_G_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PWR_LED_G_Pin PWR_LED_R_Pin GPS_LED_G_Pin */
  GPIO_InitStruct.Pin = PWR_LED_G_Pin|PWR_LED_R_Pin|GPS_LED_G_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : V_BAT_P_Pin */
  GPIO_InitStruct.Pin = V_BAT_P_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(V_BAT_P_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI_SD_CS_Pin MLX_TRIG_Pin SD_LED_G_Pin BLE_STATUS_Pin
                           BLE_EN_Pin PWR_LED_B_Pin */
  GPIO_InitStruct.Pin = SPI_SD_CS_Pin|MLX_TRIG_Pin|SD_LED_G_Pin
                          |BLE_EN_Pin|PWR_LED_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_DETECT_Pin */
  GPIO_InitStruct.Pin = SD_DETECT_Pin|BLE_STATUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : MLX_RDY_Pin BLE_INT_Pin */
  GPIO_InitStruct.Pin = MLX_RDY_Pin|BLE_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	std::map<USART_TypeDef*, UART*>::iterator it;
	it = UART::objectMap.find(huart->Instance);
	if(it != UART::objectMap.end())
		it->second->memberIRQ();
}

double convertDegMinToDecDeg (float degMin)
{
  double min = 0.0;
  double decDeg = 0.0;

  //get the minutes, fmod() requires double
  min = fmod((double)degMin, 100.0);

  //rebuild coordinates in decimal degrees
  degMin = (int) ( degMin / 100 );
  decDeg = degMin + ( min / 60 );

  return decDeg;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
