#include <Arduino.h>
#include "FreeRTOS.h"
#include "queue.hpp"

#include <CopilotMessages.hpp>

QueueHandle_t xQueueMetrics = xQueueCreate(1, sizeof(struct metrics_data));
