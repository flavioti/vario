#include <Tone32.h>
#include <map>

#define BUZZER_PIN 15
#define BUZZER_CHANNEL 0

// https://vps.skybean.eu/configurator/#/audio_profile

void play(int frequency, int duration, short pause = 0)
{
    Serial.printf("play %i %i %i\n", frequency, duration, pause);
    tone(BUZZER_PIN, frequency, duration, BUZZER_CHANNEL);
    delay(pause);
}

void StopPlaying()
{
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
}

void test_vario()
{
    Serial.printf("Task1 running on core %i", xPortGetCoreID());
    // play(200, 2000);
    // play(202, 2000);
    // play(204, 2000);
    // play(206, 2000);
    // play(210, 2000);
    // play(214, 2000);
    // play(220, 2000);
    // play(225, 2000);
    // play(235, 1950);
    // play(242, 1800);
    // play(250, 1400);
    play(1600, 95);
}

void play_melody()
{
    tone(BUZZER_PIN, NOTE_C4, 500, BUZZER_CHANNEL);
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
    tone(BUZZER_PIN, NOTE_D4, 500, BUZZER_CHANNEL);
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
    tone(BUZZER_PIN, NOTE_E4, 500, BUZZER_CHANNEL);
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
    tone(BUZZER_PIN, NOTE_F4, 500, BUZZER_CHANNEL);
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
    tone(BUZZER_PIN, NOTE_G4, 500, BUZZER_CHANNEL);
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
    tone(BUZZER_PIN, NOTE_A4, 500, BUZZER_CHANNEL);
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
    tone(BUZZER_PIN, NOTE_B4, 500, BUZZER_CHANNEL);
    noTone(BUZZER_PIN, BUZZER_CHANNEL);
}

#define tone_out1 8
#define tone_out2 9

void play_welcome_beep(void *pvParameters)
{
    Serial.printf("Task1 running on core %i", xPortGetCoreID());
    // play(200, 2000);
    // play(202, 2000);
    // play(204, 2000);
    // play(206, 2000);
    // play(210, 2000);
    // play(214, 2000);
    // play(220, 2000);
    // play(225, 2000);
    // play(235, 1950);
    // play(242, 1800);
    // play(250, 1400);
    play(1600, 95);

    // Pause the task again for 500ms
    // vTaskDelay(500 / portTICK_PERIOD_MS);
    vTaskDelete(NULL); // Deve ser sempre chamada para finalizar a tarefa
}

TaskHandle_t Task1;

void play_welcome_beep_task()
{
    xTaskCreatePinnedToCore(
        play_welcome_beep,   /* Task function. */
        "play_welcome_beep", /* name of task. */
        10000,               /* Stack size of task */
        NULL,                /* parameter of the task */
        1,                   /* priority of the task */
        &Task1,              /* Task handle to keep track of created task */
        1);                  /* pin task to core 0 */
}

std::vector<float> vVariation = {-10, -9.5, -9, -8.5, -8, -7.5, -7, -6.5, -6, -5.5, -5, -4.5, -4, -3.5, -3, -2.5, -2, -1.5, -1, -0.5, 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10};
// std::vector<float> vVariation = {-1.8, -1.7, -1.6, -1.5, -1.4, -1.3, -1.2, -1.1, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2};
std::vector<short> vFrequency = {200, 202, 204, 206, 210, 214, 220, 225, 230, 235, 242, 250, 263, 282, 305, 330, 358, 390, 424, 462, 500, 540, 600, 680, 800, 920, 1010, 1075, 1120, 1160, 1200, 1240, 1280, 1320, 1360, 1400, 1440, 1480, 1520, 1560, 1600};
std::vector<short> vLength = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100};
std::vector<short> vPause = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100};

void play_vario_beep(float vario)
{
    Serial.println(vario);
    if (vario <= -0.5 || vario >= 0.5)
    {
        for (int i = 0; i < vVariation.size(); i++)
        {
            float range1 = vVariation[i];
            float range2 = vVariation[i + 1];
            if (vario > range1 && vario < range2)
            {
                play(vFrequency[i], vLength[i], vPause[i]);
                break;
            }
        }
    }
}
