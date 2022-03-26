#include <FreeRTOS.h>

#include <flight_companion/config.hpp>
#include <flight_companion/queue.hpp>
#include <flight_companion/screen.hpp>
#include <flight_companion/copilot.hpp>

struct gnss_struct gnss_data;
struct baro_struct baro_data;

void copilot_task(void *pvParameters)
{
    Serial.println("[COPILOT] task started");
    for (;;)
    {
        int qtd = uxQueueMessagesWaiting(xQueueGNSSMetrics);
        if (qtd > 0)
        {
#ifdef XDEBUG
            Serial.printf("[COPILOT] xQueueGNSSMetrics has %i messages\n", qtd);
#endif
            struct gnss_struct indata;

            if (xQueueReceive(xQueueGNSSMetrics, &indata, portMAX_DELAY) == pdTRUE)
            {
                gnss_data = indata;
            }
        }

        qtd = uxQueueMessagesWaiting(xQueueBaro);
        if (qtd > 0)
        {
            struct baro_struct indata;

            if (xQueueReceive(xQueueBaro, &indata, portMAX_DELAY) == pdTRUE)
            {
                baro_data = indata;
            }
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

    setMetric2(&p, "baro_temperature", String(baro_data.temperature_avg));
    setMetric2(&p, "baro_pressure", String(baro_data.pressure_avg));
    setMetric2(&p, "baro_altitude", String(baro_data.altitude_avg));

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