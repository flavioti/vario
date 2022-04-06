#include <FreeRTOS.h>

#include <TinyGPS++.h>
#include <SparkFun_Ublox_Arduino_Library.h>

#include <flight_companion/config.hpp>
#include <flight_companion/queue.hpp>

HardwareSerial SerialGNSS(1);
// SFE_UBLOX_GPS gnss_manager;

void setup_gnss()
{
    Wire.begin(21, 22);
    SerialGNSS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.println("\n[GNSS] baud rate.............: " + String(SerialGNSS.baudRate()));
    Serial.println("[GNSS] availability check....: " + SerialGNSS.available() > 0 ? "OK" : "FAILED");
}

// Se o GPS não estiver funcionando
// Tem um codigo na pasta de backup com exemplos de como configurar o GPS
void gnss_task(void *pvParameters)
{
    // Aguarda execução para aguadar leitura dos dispositivos
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    Serial.println("[GNSS] task..................: RUNNING");
    TinyGPSPlus gnss_parser;
    for (;;)
    {
        while (SerialGNSS.available())
        {
            char c = SerialGNSS.read();
            gnss_parser.encode(c);
        }

        struct gnss_struct data;

        data.millis = millis();

        if ((gnss_parser.satellites.isUpdated()) and gnss_parser.satellites.isValid())
        {
            data.sat_count = gnss_parser.satellites.value();
        }

        if ((gnss_parser.location.isUpdated()) and gnss_parser.location.isValid())
        {
            data.location_lat = gnss_parser.location.lat();
            data.location_lng = gnss_parser.location.lng();
        }

        if ((gnss_parser.altitude.isUpdated()) and gnss_parser.altitude.isValid())
        {
            data.altitude_meters = gnss_parser.altitude.meters();
        }

        if ((gnss_parser.speed.isUpdated()) and gnss_parser.speed.isValid())
        {
            data.speed_kmph = gnss_parser.speed.kmph();
        }

        if ((gnss_parser.course.isUpdated()) and gnss_parser.course.isValid())
        {
            data.course_value = gnss_parser.course.value();
            data.course_deg = gnss_parser.course.deg();
            data.course_cardinal = gnss_parser.cardinal(data.course_deg);
        }

        if ((gnss_parser.date.isUpdated()) and gnss_parser.date.isValid())
        {
            data.date = gnss_parser.date.value();
        }

        if ((gnss_parser.time.isUpdated()) and gnss_parser.time.isValid())
        {
            data.time = gnss_parser.time.value();
        }

        if ((gnss_parser.hdop.isUpdated()) and gnss_parser.hdop.isValid())
        {
            // Diluição da Precisão Horizontal
            data.hdop = gnss_parser.hdop.value();
        }

        xQueueSendToBack(xQueueGNSSMetrics, &data, (TickType_t)0);

#ifdef XDEBUG
        int qtd = uxQueueMessagesWaiting(xQueueGNSSMetrics);
        Serial.printf("[GNSS] xQueueGNSSMetrics has %i messages\n", qtd);
#endif

        vTaskDelay(GNSS_READ_RATE / portTICK_PERIOD_MS);
    } // for
}