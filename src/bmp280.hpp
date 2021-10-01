#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <config.hpp>
#include <cache_global.hpp>
#include <list>
#include <queue.hpp>

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

std::list<float> altitude_avg = {0};
std::list<float> vario_avg = {0};

void loop_bmp280_by_time(void *pvParameters = NULL)
{
    // A leitura do BMP280 em thread está causando kernel panic

    baro_cache.reads++;
    std::list<float> altitude_sample = {};
    std::list<float> temp_sample = {};
    std::list<float> pressure_sample = {};
    unsigned short sample_count = 0;
    unsigned short failed_sample_count = 0;

    unsigned long initial_time = millis();
    while (millis() - initial_time < 10)
    {
#ifdef VARIO_BMP280_LOG_ENABLED
        Serial.print(".");
#endif
        float alti = bmp280.readAltitude(1013.25);
        float temp = bmp280.readTemperature();
        float pres = bmp280.readPressure() / 100;
        if (isnan(alti) or isnan(temp) or isnan(pres))
        {
            failed_sample_count++;
        }
        else
        {
            altitude_sample.push_back(alti);
            temp_sample.push_back(temp);
            pressure_sample.push_back(pres);
            sample_count++;
        }
    }
#ifdef VARIO_BMP280_LOG_ENABLED
    Serial.println("");
#endif

    baro_cache.temperature = std::accumulate(temp_sample.begin(), temp_sample.end(), 0.0) / temp_sample.size();
    baro_cache.pressure = std::accumulate(pressure_sample.begin(), pressure_sample.end(), 0.0) / pressure_sample.size();
    baro_cache.altitude_raw = bmp280.readPressure() / 100;
    baro_cache.altitude_sample = std::accumulate(altitude_sample.begin(), altitude_sample.end(), 0.0) / altitude_sample.size();
    baro_cache.sample_count = sample_count;
    baro_cache.failed_sample_count = failed_sample_count;

    // Média das últimas leituras (somente altitude)
    if (altitude_avg.size() >= VARIO_BMP280_SAMPLES)
    {
        altitude_avg.pop_front();
    }
    altitude_avg.push_back(std::accumulate(altitude_sample.begin(), altitude_sample.end(), 0.0) / altitude_sample.size());

    baro_cache.altitude = std::accumulate(altitude_avg.begin(), altitude_avg.end(), 0.0) / altitude_avg.size();

    // Variometro
    if (vario_avg.size() >= 20)
    {
        vario_avg.pop_front();
    }
    vario_avg.push_back(baro_cache.altitude_sample);
    float last_vario_avg = std::accumulate(vario_avg.begin(), vario_avg.end(), 0.0) / vario_avg.size();

    baro_cache.vario = baro_cache.altitude_sample - last_vario_avg;

    xQueueSend(xQueueVario, &baro_cache.vario, (TickType_t)0);

#ifdef VARIO_BMP280_LOG_ENABLED
    Serial.printf("BRE = %lu ", baro_cache.reads);
    Serial.printf("BTE = %f *C ", baro_cache.temperature);
    Serial.printf("BPR = %f hPa ", baro_cache.pressure);
    Serial.printf("BAL = %f m\n", baro_cache.altitude);
#endif
}
