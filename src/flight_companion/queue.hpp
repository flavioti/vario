#include <Arduino.h>
#include "FreeRTOS.h"

#ifndef QUEUE_HPP
#define QUEUE_HPP
// struct ENCODER_Motion
// {
//     float temp;
//     int16_t ax;
//     int16_t ay;
//     int16_t az;
//     int16_t gx;
//     int16_t gy;
//     int16_t gz;
//     int16_t delta_z;
// };

#endif

// Fila para notificar variação de altitude
extern QueueHandle_t xQueueVario;
// Fila de dados barométricos a serem enviadas ao data hub
extern QueueHandle_t xQueueIngressBarometricData;

extern QueueHandle_t xQueueBuzzerDeltaAZ;

extern QueueHandle_t xQueueMPU6050Metrics;

extern QueueHandle_t xQueueGNSSMetrics;

struct sensor {
  int deviceId;
  int measurementType;
  double value;
  int sat;
};