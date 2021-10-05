#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

// RTC_DATA_ATTR guarda dados durante deep sleep
struct barometer_cache
{
    float temperature = 0;
    float pressure = 0;
    float altitude = 0;
    float altitude_sample = 0;
    float vario = 0;
};

barometer_cache baro_cache;

struct geolocalization_cache
{
    float altitude = 0;
    float longitude = 0.000000;
    float latitude = 0.000000;
    int satellites = 0;
};

geolocalization_cache geo_cache;

struct system_cache
{
    int loop_counter = 0;
    float battery_voltage = 0;
    int battery_percentage = 0;
    float esp32_temperature = 0;
    float esp32_ts_temperature = 0;
    unsigned long uptime = 0;
    unsigned short power_down_voltage = 0;
    unsigned long loop_millis = 0;
    unsigned long core_status_millis = 0;
    unsigned long gps_millis = 0;
    unsigned long baro_millis = 0;
    unsigned long screen_millis = 0;
    unsigned long handle_client_millis = 0;
};

system_cache sys_cache;

struct MPU_cache
{
    float temp = 0;
    float ax = 0;
    float ay = 0;
    float az = 0;
    float gx = 0;
    float gy = 0;
    float gz = 0;
};

MPU_cache mpu_cache;

#endif
