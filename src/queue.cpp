#include <Arduino.h>
#include "FreeRTOS.h"
#include "queue.hpp"

QueueHandle_t xQueueVario = xQueueCreate(10, sizeof(float));
QueueHandle_t xQueueIngressBarometricData = xQueueCreate(1, sizeof(struct BARO_data *));
QueueHandle_t xQueueMPU6050 = xQueueCreate(1, sizeof(struct MPU_data *));
QueueHandle_t xQueueMPU6050Metrics = xQueueCreate(1, sizeof(struct ENCODER_Motion *));
