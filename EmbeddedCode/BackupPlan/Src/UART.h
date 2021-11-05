/*
 * UART.h
 *
 *  Created on: Nov 4, 2021
 *      Author: cthornton
 */

#ifndef UART_H_
#define UART_H_

#include "stm32f7xx_hal.h"

#include <map>
#include <vector>
#include <cstring>


class UART {
public:
	UART(UART_HandleTypeDef *handle);
	int init(USART_TypeDef *port, int baud, int packetSize_Bytes);
	bool hasData();
	short sendData(uint8_t *data, int numBytes, int timeout=10);
	int getData(uint8_t *buff);
	void memberIRQ();
	virtual ~UART();

	UART_HandleTypeDef *handle;
	static std::map<USART_TypeDef*, UART*> objectMap;

private:
	std::vector<uint8_t> msgBuff;
	uint8_t *uartBuff;
	int packetSize_Bytes;
	bool dataReady;
};


#endif /* UART_H_ */
