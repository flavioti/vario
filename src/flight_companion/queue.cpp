#include <Arduino.h>
#include "FreeRTOS.h"
#include "queue.hpp"

#include <model/espnow_message.hpp>

QueueHandle_t xQueueBaro = xQueueCreate(1, sizeof(struct baro_struct_t));
QueueHandle_t xQueueIngressBarometricData = xQueueCreate(1, sizeof(struct BARO_data *));
QueueHandle_t xQueueBuzzerDeltaAZ = xQueueCreate(1, sizeof(float));
QueueHandle_t xQueueMPU6050Metrics = xQueueCreate(1, sizeof(struct ENCODER_Motion *));
QueueHandle_t xQueueGNSSMetrics = xQueueCreate(10, sizeof(struct gnss_struct));
