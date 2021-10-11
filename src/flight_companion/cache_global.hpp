#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

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
    float battery_voltage = 0;
    int battery_percentage = 0;
    float esp32_temperature = 0;
    float esp32_ts_temperature = 0;
    unsigned long uptime = 0;
    unsigned short power_down_voltage = 0;
};

system_cache sys_cache;

struct MPU_cache
{
    float temp = 0;
    int16_t ax = 0;
    int16_t ay = 0;
    int16_t az = 0;
    int16_t gx = 0;
    int16_t gy = 0;
    int16_t gz = 0;
    int16_t delta_z = 0;
};

MPU_cache mpu_cache;

#endif
