/*
 * IMU.h
 *
 *  Created on: Oct 6, 2021
 *      Author: cthornton
 */

#ifndef INC_IMU_H_
#define INC_IMU_H_

#include <Sensor.h>

class IMU : public Sensor {
public:
	IMU();
	virtual ~IMU();
};

#endif /* INC_IMU_H_ */
