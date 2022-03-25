#include <FreeRTOS.h>

#include <flight_companion/config.hpp>
#include <flight_companion/queue.hpp>

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
            struct sensor element;

            BaseType_t ret = xQueueReceive(xQueueGNSSMetrics, &element, portMAX_DELAY);
            if (ret == pdTRUE)
            {
                Serial.println("[COPILOT] Sat\tAlt");
                Serial.printf("\t%i\t%f\t\n", element.sat, element.value);
            }
        }

        vTaskDelay(COPILOT_READ_RATE / portTICK_PERIOD_MS);
    }
}