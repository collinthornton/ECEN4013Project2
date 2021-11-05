/*
 * UART.cpp
 *
 *  Created on: Nov 4, 2021
 *      Author: cthornton
 */

#include "UART.h"
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"

std::map<USART_TypeDef*, UART*> UART::objectMap = std::map<USART_TypeDef*, UART*>();


<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
UART::UART(UART_HandleTypeDef *handle) {
	this->handle = handle;
	uartBuff = NULL;
=======
UART::UART() {
	buff = NULL;
	safeBuff = NULL;
>>>>>>> parent of d0a4466 (uart working well)
}

UART::~UART() {
	if (buff != NULL) free(buff);
	if (safeBuff != NULL) free(buff);

	HAL_UART_AbortReceive_IT(&handle);

	std::map<USART_TypeDef*, UART*>::iterator it;
<<<<<<< HEAD
	it = objectMap.find(handle->Instance);
=======
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
UART::UART() {
	buff = NULL;
	safeBuff = NULL;
}

UART::~UART() {
	if (buff != NULL) free(buff);
	if (safeBuff != NULL) free(buff);

	HAL_UART_AbortReceive_IT(&handle);

	std::map<USART_TypeDef*, UART*>::iterator it;
	it = objectMap.find(handle.Instance);
<<<<<<< HEAD
<<<<<<< HEAD
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
	it = objectMap.find(handle.Instance);
>>>>>>> parent of d0a4466 (uart working well)
	if (it != objectMap.end())
		objectMap.erase(it);
}

int UART::init(USART_TypeDef *port, int baud, int packetSize_Bytes) {
	// TODO Auto-generated constructor stub
	this->packetSize_Bytes = packetSize_Bytes;

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
	uartBuff = (uint8_t*)calloc(this->packetSize_Bytes+1, sizeof(uint8_t));

	handle->Instance = port;
	handle->Init.BaudRate = baud;
	handle->Init.WordLength = UART_WORDLENGTH_8B;
	handle->Init.StopBits = UART_STOPBITS_1;
	handle->Init.Parity = UART_PARITY_NONE;
	handle->Init.Mode = UART_MODE_TX_RX;
	handle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	handle->Init.OverSampling = UART_OVERSAMPLING_16;
	handle->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	handle->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(handle) != HAL_OK)
=======
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of d0a4466 (uart working well)
	buff = (uint8_t*)calloc(this->packetSize_Bytes, sizeof(uint8_t));
	safeBuff = (uint8_t*)calloc(this->packetSize_Bytes, sizeof(uint8_t));

	handle.Instance = port;
	handle.Init.BaudRate = baud;
	handle.Init.WordLength = UART_WORDLENGTH_8B;
	handle.Init.StopBits = UART_STOPBITS_1;
	handle.Init.Parity = UART_PARITY_NONE;
	handle.Init.Mode = UART_MODE_TX_RX;
	handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	handle.Init.OverSampling = UART_OVERSAMPLING_16;
	handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	handle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&handle) != HAL_OK)
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of d0a4466 (uart working well)
	{
		return -1;
	}


<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
	objectMap.insert(std::pair<USART_TypeDef*, UART*>(handle->Instance, this));
	HAL_UART_Receive_IT(handle, uartBuff, this->packetSize_Bytes);
=======
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of d0a4466 (uart working well)
	if (handle.Instance == USART2) {
		HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
	}
	else if(handle.Instance == USART3) {
		HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART3_IRQn);
	}


	objectMap.insert(std::pair<USART_TypeDef*, UART*>(handle.Instance, this));
	HAL_UART_Receive_IT(&handle, buff, this->packetSize_Bytes);
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of d0a4466 (uart working well)
	return 0;
}

bool UART::hasData() {
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
	HAL_UART_AbortReceive_IT(handle);
=======
	HAL_UART_AbortReceive_IT(&handle);
>>>>>>> parent of d0a4466 (uart working well)
	bool tmp = dataReady;
	HAL_UART_Receive_IT(&handle, buff, packetSize_Bytes);
	return tmp;
}

uint8_t* UART::getData() {
	HAL_UART_AbortReceive_IT(&handle);
	std::memcpy(safeBuff, buff, (packetSize_Bytes)*sizeof(uint8_t));
	dataReady = false;
	HAL_UART_Receive_IT(&handle, buff, packetSize_Bytes);
	return safeBuff;
}

<<<<<<< HEAD
short UART::sendData(uint8_t *data, int numBytes, int timeout) {
	HAL_UART_Transmit(handle, data, numBytes, timeout);
=======
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
	HAL_UART_AbortReceive_IT(&handle);
	bool tmp = dataReady;
	HAL_UART_Receive_IT(&handle, buff, packetSize_Bytes);
	return tmp;
}

uint8_t* UART::getData() {
	HAL_UART_AbortReceive_IT(&handle);
	std::memcpy(safeBuff, buff, (packetSize_Bytes)*sizeof(uint8_t));
	dataReady = false;
	HAL_UART_Receive_IT(&handle, buff, packetSize_Bytes);
	return safeBuff;
}

short UART::sendData(uint8_t *data, int timeout) {
	HAL_UART_Transmit(&this->handle, data, packetSize_Bytes, timeout);
<<<<<<< HEAD
<<<<<<< HEAD
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
short UART::sendData(uint8_t *data, int timeout) {
	HAL_UART_Transmit(&this->handle, data, packetSize_Bytes, timeout);
>>>>>>> parent of d0a4466 (uart working well)

	return 0;
}

void UART::memberIRQ() {
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
	dataReady = true;

	for (int i=0; i<packetSize_Bytes; ++i) {
		msgBuff.push_back(uartBuff[i]);
	}

	HAL_UART_Receive_IT(handle, uartBuff, packetSize_Bytes);
=======
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of d0a4466 (uart working well)
	//dataReady = true;
	//std::memset(buff, 0, sizeof(buff));
	HAL_UART_Transmit(&handle, buff, packetSize_Bytes, 100);
	HAL_UART_Receive_IT(&handle, buff, packetSize_Bytes);
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of d0a4466 (uart working well)
}




