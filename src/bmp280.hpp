#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <config.hpp>
#include <cache_barometer.hpp>
#include <list>

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10

Adafruit_BMP280 bmp280; // I2C

void init_bmp280()
{
    boolean status = bmp280.begin(VARIO_BMP280_I2C_ADDRESS);
    if (!status)
    {
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        for (;;)
            ;
    }
}

std::list<float> mylist = {};

void loop_bmp280()
{
    if (mylist.size() >= VARIO_BMP280_SAMPLES)
    {
        mylist.pop_front();
    }

    mylist.push_back(bmp280.readAltitude(1013.25));

    baro_cache.temperature = bmp280.readTemperature();
    baro_cache.pressure = bmp280.readPressure() / 100;
    baro_cache.altitude = std::accumulate(mylist.begin(), mylist.end(), 0.0) / mylist.size();

#ifdef VARIO_BMP280_LOG_ENABLED
    Serial.printf("Temperature = %f *C\n", baro_cache.temperature);
    Serial.printf("Pressure = %f hPa\n", baro_cache.pressure); // Pa = Pascal, hPa = hectoPascal
    Serial.printf("Approx altitude = %f m\n", baro_cache.altitude);
#endif
}
