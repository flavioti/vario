#include <Arduino.h>
#include <config.hpp>
#include <cache_barometer.hpp>

#if defined(USE_SCREEN)
#include <screen.hpp>
#endif

#if defined(USE_GPS)
#include <gps.hpp>
#endif

#if defined(USE_BMP280)
#include <bmp280.hpp>
#endif

#if defined(USE_BUZZER)
#include <buzzer.hpp>
#endif

void setup()
{
    sleep(2);
    Serial.begin(9600);
    while (!Serial)
        ;
    Serial.println("iniciando");

#if defined(USE_SCREEN)
    init_screen();
#endif

#if defined(USE_GPS)
    init_gps();
#endif

#if defined(USE_BMP280)
    init_bmp280();
#endif

#if defined(USE_BUZZER)
    play_welcome_beep();
#endif
}

void loop()
{
#if defined(USE_BMP280)
    loop_bmp280();
#endif

#if defined(USE_SCREEN)
    update_screen_a();
#endif

#if defined(USE_BUZZER)
    // play_melody();
#endif
    // Serial.println("**********************");
    delay(500);
}
