/*
 * UART.cpp
 *
 *  Created on: Nov 4, 2021
 *      Author: cthornton
 */

#include "UART.h"

UART::UART() {
	// TODO Auto-generated constructor stub
	buff = {0};

}

UART::~UART() {
	// TODO Auto-generated destructor stub
}

bool UART::isData() {
	return dataReady;
}

short UART::sendData(uint8_t &data, timeout=10) {
	HAL_UART_Transmit(&this->handle, data, sizeof(data)/sizeof(data[0]), timeout);
}
