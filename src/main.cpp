#include <esp_now.h>

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
#include <esp_now.h>
#include <WiFi.h>

#define BATT_PIN 36
#define BUTTON_1 34
#define BUTTON_2 35
#define BUTTON_3 39

TwoWire I2CBME = TwoWire(0);

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

TwoWire I2C_0 = TwoWire(0);
#define I2C_Freq 100000
#define SDA_0 15
#define SCL_0 14

void find_i2c_devices()
{
    I2C_0.begin(SDA_0, SCL_0);

    Serial.println("\nI2C Scanner");
    byte error, address;
    int nDevices;
    Serial.println("Scanning...");
    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        delay(10);
        Serial.println(address);
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        I2C_0.beginTransmission(address);
        error = I2C_0.endTransmission();
        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.println("  !");
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknown error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");
    delay(5000); // wait 5 seconds for next scan
}

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp280;

#define I2C_SDA 14
#define I2C_SCL 15

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0x84, 0xDC, 0x48};
// 8C:AA:B5:84:DC:48

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message
{
    char a[32];
    int b;
    float c;
    bool d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    memcpy(&myData, incomingData, sizeof(myData));
    Serial.print("Bytes received: ");
    Serial.println(len);
    Serial.print("Char: ");
    Serial.println(myData.a);
    Serial.print("Int: ");
    Serial.println(myData.b);
    Serial.print("Float: ");
    Serial.println(myData.c);
    Serial.print("Bool: ");
    Serial.println(myData.d);
    Serial.println();

    Rect_t area1 = {
        .x = 50,
        .y = 50,
        .width = 100 - 20,
        .height = 100 / 2 + 80};

    epd_clear_area(area1);

    cursor_x = 50;
    cursor_y = 50;

    write_string((GFXfont *)&FiraSans, (char *)"Teste fghdfgdf", &cursor_x, &cursor_y, NULL);
}

void setup()
{
    sleep(2);
    Serial.begin(9600);
    while (!Serial)
        sleep(1);

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Once ESPNow is successfully Init, we will register for recv CB to
    // get recv packer info
    esp_now_register_recv_cb(OnDataRecv);

    // find_i2c_devices();

    // Set device as a Wi-Fi Station
    // WiFi.mode(WIFI_STA);
    // WiFi.disconnect();
    // Serial.println(WiFi.macAddress());

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

    // Draw Box
    epd_draw_rect(600, 450, 120, 60, 0, framebuffer);
    cursor_x = 615;
    cursor_y = 490;
    writeln((GFXfont *)&FiraSans, "Prev", &cursor_x, &cursor_y, framebuffer);

    epd_draw_rect(740, 450, 120, 60, 0, framebuffer);
    cursor_x = 755;
    cursor_y = 490;
    writeln((GFXfont *)&FiraSans, "Próximo", &cursor_x, &cursor_y, framebuffer);

    Rect_t area = {
        .x = 160,
        .y = 420,
        .width = lilygo_width,
        .height = lilygo_height};
    epd_copy_to_framebuffer(area, (uint8_t *)lilygo_data, framebuffer);

    epd_draw_rect(10, 20, EPD_WIDTH - 20, EPD_HEIGHT / 2 + 80, 0, framebuffer);

    epd_draw_grayscale_image(epd_full_screen(), framebuffer);

    Serial.println("EPD power off");
    epd_poweroff();
}

void loop()
{
    btn1.loop();
    btn2.loop();
    btn3.loop();
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
#include <flight_companion/cache_global.hpp>
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
bool component_status[3] = {false, false, false};

TaskHandle_t CopilotTaskHandler;
TaskHandle_t BuzzerTaskHandler;
TaskHandle_t BaroTaskHandler;
TaskHandle_t AccelTaskHandler;
TaskHandle_t GNSSTaskHandler;

uint8_t broadcastAddress[] = {0x34, 0xAB, 0x95, 0x5D, 0x97, 0x18};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message
{
    char a[32];
    int b;
    float c;
    bool d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print("\r\nLast Packet Send Status:\t");
    if (status != ESP_NOW_SEND_SUCCESS)
    {
        Serial.println("Delivery Fail");
    }
}

void setup()
{
    // Aguarda 1 segundo para não bugar o texto do terminal
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    analogReadResolution(12);
    pinMode(BUZZER_PIN, OUTPUT);

    Serial.begin(9600);
    while (!Serial)
        vTaskDelay(10 / portTICK_PERIOD_MS);

    Serial.println("[CORE] Serial started");

    digitalWrite(BUZZER_PIN, LOW);

    bool OK = setup_screen();
    component_status[OLED] = OK;

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Init ESP-NOW
    if (esp_now_init() == ESP_OK)
    {
        Serial.println("[ESP-NOW] status.......................: OK");
    }
    else
    {
        Serial.println("[ESP-NOW] status.......................: FAILED");
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);

    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }

    // connect_wifi2();

    OK = init_bmp280();
    // component_status[BARO] = OK;
    component_status[BARO] = false;

    setup_mpu6050();
    setup_gnss();

    xTaskCreatePinnedToCore(copilot_task, "copilot_task", 5000, NULL, (2 | portPRIVILEGE_BIT), &CopilotTaskHandler, CORE_1);

    xTaskCreatePinnedToCore(gnss_task, "gnss_task", 5000, NULL, (2 | portPRIVILEGE_BIT), &GNSSTaskHandler, CORE_1);

    if (component_status[BARO])
    {
        // Se o barometro não for detectado, não habilita tarefas
        // do buzzer nem do barometro
        xTaskCreatePinnedToCore(buzzer_task, "buzzer_task", 1024, NULL, 10, &BuzzerTaskHandler, CORE_1);
        xTaskCreatePinnedToCore(baro_task, "baro_task", 2048, NULL, (2 | portPRIVILEGE_BIT), &BaroTaskHandler, CORE_1);
    }

    xTaskCreatePinnedToCore(accel_task, "accel_task", 2048, NULL, (2 | portPRIVILEGE_BIT), &AccelTaskHandler, CORE_1);

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

        // TESTE ESP NOW

        // Set values to send
        strcpy(myData.a, "THIS IS A CHAR");
        myData.b = random(1, 20);
        myData.c = 1.2;
        myData.d = false;

        // Send message via ESP-NOW
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

        if (result == ESP_OK)
        {
            Serial.println("Sent with success");
        }
        else
        {
            Serial.println("Error sending the data");
        }

    } // TIMED LOOP
}

#endif
