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
    Serial.println("[MPU6050] accelgyro initializing");
    accelgyro.initialize();
    if (accelgyro.testConnection())
    {
        Serial.println("[MPU6050] connection status .: CONNECTED");
        accelgyro.setDMPEnabled(true);
        accelgyro.setTempSensorEnabled(true);
        Serial.println("[MPU6050] accel .............: calibrating");
        accelgyro.CalibrateAccel();
        Serial.println("\n[MPU6050] gyro ..............: calibrating");
        accelgyro.CalibrateGyro();
    }
    else
    {
        Serial.println("[MPU6050] connection status .: FAILED");
    }
}

std::list<float> delta_z_long_tail = {0};
int16_t az_old;

void accel_task(void *pvParameters)
{
    // Aguarda execução para aguadar leitura dos dispositivos
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    for (;;)
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
            // float az_now = std::accumulate(delta_z_long_tail.begin(), delta_z_long_tail.end(), 0.0) / delta_z_long_tail.size();

            // float delta_z = 0;
            // if (az_now > az_old)
            //     delta_z = az_now - az_old;
            // else
            //     delta_z = az_old - az_now;
            // az_old = az_now;
        }

        vTaskDelay(ACEL_READ_RATE / portTICK_PERIOD_MS);
    }
}
