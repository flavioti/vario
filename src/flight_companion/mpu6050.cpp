#include "I2Cdev.h"
#include "MPU6050.h"
#include <flight_companion/config.hpp>
#include <flight_companion/queue.hpp>
#include "Wire.h"
#include <list>

MPU6050 accelgyro;

void setup_mpu6050()
{
    Wire.begin();
    Serial.println("MPU6050 Initializing");
    accelgyro.initialize();

    Serial.println("MPU6050 Testing device connections");
    if (accelgyro.testConnection())
    {
        accelgyro.setDMPEnabled(true);
        accelgyro.setTempSensorEnabled(false);

        Serial.println("MPU6050 connection status...............: OK");

        accelgyro.CalibrateAccel();
        accelgyro.CalibrateGyro();

        Serial.println("\nMPU6050 Calibration status...............: OK");
    }
    else
    {
        Serial.println("MPU6050 connection failed");
    }
}

std::list<float> delta_z_long_tail = {0};
int16_t az_old;

void loop_mpu6050()
{
    if (!xQueueIsQueueFullFromISR(xQueueBuzzerDeltaAZ))
    {
        float az_sample = accelgyro.getAccelerationZ();

        // Calcula a variação do eixo Z (delta)
        if (delta_z_long_tail.size() >= 3)
        {
            delta_z_long_tail.pop_front();
        }
        delta_z_long_tail.push_back(az_sample);
        float az_now = std::accumulate(delta_z_long_tail.begin(), delta_z_long_tail.end(), 0.0) / delta_z_long_tail.size();

        float delta_z = 0;
        if (az_now > az_old)
            delta_z = az_now - az_old;
        else
            delta_z = az_old - az_now;
        az_old = az_now;

        struct ENCODER_Motion *ENCODER_MOTION_NOW;
        ENCODER_MOTION_NOW = (struct ENCODER_Motion *)malloc(sizeof(struct ENCODER_Motion));

        ENCODER_MOTION_NOW->temp = accelgyro.getTemperature() / 340.00 + 36.53;
        ENCODER_MOTION_NOW->ax = accelgyro.getAccelerationX();
        ENCODER_MOTION_NOW->ay = accelgyro.getAccelerationY();
        ENCODER_MOTION_NOW->az = az_sample;
        ENCODER_MOTION_NOW->gx = accelgyro.getRotationX();
        ENCODER_MOTION_NOW->gy = accelgyro.getRotationY();
        ENCODER_MOTION_NOW->gz = accelgyro.getRotationZ();
        ENCODER_MOTION_NOW->delta_z = delta_z;

        xQueueSend(xQueueMPU6050Metrics, &ENCODER_MOTION_NOW, 0);

        if (delta_z <= MIN_ACCEL_Z or delta_z >= MAX_ACCEL_Z)
            xQueueSend(xQueueBuzzerDeltaAZ, &delta_z, 0);

        delete ENCODER_MOTION_NOW;
    }
}
