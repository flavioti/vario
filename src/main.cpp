#include <Arduino.h>
#include <config.hpp>
#include <cache_global.hpp>
#include <queue.hpp>
#include <FreeRTOS.h>
#include <mpu6050.hpp>
#include <esp32-hal-i2c.h>

// https://github.com/JoepSchyns/Low_power_TTGO_T-beam/tree/master/low_power
// https://github.com/JoepSchyns/Low_power_TTGO_T-beam/commit/8d2845051f3c24c58540f762110396d6d8d439a0
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html

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

#if defined(USE_WIFI) or defined(USE_WEBSERVER)
#include <network.hpp>
#endif

#if defined(USE_BUZZER)
TaskHandle_t BuzzerTaskHandler;
#endif

TaskHandle_t WelcomeTaskHandler;
TaskHandle_t MPUTaskHandler;

void setup()
{
    esp_log_level_set("*", ESP_LOG_VERBOSE);

#if defined(DISABLE_WATCH_DOG)
    disableCore0WDT();
    disableCore1WDT();
#endif

    sleep(2);
    Serial.begin(9600);
    while (!Serial)
        ;
    ESP_LOGI(&TAG, "iniciando");

    configure_system();
    // print_sys_diagnostic();

#if defined(CAPTURE_CORE_STATUS)
    cache_core_status();
#endif

#if defined(USE_MPU6050)
    setup_mpu6050();
#endif

#if defined(USE_BUZZER)
    xTaskCreatePinnedToCore(play_welcome_beep, "play_welcome_beep", 10000, NULL, 1, &WelcomeTaskHandler, CORE_1);
    xTaskCreatePinnedToCore(buzzer_task, "buzzer_task_core_0", 10000, NULL, 10, &BuzzerTaskHandler, CORE_1);
#endif

#if defined(USE_WIFI) or defined(USE_WEBSERVER)
    // setup_network();
    connect_wifi2();
#endif

#if defined(USE_SCREEN)
    init_screen();
#endif

#if defined(USE_GPS)
    setup_g();
#endif

#if defined(USE_BMP280)
    init_bmp280();
#endif

#if defined(USE_WEBSERVER)
    config_ota();
#endif
}

unsigned long next_loop_millis = millis();

void loop()
{
    if (millis() > next_loop_millis)
    {
        unsigned long initial_loop_millis = millis();
        sys_cache.loop_counter++;

#if defined(CAPTURE_CORE_STATUS)
        unsigned long core_status_millis = millis();
        cache_core_status();
        sys_cache.core_status_millis = millis() - core_status_millis;
#endif

#if defined(USE_GPS)
        unsigned long gps_millis = millis();
        loop_g();
        sys_cache.gps_millis = millis() - gps_millis;
#endif

#if defined(USE_MPU6050)
        unsigned long mpu6050_millis = millis();
        loop_mpu6050();
        sys_cache.mpu6050_millis = millis() - mpu6050_millis;
#endif

#if defined(USE_BMP280)
        unsigned long baro_millis = millis();
        loop_bmp280_by_time();
        sys_cache.baro_millis = millis() - baro_millis;
#endif

        // Converter para task de baixa prioridade no core 1

#if defined(USE_SCREEN)
        unsigned long screen_millis = millis();
        update_screen_a();
        sys_cache.screen_millis = millis() - screen_millis;
#endif

#if defined(USE_WEBSERVER)
        unsigned long handle_client_time = millis();
        handle_client();
        sys_cache.handle_client_millis = millis() - handle_client_time;
#endif

        next_loop_millis = millis() + 100;                      // Define proximo loop para 1 segundo
        sys_cache.loop_millis = millis() - initial_loop_millis; // Calcula tempo de execucao do loop
    }

    struct ENCODER_Motion *ENCODER_Received;
    if (xQueueReceive(xQueueMPU6050Metrics, &(ENCODER_Received), 0))
    {
        mpu_cache.temp = ENCODER_Received->temp;
        mpu_cache.ax = ENCODER_Received->ax;
        mpu_cache.ay = ENCODER_Received->ay;
        mpu_cache.az = ENCODER_Received->az;
        mpu_cache.gx = ENCODER_Received->gx;
        mpu_cache.gy = ENCODER_Received->gy;
        mpu_cache.gz = ENCODER_Received->gz;
        mpu_cache.delta_z = ENCODER_Received->delta_z;
    }
}
