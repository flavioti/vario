#include <Arduino.h>
#include "FreeRTOS.h"

QueueHandle_t xQueueVario = xQueueCreate(10, sizeof(float));
