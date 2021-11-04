/*
 * UART.h
 *
 *  Created on: Nov 4, 2021
 *      Author: cthornton
 */

#ifndef UART_H_
#define UART_H_

class UART {
public:
	UART();
	bool isData();
	short sendData(uint8_t &data, timeout=10);
	uint8_t& getData();
	virtual ~UART();

private:


	uint8_t buff[1024];
	UART_HandleTypeDef handle;
	bool dataReady;
};

#endif /* UART_H_ */
