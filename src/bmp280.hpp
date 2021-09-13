#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <config.hpp>

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10

Adafruit_BMP280 bmp280; // I2C

struct cached_data
{
    float temperature;
    float pressure;
    float altitude;
};

cached_data bmp_cached_data;

void init_bmp280()
{
    boolean status = bmp280.begin(VARIO_BMP280_I2C_ADDRESS);
    if (!status)
    {
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        for (;;)
            ; // Don't proceed, loop forever
    }
}

void loop_bmp280()
{
    // float seaLevelhPa = 1013.25; // Padrão
    float seaLevelhPa = 1017.65; // Santa Rita do Passa Quatro
    Serial.printf("Temperature = %f *C\n", bmp280.readTemperature());
    Serial.printf("Pressure = %f hPa\n", bmp280.readPressure() / 100); // Pa = Pascal, hPa = hectoPascal
    Serial.printf("Approx altitude = %f m\n", bmp280.readAltitude(seaLevelhPa));

    bmp_cached_data.temperature = bmp280.readTemperature();
    bmp_cached_data.pressure = bmp280.readPressure() / 100;
    bmp_cached_data.altitude = bmp280.readAltitude(seaLevelhPa);
}
