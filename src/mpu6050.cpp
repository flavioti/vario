#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <config.hpp>
#include <queue.hpp>

Adafruit_MPU6050 mpu;

void setup_mpu6050()
{
    Serial.println("Adafruit MPU6050 test!");
    if (!mpu.begin())
    {
        Serial.println("MPU6050................................. FAILED");
    }
    else
    {
        Serial.println("MPU6050................................. OK");

        mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
        Serial.print("Accelerometer range set to: ");
        switch (mpu.getAccelerometerRange())
        {
        case MPU6050_RANGE_2_G:
            Serial.println("+-2G");
            break;
        case MPU6050_RANGE_4_G:
            Serial.println("+-4G");
            break;
        case MPU6050_RANGE_8_G:
            Serial.println("+-8G");
            break;
        case MPU6050_RANGE_16_G:
            Serial.println("+-16G");
            break;
        }
        mpu.setGyroRange(MPU6050_RANGE_250_DEG);
        Serial.print("Gyro range set to: ");
        switch (mpu.getGyroRange())
        {
        case MPU6050_RANGE_250_DEG:
            Serial.println("+- 250 deg/s");
            break;
        case MPU6050_RANGE_500_DEG:
            Serial.println("+- 500 deg/s");
            break;
        case MPU6050_RANGE_1000_DEG:
            Serial.println("+- 1000 deg/s");
            break;
        case MPU6050_RANGE_2000_DEG:
            Serial.println("+- 2000 deg/s");
            break;
        }

        mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
        Serial.print("Filter bandwidth set to: ");
        switch (mpu.getFilterBandwidth())
        {
        case MPU6050_BAND_260_HZ:
            Serial.println("260 Hz");
            break;
        case MPU6050_BAND_184_HZ:
            Serial.println("184 Hz");
            break;
        case MPU6050_BAND_94_HZ:
            Serial.println("94 Hz");
            break;
        case MPU6050_BAND_44_HZ:
            Serial.println("44 Hz");
            break;
        case MPU6050_BAND_21_HZ:
            Serial.println("21 Hz");
            break;
        case MPU6050_BAND_10_HZ:
            Serial.println("10 Hz");
            break;
        case MPU6050_BAND_5_HZ:
            Serial.println("5 Hz");
            break;
        }
    }
}

void loop_mpu6050(void *pvParameters)
{
    struct ENCODER_Motion *ENCODER_MOTION_NOW;

    while (1)
    {
        /* Get new sensor events with the readings */
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        ENCODER_MOTION_NOW = (struct ENCODER_Motion *)malloc(sizeof(struct ENCODER_Motion));
        ENCODER_MOTION_NOW->temp = temp.temperature;
        ENCODER_MOTION_NOW->ax = a.acceleration.x;
        ENCODER_MOTION_NOW->ay = a.acceleration.y;
        ENCODER_MOTION_NOW->az = a.acceleration.z;
        ENCODER_MOTION_NOW->gx = g.gyro.x;
        ENCODER_MOTION_NOW->gy = g.gyro.y;
        ENCODER_MOTION_NOW->gz = g.gyro.z;

        // https: //forum.arduino.cc/t/mpu-6050-returns-wrong-values-sometimes/432528

        // xQueueSend(xQueueMPU6050, &mpu_data_now, 0);
        xQueueSend(xQueueMPU6050Metrics, &ENCODER_MOTION_NOW, 0);

        delete ENCODER_MOTION_NOW;

        vTaskDelay(500);
    }
}
