
#ifndef _BUZZER_H_
#define _BUZZER_H_

#include <map>
#include <flight_companion/queue.hpp>
#include <Arduino.h>
#include "FreeRTOS.h"
#include <flight_companion/config.hpp>

bool initBuzzer();

void play_welcome_beep();

void buzzer_task(void *pvParameters);

void play_beep_screen_error(void *pvParameters);

#endif
