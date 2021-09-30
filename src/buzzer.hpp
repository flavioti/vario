#ifndef BUZZER_H_
#define BUZZER_H_

void play_welcome_beep(void *pvParameters);

void play_welcome_beep_task();

void buzzer_task(void *pvParameters);

void play_vario_beep(float vario);

#endif /* BUZZER_H_ */
