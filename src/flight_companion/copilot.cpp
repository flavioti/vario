#include <FreeRTOS.h>
#include <Arduino.h>
#include <esp32-hal-i2c.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <flight_companion/config.hpp>
#include <flight_companion/queue.hpp>
#include <flight_companion/screen.hpp>
#include <bmp280.hpp>
#include <flight_companion/buzzer.hpp>
#include <flight_companion/network.hpp>
#include <flight_companion/mpu6050.hpp>
#include <neo6m.hpp>
#include <flight_companion/copilot.hpp>
#include <CopilotSDCard.h>
#include <CopilotMessages.hpp>
// #include <gps.hpp>
#include <neo6m.hpp>

struct gnss_struct gnss_data;
struct baro_struct_t baro_data;

static bool PEER_ADDED = false;

/////// ESP-NOW

uint8_t broadcastAddress[] = {0x34, 0xAB, 0x95, 0x5D, 0x97, 0x18};

// Create a struct_message called myData
metrics_data myData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    // if (status == ESP_NOW_SEND_FAIL)
    // {
    //     Serial.println("[CORE][ESPNOW] data sent.....: FAILED");
    // }
    // else
    // {
    //     Serial.println("[CORE][ESPNOW] data sent.....: SUCCESS");
    // }
}

void setup_esp_now()
{
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Init ESP-NOW
    // Guru Meditation Error if wifi is connected
    if (esp_now_init() == ESP_OK)
    {
        PEER_ADDED = true;
        Serial.println("\n[COPILOT][ESPNOW] setup .....: OK");
    }
    else
    {
        Serial.println("\n[COPILOT][ESPNOW] setup .....: FAILED");
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    Serial.println("[ESP-NOW] callback...........: REGISTERED");
    esp_now_register_send_cb(OnDataSent);

    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    esp_err_t result = esp_now_add_peer(&peerInfo);
    Serial.println("[COPILOT][ESPNOW] peer status: " + String(esp_err_to_name(result)));
}

void send_esp_now()
{
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    if (result != ESP_OK)
    {
        Serial.println("[COPILOT][ESPNOW] status.....: " + String(esp_err_to_name(result)));
    }
}

void copilot_task(baro_struct_t baro_data, gnss_struct_t gnss_data)
{
    myData.gnss_data = gnss_data;
    myData.baro_data = baro_data;

    Serial.println("[COPILOT] " + myData.baro_data.toString());
    Serial.println("[COPILOT] " + myData.gnss_data.toString());

    // BUZZER

    float vario = myData.baro_data.vario;
    {
        if ((vario <= VARIO_SINK_THRESHOLD_SINK || vario >= VARIO_SINK_THRESHOLD_LIFT))
        {
            xQueueSendToBack((QueueHandle_t)xQueueBuzzer, &vario, (TickType_t)0);
        }
    }

    if (PEER_ADDED)
    {
        send_esp_now();
    }
}

void print_esp32_diagnostics()
{
    Serial.println("[CORE] esp cpu frequency ....: " + String(ESP.getCpuFreqMHz()) + " MHz");
    Serial.println("[CORE] esp chip cores .......: " + String(ESP.getChipCores()) + " core(s)");
    Serial.println("[CORE] esp chip model .......: " + String(ESP.getChipModel()));
    Serial.println("[CORE] esp chip revision ....: " + String(ESP.getChipRevision()));
    Serial.println("[CORE] esp cycle count ......: " + String(ESP.getCycleCount()));
    // Serial.println("[CORE] esp efuse mac ......: " + ESP.getEfuseMac()); // Causa kernel panic
    Serial.println("[CORE] esp flash chip mode ..: " + String(ESP.getFlashChipMode()));
    Serial.println("[CORE] esp flash chip size ..: " + String(ESP.getFlashChipSize() / 1024) + " kbytes");
    Serial.println("[CORE] esp flash chip speed .: " + String(ESP.getFlashChipSpeed() / 1000 / 1000) + " MHz");
    Serial.println("[CORE] esp heap size ........: " + String(ESP.getHeapSize() / 1024) + " kbytes");
    Serial.println("[CORE] esp sdk version.......: " + String(ESP.getSdkVersion()));
    Serial.println("[CORE] esp sketch size ......: " + String(ESP.getSketchSize() / 1024) + " kbytes");
}

void setup_copilot()
{
    setCpuFrequencyMhz(80);

    // Aguarda 1 segundo para não bugar o texto do terminal
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    Serial.begin(9600);
    while (!Serial) // Aguarda até que o serial esteja pronto
        vTaskDelay(10 / portTICK_PERIOD_MS);

    Serial.println("[CORE] Serial started");

    print_esp32_diagnostics();

    analogReadResolution(12);       // Define para resolução de 12 bits
    pinMode(BUZZER_PIN, OUTPUT);    // Define o pino do buzzer como saída
    pinMode(LED_BUILTIN, OUTPUT);   // Define o pino do LED como saída
    digitalWrite(BUZZER_PIN, HIGH); // LED built-in

    // Dispositivos

    initBuzzer();
    initSDCard();    // Configura o cartão SD
    initOLED();      // Configura tela OLED
    initBMP280();    // Configura barômetro
    setup_mpu6050(false); // Configura aceletômetro
    setup_gnss();    // Configura GNSS
    // setup_gnss_old();

    // Tarefas
    // xTaskCreatePinnedToCore(buzzer_task, "buzzer_task", 1024, NULL, BUZZ_TASK_PRIORITY, &BuzzerTaskHandler, CORE_1);

    setup_esp_now();

    Serial.println("[CORE] setup ................: FINISHED");
}

void loop_copilot()
{
    baro_struct_t baro_data = read_barometer();
    gnss_struct_t gnss_data = read_gnss();
    copilot_task(baro_data, gnss_data);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}