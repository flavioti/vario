#include <Arduino.h>
#include "FreeRTOS.h"

// Fila para notificar variação de altitude
extern QueueHandle_t xQueueVario;
