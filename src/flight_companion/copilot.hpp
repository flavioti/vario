#ifndef COPILOT_H_INCLUDED
#define COPILOT_H_INCLUDED

void copilot_task(void *pvParameters);

struct system_cache
{
    float battery_voltage = 0;
    int battery_percentage = 0;
    float esp32_ts_temperature = 0;
    unsigned long uptime = 0;
    unsigned short power_down_voltage = 0;
};

String getMetrics();

void setup_esp_now();

#endif