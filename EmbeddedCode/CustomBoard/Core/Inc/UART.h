/*
 * UART.h
 *
 *  Created on: Nov 4, 2021
 *      Author: cthornton
 */

#ifndef UART_H_
#define UART_H_

#include "stm32l1xx_hal.h"

#include <map>
#include <queue>
#include <cstring>


class UART {
public:
	UART(UART_HandleTypeDef *handle);
	int init(USART_TypeDef *port, int baud, int packetSize_Bytes);
	bool hasData();
	short sendData(uint8_t *data, int numBytes, int timeout=10);
	int getData(uint8_t *buff, int length);
	int readLine(uint8_t *buff, int length, uint32_t minDelay=0);
	void memberIRQ();
	virtual ~UART();

	UART_HandleTypeDef *handle;
	static std::map<USART_TypeDef*, UART*> objectMap;

private:
	std::string msgBuff;
	uint8_t *uartBuff;
	int packetSize_Bytes;
	uint32_t lastTime;
	bool dataReady;
};


#endif /* UART_H_ */
