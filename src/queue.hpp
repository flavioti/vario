#include <Arduino.h>
#include "FreeRTOS.h"

#ifndef QUEUE_HPP
#define QUEUE_HPP
struct ENCODER_Motion
{
    float temp;
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
};

#endif

// Fila para notificar variação de altitude
extern QueueHandle_t xQueueVario;
// Fila de dados barométricos a serem enviadas ao data hub
extern QueueHandle_t xQueueIngressBarometricData;

extern QueueHandle_t xQueueMPU6050;

extern QueueHandle_t xQueueMPU6050Metrics;
