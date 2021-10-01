#include <Tone32.h>
#include <map>
#include <queue.hpp>
#include <Arduino.h>
#include "FreeRTOS.h"
#include <config.hpp>

#define BUZZER_PIN 15
#define BUZZER_CHANNEL 0

// https://vps.skybean.eu/configurator/#/audio_profile

void play(int frequency, int duration)
{
    tone(BUZZER_PIN, frequency, duration, BUZZER_CHANNEL);
}

void StopPlaying()
{
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
}

void play_welcome_beep(void *pvParameters)
{
    play(1600, 95);
    vTaskDelete(NULL);
}

std::vector<float> vVariation = {-10, -9.5, -9, -8.5, -8, -7.5, -7, -6.5, -6, -5.5, -5, -4.5, -4, -3.5, -3, -2.5, -2, -1.5, -1, -0.5, 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10};
std::vector<short> vFrequency = {200, 202, 204, 206, 210, 214, 220, 225, 230, 235, 242, 250, 263, 282, 305, 330, 358, 390, 424, 462, 500, 540, 600, 680, 800, 920, 1010, 1075, 1120, 1160, 1200, 1240, 1280, 1320, 1360, 1400, 1440, 1480, 1520, 1560, 1600};
std::vector<short> vLength = {2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 1950, 1800, 1400, 1000, 600, 380, 220, 130, 100, 100, 100, 100, 150, 200, 225, 230, 215, 200, 185, 172, 160, 150, 142, 135, 130, 125, 120, 115, 110, 105, 100, 95};
std::vector<short> vPause = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 200, 600, 1000, 1500, 2000, 2000, 1000, 600, 420, 320, 265, 230, 215, 200, 185, 172, 160, 150, 142, 135, 130, 125, 120, 115, 110, 105, 100, 95};

void buzzer_task(void *pvParameters)
{
    while (1)
    {
        float vario;
        if (xQueueReceive(xQueueVario, &vario, 0) == pdTRUE)
        {
            if (vario <= VARIO_SINK_THRESHOLD_SINK || vario >= VARIO_SINK_THRESHOLD_LIFT)
            {
                for (int i = 0; i < vVariation.size(); i++)
                {
                    float range1 = vVariation[i];
                    float range2 = vVariation[i + 1];
                    if (vario > range1 && vario < range2)
                    {
#if defined(VARIO_BUZZER_LOG_ENABLED)
                        Serial.printf("vario %f play F %i L %i P %i\n", vario, vFrequency[i], vLength[i], vPause[i]);
#endif
                        play(vFrequency[i], vLength[i]);
                        vTaskDelay(vPause[i] / portTICK_PERIOD_MS);
                        break;
                    }
                }
            }
        }
        // Watchdog reclama se rodar rapido demais
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
