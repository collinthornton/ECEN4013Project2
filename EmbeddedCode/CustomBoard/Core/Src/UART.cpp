/*
 * UART.cpp
 *
 *  Created on: Nov 4, 2021
 *      Author: cthornton
 */

#include "UART.h"
#include <algorithm>    // std::find
#include "stm32l1xx_hal.h"
#include <string.h>


std::map<USART_TypeDef*, UART*> UART::objectMap = std::map<USART_TypeDef*, UART*>();


UART::UART(UART_HandleTypeDef *handle) {
	this->handle = handle;
	lastTime = 0;
	uartBuff = NULL;
}

UART::~UART() {
	if (uartBuff != NULL) free(uartBuff);

	HAL_UART_AbortReceive_IT(handle);

	std::map<USART_TypeDef*, UART*>::iterator it;
	it = objectMap.find(handle->Instance);
	if (it != objectMap.end())
		objectMap.erase(it);
}

int UART::init(USART_TypeDef *port, int baud, int packetSize_Bytes) {
	// TODO Auto-generated constructor stub
	this->packetSize_Bytes = packetSize_Bytes;

	uartBuff = (uint8_t*)calloc(this->packetSize_Bytes+1, sizeof(uint8_t));

	handle->Instance = port;
	handle->Init.BaudRate = baud;
	handle->Init.WordLength = UART_WORDLENGTH_8B;
	handle->Init.StopBits = UART_STOPBITS_1;
	handle->Init.Parity = UART_PARITY_NONE;
	handle->Init.Mode = UART_MODE_TX_RX;
	handle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	handle->Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(handle) != HAL_OK)
	{
		return -1;
	}


	objectMap.insert(std::pair<USART_TypeDef*, UART*>(handle->Instance, this));
	HAL_UART_Receive_IT(handle, uartBuff, this->packetSize_Bytes);
	return 0;
}

bool UART::hasData() {
	HAL_UART_AbortReceive_IT(handle);
	bool tmp = dataReady;
	HAL_UART_Receive_IT(handle, uartBuff, packetSize_Bytes);
	return tmp;
}

int UART::getData(uint8_t *buff, int length) {
	HAL_UART_AbortReceive_IT(handle);

	memset(buff, '\0', length);
	int len = msgBuff.size();
	std::copy(msgBuff.begin(), msgBuff.end(), buff);
	msgBuff.clear();
	dataReady = false;

	HAL_UART_Receive_IT(handle, uartBuff, packetSize_Bytes);
	return len;
}

int UART::readLine(uint8_t *buff, int length, uint32_t minDelay) {
	if(HAL_UART_AbortReceive_IT(handle) != HAL_OK) {
		__NOP();
	}
	memset(buff, '\0', length);

	std::size_t pos = msgBuff.find('\n');
	if(pos != std::string::npos) {
		const char* tmp = msgBuff.substr(0,pos+1).c_str();
		const char* tmp1 = msgBuff.c_str();
		strcpy((char*)buff, msgBuff.substr(0, pos+1).c_str());
		msgBuff.clear();
		if(HAL_UART_Receive_IT(handle, uartBuff, packetSize_Bytes) != HAL_OK) {
			__NOP();
		}
		return pos+1;
	}
	if(HAL_UART_Receive_IT(handle, uartBuff, packetSize_Bytes) != HAL_OK) {
		__NOP();
	}
	return 0;
}

short UART::sendData(uint8_t *data, int numBytes, int timeout) {
	HAL_UART_Transmit(handle, data, numBytes, timeout);

	return 0;
}

void UART::memberIRQ() {
	dataReady = true;

	msgBuff += (char*)uartBuff;
	while(msgBuff.size() > 1024) msgBuff.erase(0, msgBuff.size()-1024);

	lastTime = HAL_GetTick();
	HAL_UART_Receive_IT(handle, uartBuff, packetSize_Bytes);
}




