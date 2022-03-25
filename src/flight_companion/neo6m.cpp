#include <FreeRTOS.h>

#include <TinyGPS++.h>
#include <SparkFun_Ublox_Arduino_Library.h>

#include <flight_companion/config.hpp>
#include <flight_companion/queue.hpp>

HardwareSerial SerialGNSS(1);
TinyGPSPlus gnss_parser;
SFE_UBLOX_GPS gnss_manager;

void setup_gnss()
{
    Serial.println("setup_gnss");
    Wire.begin(21, 22);
    SerialGNSS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
}

// Se o GPS não estiver funcionando
// Tem um codigo na pasta de backup com exemplos de como configurar o GPS
void gnss_task(void *pvParameters)
{
    for (;;)
    {
        while (SerialGNSS.available())
        {
            char c = SerialGNSS.read();
            gnss_parser.encode(c);
        }

        if (gnss_parser.altitude.meters() > 0)
        {

            struct sensor mySensor;

            mySensor.deviceId = 55;
            mySensor.measurementType = 1;
            mySensor.value = gnss_parser.altitude.meters();
            mySensor.sat = gnss_parser.satellites.value();

            xQueueSend(xQueueGNSSMetrics, &mySensor, (TickType_t)0);

#ifdef XDEBUG
            int qtd = uxQueueMessagesWaiting(xQueueGNSSMetrics);
            Serial.printf("[GNSS] xQueueGNSSMetrics has %i messages\n", qtd);
#endif
        }

        vTaskDelay(GNSS_READ_RATE / portTICK_PERIOD_MS);
    } // for
}