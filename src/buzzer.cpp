#include <map>
#include <queue.hpp>
#include <Arduino.h>
#include "FreeRTOS.h"
#include <config.hpp>

#define BUZZER_PIN 15
#define BUZZER_CHANNEL 0

// https://vps.skybean.eu/configurator/#/audio_profile

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
    bool buzzer_ativo = false;
    while (1)
    {
        float vario;
        if ((xQueueVario) and
            (!xQueueIsQueueEmptyFromISR(xQueueVario)) and
            (xQueueReceive(xQueueVario, &vario, 0) == pdTRUE))
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
                        Serial.printf("vario %f play F %i L %i P %i CORE %i\n", vario, vFrequency[i], vLength[i], vPause[i], xPortGetCoreID());
#endif
                        // Tempo mínimo de 10 ms caso tenha algum tom tocando
                        if (buzzer_ativo)
                        {
                            vTaskDelay(100);
                        }
                        noTone(BUZZER_PIN, BUZZER_CHANNEL);
                        // Tempo para poder ouvir a transição entre as notas (caso tenha alguma ativa)
                        if (buzzer_ativo)
                        {
                            vTaskDelay(100);
                        }
                        // Toca ou muda o tom atual
                        tone(BUZZER_PIN, vFrequency[i], BUZZER_CHANNEL);
                        // Espera o tempo de duração do tom
                        duracao = millis() + vLength[i];
                        // Duração da pausa caso o tom não seja alterado
                        // dentro do período de tempo (duracao)
                        pause = vPause[i];
                        buzzer_ativo = true;
                    }
                }
            }
        }
        if (buzzer_ativo and (millis() > duracao))
        {
            Serial.println("vario buzzer stop");
            buzzer_ativo = false;
            noTone(BUZZER_PIN, BUZZER_CHANNEL);
            // Aplica pause do último tone executado
            Serial.printf("vario buzzer pause %i\n", pause);
            vTaskDelay(pause);
        }
        vTaskDelay(1);
    }
}
