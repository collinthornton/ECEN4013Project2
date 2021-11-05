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
<<<<<<< HEAD
#include <vector>
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
#include <cstring>


class UART {
public:
<<<<<<< HEAD
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
=======
	UART();
	int init(USART_TypeDef *port, int baud, int packetSize_Bytes);
	bool hasData();
	short sendData(uint8_t *data, int timeout=10);
	uint8_t* getData();
	void memberIRQ();
	virtual ~UART();

	UART_HandleTypeDef handle;
	static std::map<USART_TypeDef*, UART*> objectMap;

private:
	uint8_t *buff, *safeBuff;
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
	int packetSize_Bytes;
	bool dataReady;
};


#endif /* UART_H_ */
