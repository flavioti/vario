#include <map>
#include <flight_companion/queue.hpp>
#include <Arduino.h>
#include "FreeRTOS.h"
#include <flight_companion/config.hpp>

#define BUZZER_PIN 15
#define BUZZER_CHANNEL 0

void tone(uint8_t pin, unsigned int frequency, uint8_t channel)
{
    if (ledcRead(channel))
    {
        log_e("Tone channel %d is already in use", ledcRead(channel));
        return;
    }
    ledcAttachPin(pin, channel);
    ledcWriteTone(channel, frequency);
}

void noTone(uint8_t pin, uint8_t channel)
{
    ledcDetachPin(pin);
    ledcWrite(channel, 0);
}

void play_welcome_beep(void *pvParameters)
{
    ESP_LOGI(&TAG, "play_welcome_beep running");
    tone(BUZZER_PIN, 1600, BUZZER_CHANNEL);
    delay(95);
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
    vTaskDelete(NULL);
}

std::vector<float> vVariation = {-10, -9.5, -9, -8.5, -8, -7.5, -7, -6.5, -6, -5.5, -5, -4.5, -4, -3.5, -3, -2.5, -2, -1.5, -1, -0.5, 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10};
std::vector<short> vFrequency = {200, 202, 204, 206, 210, 214, 220, 225, 230, 235, 242, 250, 263, 282, 305, 330, 358, 390, 424, 462, 500, 540, 600, 680, 800, 920, 1010, 1075, 1120, 1160, 1200, 1240, 1280, 1320, 1360, 1400, 1440, 1480, 1520, 1560, 1600};
std::vector<short> vLength = {2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 1950, 1800, 1400, 1000, 600, 380, 220, 130, 100, 100, 100, 100, 150, 200, 225, 230, 215, 200, 185, 172, 160, 150, 142, 135, 130, 125, 120, 115, 110, 105, 100, 95};
std::vector<short> vPause = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 200, 600, 1000, 1500, 2000, 2000, 1000, 600, 420, 320, 265, 230, 215, 200, 185, 172, 160, 150, 142, 135, 130, 125, 120, 115, 110, 105, 100, 95};

void buzzer_task(void *pvParameters)
{
    int duracao = millis();
    int pause = 0;
    bool buzzer_buzy = false;

    float last_delta_z = 0; // variação mais recente do eixo Z
    while (1)
    {
        xQueueReceive(xQueueBuzzerDeltaAZ, &last_delta_z, 0);

        float vario;
        if (xQueueReceive(xQueueVario, &vario, 0))
        {
            Serial.println("VARIO\tDELTA_Z");
            Serial.printf("%.3f\t%.3f\n", vario, last_delta_z);

            if ((vario <= VARIO_SINK_THRESHOLD_SINK ||
                 vario >= VARIO_SINK_THRESHOLD_LIFT) and
                (last_delta_z <= VARIO_ACCEL_Z_THRESHOLD_SINK ||
                 last_delta_z >= VARIO_ACCEL_Z_THRESHOLD_LIFT))
            {
                for (int i = 0; i < vVariation.size(); i++)
                {
                    float range1 = vVariation[i];
                    float range2 = vVariation[i + 1];
                    if (vario > range1 && vario < range2)
                    {
                        if (!buzzer_buzy)
                        {
                            buzzer_buzy = true;
                            Serial.println("VARIO\tDELTA_Z\tFREQ\tDURACAO\tPAUSA\tCORE");
                            Serial.printf("%.3f\t%.3f\t%i\t%i\t%i\t%i\n",
                                          vario, last_delta_z, vFrequency[i], vLength[i], vPause[i], xPortGetCoreID());
                            noTone(BUZZER_PIN, BUZZER_CHANNEL);
                            tone(BUZZER_PIN, vFrequency[i], BUZZER_CHANNEL);
                            duracao = millis() + vLength[i];
                            pause = vPause[i];
                        }
                    }
                }
            }
        }

        if (buzzer_buzy and (millis() > duracao))
        {
            buzzer_buzy = false;
            noTone(BUZZER_PIN, BUZZER_CHANNEL);
            vTaskDelay(pause);
        }
        vTaskDelay(1);
    }
}
