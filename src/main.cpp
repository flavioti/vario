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

#include <energia.hpp>

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

    configure_system();
    print_sys_diagnostic();
    cache_core_status();

#if defined(USE_BUZZER)
    play_welcome_beep_task();
#endif

#if defined(USE_WIFI) or defined(USE_OTA)
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

#if defined(USE_OTA)
    config_ota();
#endif
}

unsigned long last_loop_time = millis();

void loop()
{
    sys_cache.loop_millis = millis() - last_loop_time;
    if (sys_cache.loop_millis > 10) // 500ms
    {
        last_loop_time = millis();
        sys_cache.loop_counter++;
        cache_core_status();

#if defined(USE_GPS)
        loop_g();
#endif

#if defined(USE_BMP280)
        // begin_read_bmp280_task();
        // loop_bmp280();
        loop_bmp280_by_time();
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
    }

    play_vario_beep(baro_cache.vario);
}
