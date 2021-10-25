/*
 * MPU6050.h
 *
 *  Created on: Oct 6, 2021
 *      Author: cthornton
 */

#ifndef SRC_MPU6050_H_
#define SRC_MPU6050_H_

#include <IMU.h>

class MPU6050 : public IMU {
public:
	MPU6050();
	virtual ~MPU6050();
};

#endif /* SRC_MPU6050_H_ */
