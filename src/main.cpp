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

#include <flight_display/queue.hpp>
#include <flight_display/display.hpp>
#include <model/espnow_message.hpp>

TaskHandle_t DisplayTaskHandler;

#define BUTTON_1 34
#define BUTTON_2 35
#define BUTTON_3 39

TwoWire I2CBME = TwoWire(0);

int state = 0;

uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0x84, 0xDC, 0x48};
esp_now_peer_info_t peerInfo;

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    // Envia dados de entrada para fila para que possam
    // ser consumidos pela task de display
    xQueueSendToBack(xQueueMetrics, incomingData, 10);
}

void setup()
{
    setCpuFrequencyMhz(80);

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
}

int one_second = 1000;
int one_minute = 60000;

unsigned long elapsed_time_100_milli = millis();
unsigned long elapsed_time_1_second = millis();
unsigned long elapsed_time_1_minute = millis();

void loop()
{
    collect_metrics();
    display_mutable_elements();

    // if (elapsed_time_100_milli < millis())
    // {
    //     elapsed_time_100_milli = millis() + 100;
    // }

    // if (elapsed_time_1_second < millis())
    // {
    //     elapsed_time_1_second = millis() + one_second;
    // }

    // if (elapsed_time_1_minute < millis())
    // {
    //     calc_battery();
    //     elapsed_time_1_minute = millis() + one_minute; // 1 min
    // }
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
    setCpuFrequencyMhz(80);

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

    xTaskCreatePinnedToCore(copilot_task, "copilot_task", 5000, NULL, COPI_TASK_PRIORITY, &CopilotTaskHandler, 0);
    xTaskCreatePinnedToCore(gnss_task, "gnss_task", 5000, NULL, GNSS_TASK_PRIORITY, &GNSSTaskHandler, CORE_1);

    if (component_status[BARO])
    {
        // Se o barometro não for detectado, não habilita tarefas
        // do buzzer nem do barometro
        // xTaskCreatePinnedToCore(buzzer_task, "bssuzzer_task", 1024, NULL, BUZZ_TASK_PRIORITY, &BuzzerTaskHandler, CORE_1);
        xTaskCreatePinnedToCore(baro_task, "baro_task", 2048, NULL, BARO_TASK_PRIORITY, &BaroTaskHandler, CORE_1);
    }

    xTaskCreatePinnedToCore(accel_task, "accel_task", 2048, NULL, ACEL_TASK_PRIORITY, &AccelTaskHandler, CORE_1);

// Configura o WIFI ou ESP NOW
#if defined(USE_ESPNOW)
    setup_esp_now();
#else
    connect_wifi();
    // Levanta servidor HTTP interno
    config_web_server();
#endif
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

// #if defined(ESP32_SDCARD)
// /*
//   Rui Santos
//   Complete project details at https://RandomNerdTutorials.com/esp32-microsd-card-arduino/

//   This sketch can be found at: Examples > SD(esp32) > SD_Test
// */

// #include "FS.h"
// #include "SD.h"
// #include "SPI.h"

// void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
// {
//     Serial.printf("Listing directory: %s\n", dirname);

//     File root = fs.open(dirname);
//     if (!root)
//     {
//         Serial.println("Failed to open directory");
//         return;
//     }
//     if (!root.isDirectory())
//     {
//         Serial.println("Not a directory");
//         return;
//     }

//     File file = root.openNextFile();
//     while (file)
//     {
//         if (file.isDirectory())
//         {
//             Serial.print("  DIR : ");
//             Serial.println(file.name());
//             if (levels)
//             {
//                 listDir(fs, file.name(), levels - 1);
//             }
//         }
//         else
//         {
//             Serial.print("  FILE: ");
//             Serial.print(file.name());
//             Serial.print("  SIZE: ");
//             Serial.println(file.size());
//         }
//         file = root.openNextFile();
//     }
// }

// void createDir(fs::FS &fs, const char *path)
// {
//     Serial.printf("Creating Dir: %s\n", path);
//     if (fs.mkdir(path))
//     {
//         Serial.println("Dir created");
//     }
//     else
//     {
//         Serial.println("mkdir failed");
//     }
// }

// void removeDir(fs::FS &fs, const char *path)
// {
//     Serial.printf("Removing Dir: %s\n", path);
//     if (fs.rmdir(path))
//     {
//         Serial.println("Dir removed");
//     }
//     else
//     {
//         Serial.println("rmdir failed");
//     }
// }

// void readFile(fs::FS &fs, const char *path)
// {
//     Serial.printf("Reading file: %s\n", path);

//     File file = fs.open(path);
//     if (!file)
//     {
//         Serial.println("Failed to open file for reading");
//         return;
//     }

//     Serial.print("Read from file: ");
//     while (file.available())
//     {
//         Serial.write(file.read());
//     }
//     file.close();
// }

// void writeFile(fs::FS &fs, const char *path, const char *message)
// {
//     Serial.printf("Writing file: %s\n", path);

//     File file = fs.open(path, FILE_WRITE);
//     if (!file)
//     {
//         Serial.println("Failed to open file for writing");
//         return;
//     }
//     if (file.print(message))
//     {
//         Serial.println("File written");
//     }
//     else
//     {
//         Serial.println("Write failed");
//     }
//     file.close();
// }

// void appendFile(fs::FS &fs, const char *path, const char *message)
// {
//     Serial.printf("Appending to file: %s\n", path);

//     File file = fs.open(path, FILE_APPEND);
//     if (!file)
//     {
//         Serial.println("Failed to open file for appending");
//         return;
//     }
//     if (file.print(message))
//     {
//         Serial.println("Message appended");
//     }
//     else
//     {
//         Serial.println("Append failed");
//     }
//     file.close();
// }

// void renameFile(fs::FS &fs, const char *path1, const char *path2)
// {
//     Serial.printf("Renaming file %s to %s\n", path1, path2);
//     if (fs.rename(path1, path2))
//     {
//         Serial.println("File renamed");
//     }
//     else
//     {
//         Serial.println("Rename failed");
//     }
// }

// void deleteFile(fs::FS &fs, const char *path)
// {
//     Serial.printf("Deleting file: %s\n", path);
//     if (fs.remove(path))
//     {
//         Serial.println("File deleted");
//     }
//     else
//     {
//         Serial.println("Delete failed");
//     }
// }

// void testFileIO(fs::FS &fs, const char *path)
// {
//     File file = fs.open(path);
//     static uint8_t buf[512];
//     size_t len = 0;
//     uint32_t start = millis();
//     uint32_t end = start;
//     if (file)
//     {
//         len = file.size();
//         size_t flen = len;
//         start = millis();
//         while (len)
//         {
//             size_t toRead = len;
//             if (toRead > 512)
//             {
//                 toRead = 512;
//             }
//             file.read(buf, toRead);
//             len -= toRead;
//         }
//         end = millis() - start;
//         Serial.printf("%u bytes read for %u ms\n", flen, end);
//         file.close();
//     }
//     else
//     {
//         Serial.println("Failed to open file for reading");
//     }

//     file = fs.open(path, FILE_WRITE);
//     if (!file)
//     {
//         Serial.println("Failed to open file for writing");
//         return;
//     }

//     size_t i;
//     start = millis();
//     for (i = 0; i < 2048; i++)
//     {
//         file.write(buf, 512);
//     }
//     end = millis() - start;
//     Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
//     file.close();
// }

// void setup()
// {
//     Serial.begin(9600);
//     if (!SD.begin(5))
//     {
//         Serial.println("Card Mount Failed");
//         return;
//     }
//     uint8_t cardType = SD.cardType();

//     if (cardType == CARD_NONE)
//     {
//         Serial.println("No SD card attached");
//         return;
//     }

//     Serial.print("SD Card Type: ");
//     if (cardType == CARD_MMC)
//     {
//         Serial.println("MMC");
//     }
//     else if (cardType == CARD_SD)
//     {
//         Serial.println("SDSC");
//     }
//     else if (cardType == CARD_SDHC)
//     {
//         Serial.println("SDHC");
//     }
//     else
//     {
//         Serial.println("UNKNOWN");
//     }

//     uint64_t cardSize = SD.cardSize() / (1024 * 1024);
//     Serial.printf("SD Card Size: %lluMB\n", cardSize);

//     listDir(SD, "/", 0);
//     createDir(SD, "/mydir");
//     listDir(SD, "/", 0);
//     removeDir(SD, "/mydir");
//     listDir(SD, "/", 2);
//     writeFile(SD, "/hello.txt", "Hello ");
//     appendFile(SD, "/hello.txt", "World!\n");
//     readFile(SD, "/hello.txt");
//     deleteFile(SD, "/foo.txt");
//     renameFile(SD, "/hello.txt", "/foo.txt");
//     readFile(SD, "/foo.txt");
//     testFileIO(SD, "/test.txt");
//     Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
//     Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
// }

// void loop()
// {
// }

// #endif