#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

struct barometer_cache
{
    float temperature = 0;
    float pressure = 0;
    float altitude = 0;
};

barometer_cache baro_cache;

#endif
