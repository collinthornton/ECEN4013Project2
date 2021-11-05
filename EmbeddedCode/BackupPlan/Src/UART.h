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
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
#include <vector>
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of d0a4466 (uart working well)
#include <cstring>


class UART {
public:
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
	UART(UART_HandleTypeDef *handle);
=======
	UART();
>>>>>>> parent of d0a4466 (uart working well)
	int init(USART_TypeDef *port, int baud, int packetSize_Bytes);
	bool hasData();
	short sendData(uint8_t *data, int timeout=10);
	uint8_t* getData();
	void memberIRQ();
	virtual ~UART();

	UART_HandleTypeDef handle;
	static std::map<USART_TypeDef*, UART*> objectMap;

private:
<<<<<<< HEAD
	std::vector<uint8_t> msgBuff;
	uint8_t *uartBuff;
=======
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
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
<<<<<<< HEAD
<<<<<<< HEAD
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
>>>>>>> parent of 2d4b392 (prep for i2c gyro testing)
=======
	uint8_t *buff, *safeBuff;
>>>>>>> parent of d0a4466 (uart working well)
	int packetSize_Bytes;
	bool dataReady;
};


#endif /* UART_H_ */
