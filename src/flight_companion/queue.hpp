#include <Arduino.h>
#include "FreeRTOS.h"

#include <iostream>
#include <string>

#ifndef _QUEUE_HPP_
#define _QUEUE_HPP_

struct gnss_struct
{
  uint32_t millis; // Para saber se o dado esta mudando no prometheus
  uint32_t sat_count;
  double location_lat;
  double location_lng;
  double altitude_meters;
  double speed_kmph;
  int32_t course_value;
  double course_deg;
  const char *course_cardinal;
  uint32_t date;
  uint32_t time;
  int32_t hdop;

  String toString()
  {
    return "sat=" + String(sat_count) +
           " lat=" + String(location_lat) +
           " lng=" + String(location_lng) +
           " altitude=" + String(altitude_meters) +
           " kmph=" + String(speed_kmph) +
           " course_value=" + String(course_value) +
           " course_deg=" + String(course_deg) +
           " course_cardinal=" + String(course_cardinal) +
           " date=" + String(date) +
           " time=" + String(time) +
           " hdop=" + String(hdop);
  }
};

#endif

// Fila para notificar variação de altitude
extern QueueHandle_t xQueueBaro;
// Fila de dados barométricos a serem enviadas ao data hub
extern QueueHandle_t xQueueIngressBarometricData;

extern QueueHandle_t xQueueBuzzerDeltaAZ;

extern QueueHandle_t xQueueMPU6050Metrics;

extern QueueHandle_t xQueueGNSSMetrics;

extern QueueHandle_t xQueueBuzzer;