#include <esp_now.h>

#if defined(LILYGO_T5_47)

#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc_cal.h"
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>

#include <flight_display/Button2.h>
#include <flight_display/lilygo.h>
#include <flight_display/logo.h>
#include <flight_display/queue.hpp>
#include <flight_display/display.hpp>
#include <model/espnow_message.hpp>

TaskHandle_t DisplayTaskHandler;

#define BUTTON_1 34
#define BUTTON_2 35
#define BUTTON_3 39

TwoWire I2CBME = TwoWire(0);

Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
Button2 btn3(BUTTON_3);

int state = 0;

uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0x84, 0xDC, 0x48};
esp_now_peer_info_t peerInfo;

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    // Envia dados de entrada para fila para que possam
    // ser consumidos pela task de display
    xQueueSend(xQueueMetrics, incomingData, 2);
}

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        sleep(1);

    setup_display();

    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);

    DrawLabelAndElement();

}

unsigned long next_run = millis();

void loop()
{
    btn1.loop();
    btn2.loop();
    btn3.loop();

    if (next_run < millis())
    {
        next_run = millis() + 1000;

        display_task();
    }
}

#endif

#if defined(ESP32_DEV_KIT)

#include <FreeRTOS.h>
#include <Arduino.h>
#include <esp32-hal-i2c.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <flight_companion/config.hpp>
#include <flight_companion/queue.hpp>
#include <flight_companion/screen.hpp>
#include <flight_companion/bmp280.hpp>
#include <flight_companion/buzzer.hpp>
#include <flight_companion/network.hpp>
#include <flight_companion/config.hpp>
#include <flight_companion/mpu6050.hpp>
#include <flight_companion/neo6m.hpp>
#include <flight_companion/copilot.hpp>

// STATUS

static int BARO = 0;
// static int ACEL = 1;
static int OLED = 2;
bool component_status[4] = {false, false, false};

TaskHandle_t CopilotTaskHandler;
TaskHandle_t BuzzerTaskHandler;
TaskHandle_t BaroTaskHandler;
TaskHandle_t AccelTaskHandler;
TaskHandle_t GNSSTaskHandler;

void print_diagnostics()
{
    Serial.println("[CORE] cpu frequency.........: " + String(ESP.getCpuFreqMHz()) + " MHz");
}

void setup()
{
    // Aguarda 1 segundo para não bugar o texto do terminal
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    Serial.begin(9600);
    while (!Serial) // Aguarda até que o serial esteja pronto
        vTaskDelay(10 / portTICK_PERIOD_MS);

    Serial.println("[CORE] Serial started");

    print_diagnostics();

    analogReadResolution(12);      // Define para resolução de 12 bits
    pinMode(BUZZER_PIN, OUTPUT);   // Define o pino do buzzer como saída
    pinMode(LED_BUILTIN, OUTPUT);  // Define o pino do LED como saída
    digitalWrite(BUZZER_PIN, LOW); // Desliga LED built-in

    // Dispositivos

    component_status[OLED] = setup_screen(); // Configura tela OLED
    component_status[BARO] = init_bmp280();  // Configura barômetro
    setup_mpu6050();                         // Configura aceletômetro
    setup_gnss();                            // Configura GNSS

    // Tarefas

    xTaskCreatePinnedToCore(copilot_task, "copilot_task", 5000, NULL, (2 | portPRIVILEGE_BIT), &CopilotTaskHandler, 0);
    xTaskCreatePinnedToCore(gnss_task, "gnss_task", 5000, NULL, (2 | portPRIVILEGE_BIT), &GNSSTaskHandler, CORE_1);

    if (component_status[BARO])
    {
        // Se o barometro não for detectado, não habilita tarefas
        // do buzzer nem do barometro
        // xTaskCreatePinnedToCore(buzzer_task, "buzzer_task", 1024, NULL, 10, &BuzzerTaskHandler, CORE_1);
        xTaskCreatePinnedToCore(baro_task, "baro_task", 2048, NULL, (2 | portPRIVILEGE_BIT), &BaroTaskHandler, CORE_1);
    }

    xTaskCreatePinnedToCore(accel_task, "accel_task", 2048, NULL, (2 | portPRIVILEGE_BIT), &AccelTaskHandler, CORE_1);

// Configura o WIFI ou ESP NOW
#if defined(USE_ESPNOW)
    setup_esp_now();
#else
    connect_wifi();
#endif

    // Levanta servidor HTTP interno
    config_web_server();

    Serial.println("[CORE] setup done");
}

unsigned long min_next_loop_millis = 0;

char ptrTaskList[250];

void loop()
{
    if (millis() > min_next_loop_millis)
    {
#ifdef XDEBUG
        if (uxTaskGetNumberOfTasks() > 15)
        {
            Serial.printf("[CORE] Task count: %i\n", uxTaskGetNumberOfTasks());
        }
#endif
        min_next_loop_millis = millis() + 10000;
    } // TIMED LOOP

    // Configura o WIFI ou ESP NOW
#if not defined(USE_ESPNOW)
      // Response when a client access via http
    handle_client();
#endif
}

#endif
