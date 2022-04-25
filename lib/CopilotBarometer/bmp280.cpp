#include <math.h>
#include <Wire.h>
#include <SPI.h>
#include <list>

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#include <bmp280.hpp>

Adafruit_BMP280 bmp280;

bool initBMP280()
{
    Serial.println("[BMP280] ....................: INIT");
    boolean status = bmp280.begin(0x76);
    if (status)
    {
        Serial.println("[BMP280] connection status...: OK");
        return true;
    }
    else
    {
        Serial.println("[BMP280] connection status...: FAILED");
        return false;
    }
}

std::list<float> altitude_avg = {0};
std::list<float> vario_avg = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

float calc_vario(float altitude)
{
    if (vario_avg.size() >= 5)
    {
        vario_avg.pop_front();
    }
    vario_avg.push_back(altitude);

    float last_vario_avg = std::accumulate(vario_avg.begin(), vario_avg.end(), 0.0) / vario_avg.size();

    float vario = last_vario_avg - altitude;
    if (vario > 10)
    {
        vario = 0;
    }
    return vario;
}

baro_struct_t read_barometer()
{
    std::list<float> altitude_sample = {};
    std::list<float> temp_sample = {};
    std::list<float> pressure_sample = {};
    unsigned long initial_time = millis();

    // PASSO 1 - Laço de leitura durante 10 milisegundos
    while (millis() - initial_time < 10)
    {
        float alti = bmp280.readAltitude(1013.25);    // 1013.25 = 1013.25 hPa
        float temp = bmp280.readTemperature();        // in Celsius
        float pres_hpa = bmp280.readPressure() / 100; // Converte para hPa

        // Se retornar valores válidos, adiciona na lista
        if (isnan(alti) or isnan(temp) or isnan(pres_hpa))
        {
            Serial.println("Invalid baro data");
        }
        else
        {
            // Adiciona valores na lista para posterior calculo da média imediata
            altitude_sample.push_back(alti);
            temp_sample.push_back(temp);
            pressure_sample.push_back(pres_hpa);
        }
    }

    float temperature = std::accumulate(temp_sample.begin(), temp_sample.end(), 0.0) / temp_sample.size();
    float pressure = std::accumulate(pressure_sample.begin(), pressure_sample.end(), 0.0) / pressure_sample.size();
    float altitude = std::accumulate(altitude_sample.begin(), altitude_sample.end(), 0.0) / altitude_sample.size();

    if (temperature > 99)
    {
        temperature = 0;
    }

    if (pressure > 9999)
    {
        pressure = 0;
    }

    if (altitude > 9999)
    {
        altitude = 0;
    }

    struct baro_struct_t data;
    data.temperature = ((int)(temperature * 100 + .5) / 100.0); // arredonda
    data.pressure = ((int)(pressure * 100 + .5) / 100.0);       // arredonda
    data.altitude = ((int)(altitude * 100 + .5) / 100.0);       // arredonda
    data.vario = calc_vario(altitude);

    return data;
}
