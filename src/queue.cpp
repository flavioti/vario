#include <Arduino.h>
#include "FreeRTOS.h"

QueueHandle_t xQueueVario = xQueueCreate(1, sizeof(float));
