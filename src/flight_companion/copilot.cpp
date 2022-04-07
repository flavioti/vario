#include <FreeRTOS.h>

#include <WiFi.h>
#include <esp_now.h>

#include <flight_companion/config.hpp>
#include <flight_companion/queue.hpp>
#include <flight_companion/screen.hpp>
#include <flight_companion/copilot.hpp>
#include <model/espnow_message.hpp>

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
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    if (result != ESP_OK)
    {
        Serial.println("[COPILOT][ESPNOW] status.....: " + String(esp_err_to_name(result)));
    }
}

/////// COPILOT LOGIC

// TODO: Alterar o delay conforme a quantidade de mensagens na fila
void copilot_task(void *pvParameters)
{
    Serial.println("[COPILOT] task ..............: STARTED");
    // Aguarda execução para aguadar leitura dos dispositivos
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    for (;;)
    {
        try
        {
            if (uxQueueMessagesWaiting(xQueueBaro) > 0)
            {
#ifdef XDEBUG
                Serial.printf("[COPILOT] xQueueBaro has %i messages\n", uxQueueMessagesWaiting(xQueueBaro));
#endif
                struct baro_struct_t indata;
                if (xQueueReceive(xQueueBaro, &indata, portMAX_DELAY) == pdTRUE)
                {
                    baro_data = indata;
                }
            }

            if (uxQueueMessagesWaiting(xQueueGNSSMetrics) > 0)
            {
#ifdef XDEBUG
                Serial.printf("[COPILOT] xQueueGNSSMetrics has %i messages\n", uxQueueMessagesWaiting(xQueueGNSSMetrics));
#endif
                struct gnss_struct indata;
                if (xQueueReceive(xQueueGNSSMetrics, &indata, portMAX_DELAY) == pdTRUE)
                {
                    gnss_data = indata;
                }
            }

            if (uxQueueMessagesWaiting(xQueueBaro) > 0)
            {
                struct baro_struct_t indata;
                if (xQueueReceive(xQueueBaro, &indata, portMAX_DELAY) == pdTRUE)
                {
                    baro_data = indata;
                }
            }
            myData.baro_data.altitude = baro_data.altitude;
            myData.baro_data.temperature = baro_data.temperature;
            myData.baro_data.pressure = baro_data.pressure;
            myData.baro_data.vario = baro_data.vario;

            // BUZZER

            float vario = myData.baro_data.vario;
            {
                if ((vario <= VARIO_SINK_THRESHOLD_SINK || vario >= VARIO_SINK_THRESHOLD_LIFT))
                {
                    xQueueSendToBack((QueueHandle_t)xQueueBuzzer, &vario, (TickType_t)0);
                }
            }

#ifdef USE_ESPNOW
            if (PEER_ADDED)
            {
                send_esp_now();
            }
#endif
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        vTaskDelay(COPILOT_READ_RATE / portTICK_PERIOD_MS);
    }
}

// METRICAS PARA PROMETHEUS

void setMetric2(String *p, String metric, String value)
{
    *p += "# " + metric + "\n";
    *p += "# TYPE " + metric + " gauge\n";
    *p += "" + metric + " ";
    *p += value;
    *p += "\n";
}

String getMetrics()
{
    String p = ""; // acumulador de dados

    int sketch_size = ESP.getSketchSize();
    int flash_size = ESP.getFreeSketchSpace();
    int available_size = flash_size - sketch_size;

    setMetric2(&p, "esp32_uptime", String(millis()));
    setMetric2(&p, "esp32_heap_size", String(ESP.getHeapSize()));
    setMetric2(&p, "esp32_free_heap", String(xPortGetFreeHeapSize()));
    setMetric2(&p, "esp32_min_ever_free_heap", String(xPortGetMinimumEverFreeHeapSize()));
    setMetric2(&p, "esp32_sketch_size", String(sketch_size));
    setMetric2(&p, "esp32_flash_size", String(flash_size));
    setMetric2(&p, "esp32_available_size", String(available_size));

    // setMetric2(&p, "esp32_battery_voltage", String(sys_cache2.battery_voltage));
    // setMetric2(&p, "esp32_battery_percentage", String(sys_cache2.battery_percentage));
    // setMetric2(&p, "esp32_power_down_voltage", String(sys_cache2.power_down_voltage));

    setMetric2(&p, "baro_temperature", String(baro_data.temperature));
    setMetric2(&p, "baro_pressure", String(baro_data.pressure));
    setMetric2(&p, "baro_altitude", String(baro_data.altitude));

    setMetric2(&p, "gnss_altitude", String(gnss_data.altitude_meters));
    setMetric2(&p, "gnss_latitude", String(gnss_data.location_lat, 6));
    setMetric2(&p, "gnss_longitude", String(gnss_data.location_lng, 6));
    setMetric2(&p, "gnss_satellite", String(gnss_data.sat_count));
    setMetric2(&p, "gnss_millis", String(gnss_data.millis));
    setMetric2(&p, "gnss_speed", String(gnss_data.speed_kmph));
    setMetric2(&p, "gnss_hdop", String(gnss_data.hdop));
    setMetric2(&p, "gnss_date", String(gnss_data.date));
    setMetric2(&p, "gnss_time", String(gnss_data.time));

    // setMetric2(&p, "esp32_mpu_temp", String(mpu_cache.temp));
    // setMetric2(&p, "esp32_mpu_gx", String(mpu_cache.gx));
    // setMetric2(&p, "esp32_mpu_gy", String(mpu_cache.gy));
    // setMetric2(&p, "esp32_mpu_gz", String(mpu_cache.gz));
    // setMetric2(&p, "esp32_mpu_ax", String(mpu_cache.ax));
    // setMetric2(&p, "esp32_mpu_ay", String(mpu_cache.ay));
    // setMetric2(&p, "esp32_mpu_az", String(mpu_cache.az));
    // setMetric2(&p, "esp32_mpu_delta_z", String(mpu_cache.delta_z));

    return p;
}
