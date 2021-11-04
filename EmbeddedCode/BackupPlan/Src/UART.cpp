/*
 * UART.cpp
 *
 *  Created on: Nov 4, 2021
 *      Author: cthornton
 */

#include "UART.h"
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"

std::map<UART_HandleTypeDef*, UART*> UART::objectMap = std::map<UART_HandleTypeDef*, UART*>();


UART::UART() {
	buff = NULL;
	safeBuff = NULL;
}

UART::~UART() {
	if (buff != NULL) free(buff);
	if (safeBuff != NULL) free(buff);

	HAL_UART_AbortReceive_IT(&handle);

	std::map<UART_HandleTypeDef*, UART*>::iterator it;
	it = objectMap.find(&handle);
	if (it != objectMap.end())
		objectMap.erase(it);
}

int UART::init(USART_TypeDef *port, int baud, int packetSize_Bytes) {
	// TODO Auto-generated constructor stub
	this->packetSize_Bytes = packetSize_Bytes;

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
	{
		return -1;
	}

	objectMap.insert(std::pair<UART_HandleTypeDef*, UART*>(&handle, this));
	HAL_UART_Receive_IT(&handle, buff, this->packetSize_Bytes);
	return 0;
}

bool UART::hasData() {
	HAL_UART_AbortReceive_IT(&handle);
	bool tmp = dataReady;
	HAL_UART_Receive_IT(&handle, buff, this->packetSize_Bytes);
	return tmp;
}

uint8_t* UART::getData() {
	HAL_UART_AbortReceive_IT(&handle);
	std::memcpy(safeBuff, buff, (packetSize_Bytes+1)*sizeof(uint8_t));
	HAL_UART_Receive_IT(&handle, buff, packetSize_Bytes);
	return safeBuff;
}

short UART::sendData(uint8_t *data, int timeout) {
	HAL_UART_Transmit(&this->handle, data, sizeof(*data)/sizeof(data[0]), timeout);

	return 0;
}

void UART::memberIRQ() {
	dataReady = true;
	HAL_UART_Receive_IT(&handle, buff, this->packetSize_Bytes);
}




