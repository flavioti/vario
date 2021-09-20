#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

// RTC_DATA_ATTR guarda dados durante deep sleep
struct barometer_cache
{
    float temperature = 0;
    float pressure = 0;
    float altitude = 0;
};

barometer_cache baro_cache;

struct geolocalization_cache
{
    float altitude = 9999;
    float longitude = 9999;
    float latitude = 9999;
    int satellites = 9999;
};

geolocalization_cache geo_cache;

struct system_cache
{
    int loop_counter = 0;
    float battery_voltage = 9999;
    int battery_percentage = 9999;
};

system_cache sys_cache;

#endif
