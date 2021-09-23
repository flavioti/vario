#include <Arduino.h>
#include <config.hpp>
#include <cache_global.hpp>

// https://github.com/JoepSchyns/Low_power_TTGO_T-beam/tree/master/low_power
// https://github.com/JoepSchyns/Low_power_TTGO_T-beam/commit/8d2845051f3c24c58540f762110396d6d8d439a0

#if defined(USE_SCREEN)
#include <screen.hpp>
#endif

#if defined(USE_GPS)
#include <TinyGPS++.h>
#include <gps.hpp>
#endif

#if defined(USE_BMP280)
#include <bmp280.hpp>
#endif

#if defined(USE_BUZZER)
#include <buzzer.hpp>
#endif

#if defined(USE_ENERGIA)
#include <energia.hpp>
#endif

#if defined(USE_WIFI) or defined(USE_OTA)
#include <network.hpp>
#endif

void Task1code(void *pvParameters)
{
    for (;;)
    {
        Serial.print("Task1 running on core ");
        Serial.println(xPortGetCoreID());

        test_vario();
    }
}

void setup()
{
    sleep(2);
    Serial.begin(9600);
    while (!Serial)
        ;
    Serial.println("iniciando");

#if defined(USE_ENERGIA)
    cache_status();
#endif

#if defined(USE_WIFI)
    connect_wifi2();
#endif

#if defined(USE_SCREEN)
    init_screen();
#endif

#if defined(USE_GPS)
    Serial.println("USE_GPS");
    setup_g();
#endif

#if defined(USE_BMP280)
    init_bmp280();
#endif

#if defined(USE_BUZZER)
    play_welcome_beep_task();
#endif

#if defined(USE_OTA)
    config_ota();
#endif
}

void loop()
{
    sys_cache.loop_counter++;

#if defined(USE_GPS)
    loop_g();
#endif

#if defined(USE_BMP280)
    loop_bmp280();
#endif

#if defined(USE_SCREEN)
    update_screen_a();
#endif

#if defined(USE_POST_METRICS) and defined(USE_WIFI)
    send_metrics();
#endif

#if defined(USE_OTA)
    handle_client();
#endif

    // delay(500);
}
