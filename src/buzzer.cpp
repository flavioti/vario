#include <Tone32.h>
#include <map>

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

// std::vector<float> variation = {-0.5, 0, 0.5};
// std::vector<float> frequency = {462, 500, 540};
// std::vector<float> length = {100, 100, 150};
// std::vector<float> pause = {1000, 600, 420};

float last_vario = 0;

void play_vario_beep(float vario)
{
    Serial.printf("%f ", vario);
    if ((vario == 0) and (last_vario != 0))
    {
        Serial.println("0 e 0");
        play(500, 100);
    }
    else if (vario > 0)
    {
        if (vario >= 1.0)
        {
            Serial.println(" > 1.0");
            play(600, 200);
        }
        else if (vario >= 0.5)
        {
            Serial.println(" > 0.5");
            play(540, 100);
        }
        else
        {
            Serial.println("");
        }
    }
    else if (vario < 0)
    {
        if (vario <= -0.5)
        {
            Serial.println(" < -0.5");
            play(462, 100);
        }
        else if ((vario > -0.5) and (vario <= -1.0))
        {
            Serial.println(" < -1.0");
            play(424, 100);
        }
        else
        {
            Serial.println("");
        }
    }

    last_vario = vario;
}
