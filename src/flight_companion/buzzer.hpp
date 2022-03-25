
#ifndef BUZZER_H_
#define BUZZER_H_

void play_welcome_beep();

void buzzer_task(void *pvParameters);

void play_beep_screen_error(void *pvParameters);

void noTone(uint8_t pin, uint8_t channel);

void tone(uint8_t pin, unsigned int frequency, uint8_t channel);

#endif
