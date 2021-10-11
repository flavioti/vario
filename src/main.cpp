#include <esp_now.h>

#if defined(TLORA_V1)

#include <FreeRTOS.h>
#include <Arduino.h>

#include <esp32-hal-i2c.h>
#include <TinyGPS++.h>

#include <flight_companion/config.hpp>
#include <flight_companion/cache_global.hpp>
#include <flight_companion/queue.hpp>
#include <flight_companion/mpu6050.hpp>
#include <flight_companion/screen.hpp>
#include <flight_companion/gps.hpp>
#include <flight_companion/bmp280.hpp>
#include <flight_companion/buzzer.hpp>
#include <flight_companion/energia.hpp>
#include <flight_companion/network.hpp>

TaskHandle_t BuzzerTaskHandler;
TaskHandle_t WelcomeTaskHandler;

void setup()
{
    esp_log_level_set("MAIN", ESP_LOG_VERBOSE);

    sleep(2);
    Serial.begin(9600);
    while (!Serial)
        ;
    ESP_LOGI(&TAG, "iniciando");
    configure_system();

    connect_wifi2();

    // Buzzer tasks
    xTaskCreatePinnedToCore(play_welcome_beep, "play_welcome_beep", 10000, NULL, 1, &WelcomeTaskHandler, CORE_1);
    xTaskCreatePinnedToCore(buzzer_task, "buzzer_task_core_0", 10000, NULL, 10, &BuzzerTaskHandler, CORE_1);

    setup_screen();
    setup_gnss();
    setup_mpu6050();
    init_bmp280();
    config_web_server();
}

unsigned long next_loop_millis = millis();

void loop()
{
    if (millis() > next_loop_millis)
    {
        cache_core_status();
        loop_g();
        loop_mpu6050();
        loop_bmp280_by_time();
        update_screen_a();
        handle_client();
        next_loop_millis = millis() + 100; // Define proximo loop para 1 segundo
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

#endif

#if defined(LILYGO_T5_47)

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "epd_driver.h"
#include "flight_display/firasans.h"
#include "esp_adc_cal.h"
#include "flight_display/Button2.h"
#include <Wire.h>
#include "flight_display/lilygo.h"
#include "flight_display/logo.h"
#include "WiFi.h"
#include <esp_now.h>

#define BATT_PIN 36
#define BUTTON_1 34
#define BUTTON_2 35
#define BUTTON_3 39

Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
Button2 btn3(BUTTON_3);

uint8_t *framebuffer;
int vref = 1100;
int cursor_x = 20;
int cursor_y = 60;
int state = 0;

Rect_t area1 = {
    .x = 10,
    .y = 20,
    .width = EPD_WIDTH - 20,
    .height = EPD_HEIGHT / 2 + 80};

const char *overview[] = {
    "   ESP32 is a single 2.4 GHz Wi-Fi-and-Bluetooth\n"
    "combo chip designed with the TSMC ultra-low-po\n"
    "wer 40 nm technology. It is designed to achieve \n"
    "the best power and RF performance, showing rob\n"
    "ustness versatility and reliability in a wide variet\n"
    "y of applications and power scenarios.\n",
    "➸ Xtensa® dual-core 32-bit LX6 microprocessor\n"
    "➸ 448 KB ROM & External 16MBytes falsh\n"
    "➸ 520 KB SRAM & External 16MBytes PSRAM\n"
    "➸ 16 KB SRAM in RTC\n"
    "➸ Multi-connections in Classic BT and BLE\n"
    "➸ 802.11 n (2.4 GHz), up to 150 Mbps\n",
    "➸ 16 color grayscale\n"
    "➸ Use with 4.7\" EPDs\n"
    "➸ High-quality font rendering\n"
    "➸ ~630ms for full frame draw\n"};

void displayInfo(void)
{
    cursor_x = 20;
    cursor_y = 60;
    state %= 4;
    switch (state)
    {
    case 0:
        epd_clear_area(area1);
        write_string((GFXfont *)&FiraSans, (char *)overview[0], &cursor_x, &cursor_y, NULL);
        break;
    case 1:
        epd_clear_area(area1);
        write_string((GFXfont *)&FiraSans, (char *)overview[1], &cursor_x, &cursor_y, NULL);
        break;
    case 2:
        epd_clear_area(area1);
        write_string((GFXfont *)&FiraSans, (char *)overview[2], &cursor_x, &cursor_y, NULL);
        break;
    case 3:
        delay(1000);
        epd_clear_area(area1);
        write_string((GFXfont *)&FiraSans, "DeepSleep", &cursor_x, &cursor_y, NULL);
        epd_poweroff_all();
        // Set to wake up by GPIO39
        esp_sleep_enable_ext1_wakeup(GPIO_SEL_39, ESP_EXT1_WAKEUP_ALL_LOW);
        esp_deep_sleep_start();
        break;
    case 4:
        break;
    default:
        break;
    }
    epd_poweroff();
}

void buttonPressed(Button2 &b)
{
    displayInfo();
    state++;
}

void setup()
{
    Serial.begin(9600);

    WiFi.mode(WIFI_MODE_STA);
    Serial.println(WiFi.macAddress());

    // esp_now_init();
    // esp_now_add_peer();
    // esp_now_send();

    // esp_now_register_rcv_cb();

    epd_init();

    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer)
    {
        Serial.println("alloc memory failed !!!");
        while (1)
            ;
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

    btn1.setPressedHandler(buttonPressed);
    btn2.setPressedHandler(buttonPressed);
    btn3.setPressedHandler(buttonPressed);

    epd_poweron();
    epd_clear();
    write_string((GFXfont *)&FiraSans, (char *)overview[0], &cursor_x, &cursor_y, framebuffer);

    //Draw Box
    epd_draw_rect(600, 450, 120, 60, 0, framebuffer);
    cursor_x = 615;
    cursor_y = 490;
    writeln((GFXfont *)&FiraSans, "Prev", &cursor_x, &cursor_y, framebuffer);

    epd_draw_rect(740, 450, 120, 60, 0, framebuffer);
    cursor_x = 755;
    cursor_y = 490;
    writeln((GFXfont *)&FiraSans, "Next", &cursor_x, &cursor_y, framebuffer);

    Rect_t area = {
        .x = 160,
        .y = 420,
        .width = lilygo_width,
        .height = lilygo_height};
    epd_copy_to_framebuffer(area, (uint8_t *)lilygo_data, framebuffer);

    epd_draw_rect(10, 20, EPD_WIDTH - 20, EPD_HEIGHT / 2 + 80, 0, framebuffer);

    epd_draw_grayscale_image(epd_full_screen(), framebuffer);

    epd_poweroff();
}

void loop()
{
    btn1.loop();
    btn2.loop();
    btn3.loop();
}

// 34:AB:95:5D:97:18 id da tela

#endif
