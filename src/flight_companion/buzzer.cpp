#include <flight_companion/buzzer.hpp>

bool initBuzzer()
{
    ledcSetup(BUZZER_CHANNEL, BUZZER_DEFAULT_FREQUENCY, BUZZER_RESOLUTION_BITS);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    ledcWriteTone(BUZZER_CHANNEL, 100);
    delay(500);
    ledcWriteTone(BUZZER_CHANNEL, 0);
    return true;
}

void tone(uint8_t pin, unsigned int frequency, uint8_t channel)
{
    if (ledcRead(channel))
    {
        Serial.printf("Tone channel %d is already in use\n", ledcRead(channel));
        return;
    }
    // ledcAttachPin(pin, channel);
    ledcWriteTone(channel, frequency);
}

void noTone(uint8_t pin, uint8_t channel)
{
    // ledcDetachPin(pin);
    ledcWrite(channel, 0);
}

std::vector<float> vVariation = {-10, -9.5, -9, -8.5, -8, -7.5, -7, -6.5, -6, -5.5, -5, -4.5, -4, -3.5, -3, -2.5, -2, -1.5, -1, -0.5, 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10};
std::vector<short> vFrequency = {200, 202, 204, 206, 210, 214, 220, 225, 230, 235, 242, 250, 263, 282, 305, 330, 358, 390, 424, 462, 500, 540, 600, 680, 800, 920, 1010, 1075, 1120, 1160, 1200, 1240, 1280, 1320, 1360, 1400, 1440, 1480, 1520, 1560, 1600};
std::vector<short> vLength = {2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 1950, 1800, 1400, 1000, 600, 380, 220, 130, 100, 100, 100, 100, 150, 200, 225, 230, 215, 200, 185, 172, 160, 150, 142, 135, 130, 125, 120, 115, 110, 105, 100, 95};
std::vector<short> vPause = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 200, 600, 1000, 1500, 2000, 2000, 1000, 600, 420, 320, 265, 230, 215, 200, 185, 172, 160, 150, 142, 135, 130, 125, 120, 115, 110, 105, 100, 95};

void buzzer_task(void *pvParameters)
{
    // Aguarda execução para aguadar leitura dos dispositivos
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    int duracao = millis() + 2000;
    int pause = 500;
    bool buzzer_buzy = false;

    for (;;)
    {
        if (buzzer_buzy and (millis() > duracao))
        {
            buzzer_buzy = false;
            noTone(BUZZER_PIN, BUZZER_CHANNEL);
            vTaskDelay(pause);
        }
        else
        {
            float vario;
            if (xQueueReceive(xQueueBuzzer, &vario, 0))
            {
                for (int i = 0; i < vVariation.size(); i++)
                {
                    float range1 = vVariation[i];
                    float range2 = vVariation[i + 1];
                    if (vario >= range1 && vario < range2)
                    {
                        Serial.print("i: ");
                        Serial.print("Vario: ");
                        Serial.println(i);
                        Serial.println(vario);

                        if (!buzzer_buzy)
                        {
                            buzzer_buzy = true;
                            noTone(BUZZER_PIN, BUZZER_CHANNEL);
                            tone(BUZZER_PIN, vFrequency[i], BUZZER_CHANNEL);
                            duracao = millis() + vLength[i];
                            pause = vPause[i];
                        }
                    }
                }
            }
        }

#ifdef XDEBUG_MEMORY
        UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("BuzzerTask size: %i words\n", uxHighWaterMark);
#endif

        vTaskDelay(BUZZER_RATE / portTICK_PERIOD_MS);

    } // for
}

void play_welcome_beep_task(void *pvParameters)
{
    noTone(BUZZER_PIN, 0);
    tone(BUZZER_PIN, 1600, 0);
    delay(100);
    tone(BUZZER_PIN, 2000, 0);
    delay(50);
    noTone(BUZZER_PIN, 0);

    vTaskDelete(NULL);
}

void play_welcome_beep()
{
    Serial.println("play_welcome_beep");
    // xTaskCreatePinnedToCore(play_welcome_beep_task, "play_welcome_beep_task", 1024, NULL, 10, NULL, CORE_1);
    noTone(15, 0);
    // tone(BUZZER_PIN, 1600, 0);
    // delay(100);
    // tone(BUZZER_PIN, 2000, 0);
    // delay(50);
    // noTone(BUZZER_PIN, 0);
}

void play_beep_screen_error()
{
    noTone(BUZZER_PIN, 0);
    tone(BUZZER_PIN, 500, 0);
    delay(100);
    noTone(BUZZER_PIN, 0);
    tone(BUZZER_PIN, 800, 0);
    delay(50);
    noTone(BUZZER_PIN, 0);
}