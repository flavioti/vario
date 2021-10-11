#include "I2Cdev.h"
#include "MPU6050.h"
#include <config.hpp>
#include <queue.hpp>
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

        Serial.println("MPU6050 connection status...............: OK");
        Serial.printf("MPU6050 DMP enabled.....................: %i\n", accelgyro.getDMPEnabled());
        Serial.printf("MPU6050 temp sensor enabled.............: %i\n", accelgyro.getTempSensorEnabled());

        Serial.print("Calibrating gyro and accel ");
        accelgyro.CalibrateAccel(7);
        accelgyro.CalibrateGyro(7);
        Serial.println("\ndone!");

        accelgyro.PrintActiveOffsets();
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
        std::list<int16_t> ax_raw = {};
        std::list<int16_t> ay_raw = {};
        std::list<int16_t> az_raw = {};

        unsigned long initial_time = millis();
        while (millis() - initial_time < 10)
        {
            ax_raw.push_back(accelgyro.getAccelerationX());
            ay_raw.push_back(accelgyro.getAccelerationY());
            az_raw.push_back(accelgyro.getAccelerationZ());
            delay(1);
        }

        float ax_sample = std::accumulate(ax_raw.begin(), ax_raw.end(), 0.0) / ax_raw.size();
        float ay_sample = std::accumulate(ay_raw.begin(), ay_raw.end(), 0.0) / ay_raw.size();
        float az_sample = std::accumulate(az_raw.begin(), az_raw.end(), 0.0) / az_raw.size();

        // Calcula a variação do eixo Z (delta)
        if (delta_z_long_tail.size() >= 3)
        {
            delta_z_long_tail.pop_front();
        }
        delta_z_long_tail.push_back(az_sample);
        float az_now = std::accumulate(delta_z_long_tail.begin(), delta_z_long_tail.end(), 0.0) / delta_z_long_tail.size();

        float delta_z = 0;
        if (az_now > az_old)
        {
            delta_z = az_now - az_old;
        }
        else
        {
            delta_z = az_old - az_now;
        }
        az_old = az_now;

        struct ENCODER_Motion *ENCODER_MOTION_NOW;
        ENCODER_MOTION_NOW = (struct ENCODER_Motion *)malloc(sizeof(struct ENCODER_Motion));

        ENCODER_MOTION_NOW->temp = accelgyro.getTemperature() / 340.00 + 36.53;
        ENCODER_MOTION_NOW->ax = ax_sample;
        ENCODER_MOTION_NOW->ay = ay_sample;
        ENCODER_MOTION_NOW->az = az_sample;
        ENCODER_MOTION_NOW->gx = accelgyro.getRotationX();
        ENCODER_MOTION_NOW->gy = accelgyro.getRotationY();
        ENCODER_MOTION_NOW->gz = accelgyro.getRotationZ();
        ENCODER_MOTION_NOW->delta_z = delta_z;

        xQueueSend(xQueueMPU6050Metrics, &ENCODER_MOTION_NOW, 0);

        if (delta_z <= MIN_ACCEL_Z or delta_z >= MAX_ACCEL_Z)
        {
            // Serial.printf("MPU6050 Enviado: %f\n", delta_z);
            xQueueSend(xQueueBuzzerDeltaAZ, &delta_z, 0);
        }

        delete ENCODER_MOTION_NOW;
    }
}
