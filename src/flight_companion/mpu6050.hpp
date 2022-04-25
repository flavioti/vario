#ifndef MPU6050_H_INCLUDED
#define MPU6050_H_INCLUDED

void setup_mpu6050(bool calibrate);

void accel_task(void *pvParameters);

#endif
