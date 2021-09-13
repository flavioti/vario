#include <Arduino.h>
#include <config.hpp>

#if defined(USE_SCREEN)
#include <screen.hpp>
#endif

#if defined(USE_GPS)
#include <gps.hpp>
#endif

#if defined(USE_BMP280)
#include <bmp280.hpp>
#endif

// #pragma once

void setup()
{
    sleep(1);
    Serial.begin(9600);
    while (!Serial)
        ;

#if defined(USE_SCREEN)
    init_screen();
#endif

#if defined(USE_GPS)
    init_gps();
#endif

#if defined(USE_BMP280)
    init_bmp280();
#endif
}

void loop()
{
#if defined(USE_BMP280)
    loop_bmp280();
#endif

    Serial.println("**********************");
    delay(3000);
}
