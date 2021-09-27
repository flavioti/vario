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

std::list<float> mylist = {0};

void loop_bmp280(void *pvParameters = NULL)
{
    if (mylist.size() >= VARIO_BMP280_SAMPLES)
    {
        mylist.pop_front();
    }

    mylist.push_back(bmp280.readAltitude(1013.25));

    baro_cache.temperature = bmp280.readTemperature();
    baro_cache.pressure = bmp280.readPressure() / 100;
    if (mylist.size() > 0)
    {
        baro_cache.altitude = std::accumulate(mylist.begin(), mylist.end(), 0.0) / mylist.size();
    }

#ifdef VARIO_BMP280_LOG_ENABLED
    Serial.printf("BRE = %lu ", baro_cache.reads);
    Serial.printf("BTE = %f *C ", baro_cache.temperature);
    Serial.printf("BPR = %f hPa ", baro_cache.pressure);
    Serial.printf("BAL = %f m\n", baro_cache.altitude);
#endif
    // vTaskDelay(5000 / portTICK_PERIOD_MS);
    // vTaskDelete(NULL);
}

std::list<float> altitude_avg = {0};
std::list<float> vario_avg = {0};

// Iteração baseada no tempo
void loop_bmp280_by_time(void *pvParameters = NULL)
{
    // Faz leitura do sensor dentro de 500 milisegundos
    baro_cache.reads++;
    std::list<float> altitude_sample = {};
    std::list<float> temp_sample = {};
    std::list<float> pressure_sample = {};
    unsigned short sample_count = 0;

    unsigned long initial_time = millis();
    while (millis() - initial_time < 100)
    {
#ifdef VARIO_BMP280_LOG_ENABLED
        Serial.print(".");
#endif
        altitude_sample.push_back(bmp280.readAltitude(1013.25));
        temp_sample.push_back(bmp280.readTemperature());
        pressure_sample.push_back(bmp280.readPressure() / 100);
        sample_count++;
    }
#ifdef VARIO_BMP280_LOG_ENABLED
    Serial.println("");
#endif

    baro_cache.temperature = std::accumulate(temp_sample.begin(), temp_sample.end(), 0.0) / temp_sample.size();
    baro_cache.pressure = std::accumulate(pressure_sample.begin(), pressure_sample.end(), 0.0) / pressure_sample.size();
    baro_cache.altitude_raw = bmp280.readPressure() / 100;
    baro_cache.altitude_sample = std::accumulate(altitude_sample.begin(), altitude_sample.end(), 0.0) / altitude_sample.size();
    baro_cache.sample_count = sample_count;

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

#ifdef VARIO_BMP280_LOG_ENABLED
    Serial.printf("BRE = %lu ", baro_cache.reads);
    Serial.printf("BTE = %f *C ", baro_cache.temperature);
    Serial.printf("BPR = %f hPa ", baro_cache.pressure);
    Serial.printf("BAL = %f m\n", baro_cache.altitude);
#endif
    // vTaskDelay(5000 / portTICK_PERIOD_MS);
    // vTaskDelete(NULL);
}

TaskHandle_t BaroTask;

// void begin_read_bmp280_task()
// {
//     // Serial.println("BMP280 task started");
//     xTaskCreatePinnedToCore(
//         loop_bmp280,      /* Task function. */
//         "read_baro_task", /* name of task. */
//         10000,            /* Stack size of task */
//         NULL,             /* parameter of the task */
//         1,                /* priority of the task */
//         &BaroTask,        /* Task handle to keep track of created task */
//         1);               /* pin task to core */
// }
