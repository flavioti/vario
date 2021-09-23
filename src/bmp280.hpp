#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <config.hpp>
#include <cache_global.hpp>
#include <list>

Adafruit_BMP280 bmp280;

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
    Serial.printf("BTE = %f *C ", baro_cache.temperature);
    Serial.printf("BPR = %f hPa ", baro_cache.pressure);
    Serial.printf("BAL = %f m\n", baro_cache.altitude);
#endif
}
