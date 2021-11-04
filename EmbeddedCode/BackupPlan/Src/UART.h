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
#include <cstring>


class UART {
public:
	UART();
	int init(USART_TypeDef *port, int baud, int packetSize_Bytes);
	bool hasData();
	short sendData(uint8_t *data, int timeout=10);
	uint8_t* getData();
	void memberIRQ();
	virtual ~UART();

	static std::map<UART_HandleTypeDef*, UART*> objectMap;

private:
	uint8_t *buff, *safeBuff;
	UART_HandleTypeDef handle;
	int packetSize_Bytes;
	bool dataReady;
};


#endif /* UART_H_ */
