#include <math.h>
#include <Wire.h>
#include <SPI.h>
#include <list>

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#include <flight_companion/config.hpp>
#include <flight_companion/queue.hpp>

Adafruit_BMP280 bmp280;

bool init_bmp280()
{
    Serial.println("[BMP280] ....................: INIT");
    boolean status = bmp280.begin(BMP280_ADDRESS);
    if (status)
    {
        Serial.println("[BMP280] connection status.............: OK");
        return true;
    }
    else
    {
        Serial.println("[BMP280] connection status.............: FAILED");
        return false;
    }
}

std::list<float> altitude_avg = {0};
std::list<float> vario_avg = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void baro_task(void *pvParameters)
{
    for (;;)
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

        struct baro_struct data;
        data.millis = millis();
        data.temperature_avg = std::accumulate(temp_sample.begin(), temp_sample.end(), 0.0) / temp_sample.size();
        data.pressure_avg = std::accumulate(pressure_sample.begin(), pressure_sample.end(), 0.0) / pressure_sample.size();
        data.altitude_avg = std::accumulate(altitude_sample.begin(), altitude_sample.end(), 0.0) / altitude_sample.size();

#ifdef XDEBUG
        Serial.println("[BMP280] Sending message to queue xQueueVario");
#endif
        xQueueSend(xQueueBaro, &data, (TickType_t)0);
#ifdef XDEBUG
        UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("BaroTask size: %i words\n", uxHighWaterMark);
#endif

        vTaskDelay(BARO_READ_RATE / portTICK_PERIOD_MS);
    }
}

// void baro_task(void *pvParameters)
// {
//     for (;;)
//     {

//         std::list<float> altitude_sample = {};
//         std::list<float> temp_sample = {};
//         std::list<float> pressure_sample = {};
//         unsigned short sample_count = 0;
//         unsigned long initial_time = millis();

//         // PASSO 1 - Laço de leitura durante 10 milisegundos
//         while (millis() - initial_time < 10)
//         {
//             float alti = bmp280.readAltitude(1013.25); // 1013.25 = 1013.25 hPa
//             float temp = bmp280.readTemperature(); // in Celsius
//             float pres_hpa = bmp280.readPressure() / 100; // Converte para hPa

//             // Se retornar valores válidos, adiciona na lista
//             if (isnan(alti) or isnan(temp) or isnan(pres_hpa))
//             {
//                 Serial.println("Invalid baro data");
//             }
//             else
//             {
//                 // Adiciona valores na lista para posterior calculo da média imediata
//                 altitude_sample.push_back(alti);
//                 temp_sample.push_back(temp);
//                 pressure_sample.push_back(pres_hpa);
//                 sample_count++; // Usado no calculo da média de amostra válidas
//             }
//         }

//         // PASSO 2 - Calcula a média de amostras válidas

//         float temperature_avg = std::accumulate(temp_sample.begin(), temp_sample.end(), 0.0) / temp_sample.size();
//         float pressure_avg = std::accumulate(pressure_sample.begin(), pressure_sample.end(), 0.0) / pressure_sample.size();
//         float altitude_avg = std::accumulate(altitude_sample.begin(), altitude_sample.end(), 0.0) / altitude_sample.size();

//         // Serial.print("Altitude: ");
//         // Serial.println(altitude_avg);

//         // Média das últimas leituras (somente altitude)
//         // if (altitude_avg.size() >= VARIO_BMP280_SAMPLES)
//         // {
//         //     altitude_avg.pop_front();
//         // }
//         // altitude_avg.push_back(std::accumulate(altitude_sample.begin(), altitude_sample.end(), 0.0) / altitude_sample.size());

//         // float altitude = std::accumulate(altitude_avg.begin(), altitude_avg.end(), 0.0) / altitude_avg.size();

//         // Variometro - Remove itens antigos da lista restando somente 20 itens
//         if (vario_avg.size() >= 5)
//         {
//             vario_avg.pop_front();
//         }

//         // Adiciona a medida nova
//         vario_avg.push_back(altitude_avg);

//         // Calcula a média das últimas 20 leituras persistentes
//         float last_vario_avg = std::accumulate(vario_avg.begin(), vario_avg.end(), 0.0) / vario_avg.size();

//         // Subtrai a altura média imediata da altura média persistente
//         float vario = altitude_avg - last_vario_avg;

//         // Posta medida na lista para ser consumida pelo buzzer
//         if (vario <= VARIO_SINK_THRESHOLD_SINK || vario >= VARIO_SINK_THRESHOLD_LIFT)
//         {
//             Serial.println("[BMP280] Sending message to queue xQueueVario");
//             // xQueueSendToBack(xQueueVario, &vario, 0);
//             // xQueueGenericSend(xQueueVario, &vario, 0, queueSEND_TO_BACK);
//         }

// #ifdef XDEBUG
//         UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
//         Serial.printf("BaroTask size: %i words\n", uxHighWaterMark);
// #endif

//         vTaskDelay(ACEL_READ_RATE / portTICK_PERIOD_MS);
//     }
// }