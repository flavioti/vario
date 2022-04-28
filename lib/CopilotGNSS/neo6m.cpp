#include <FreeRTOS.h>

#include <TinyGPS++.h>
#include <SparkFun_Ublox_Arduino_Library.h>

#include <neo6m.hpp>

HardwareSerial SerialGNSS(1);
TinyGPSPlus gnss_parser;
SFE_UBLOX_GPS myGPS;

#define RX_PIN 16 // RX2
#define TX_PIN 17 // TX2

void setup_gnss()
{
    SerialGNSS.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN, false);
    Serial.println("\n[GNSS] baud rate.............: " + String(SerialGNSS.baudRate()));
    Serial.println("[GNSS] availability check....: " + SerialGNSS.available() > 0 ? "OK" : "FAILED");
}

int state = 3;
unsigned long reset_gnss_control = millis();

gnss_struct_t read_gnss()
{
    switch (state)
    {
    case 0: // soft solution, should be sufficient and works in most (all) cases
        do
        {
            Serial.println("[GNSS] soft solution");
            if (myGPS.begin(SerialGNSS))
            {
                Serial.println("[NEO6M] status ........................: OK");
                myGPS.setUART1Output(COM_TYPE_NMEA); // Set the UART port to output NMEA only
                myGPS.saveConfiguration();           // Save the current settings to flash and BBR
                // GPS serial connected, output set to NMEA
                myGPS.disableNMEAMessage(UBX_NMEA_GLL, COM_PORT_UART1);
                myGPS.disableNMEAMessage(UBX_NMEA_GSA, COM_PORT_UART1);
                myGPS.disableNMEAMessage(UBX_NMEA_GSV, COM_PORT_UART1);
                myGPS.disableNMEAMessage(UBX_NMEA_VTG, COM_PORT_UART1);
                myGPS.disableNMEAMessage(UBX_NMEA_RMC, COM_PORT_UART1);
                myGPS.enableNMEAMessage(UBX_NMEA_GGA, COM_PORT_UART1);
                myGPS.saveConfiguration(); // Save the current settings to flash and BBR
                break;
            }
        } while (1);
        Serial.println("GPS Saved config");
        state++;
        break;

    case 1: // hardReset
        Serial.println("GPS Issuing hardReset (cold start)");
        myGPS.hardReset();
        delay(1000);
        SerialGNSS.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN, false);
        if (myGPS.begin(SerialGNSS))
        {
            Serial.println("GPS Success.");
            state++;
        }
        else
        {
            Serial.println("*** GPS did not respond, starting over.");
            state = 0;
        }
        break;

    case 2: // factoryReset, expect to see GPS back at 9600 baud
        Serial.println("Issuing factoryReset");
        myGPS.factoryReset();
        delay(1000); // takes more than one second... a loop to resync would be best
        SerialGNSS.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN, false);
        if (myGPS.begin(SerialGNSS))
        {
            Serial.println("GPS Success, gps has been reset with factory settings");
            state++;
        }
        else
        {
            Serial.println("*** GPS did not respond, starting over.");
            state = 0;
        }
        break;

    case 3:

        // Caso tenha passado 5 minutos desde o último dado válido (satelites > 0) reseta o GPS
        if ((reset_gnss_control + 300000) < millis())
        {
            reset_gnss_control = millis();
            state = 0;
            break;
        }

        while (SerialGNSS.available())
        {
            char c = SerialGNSS.read();
            gnss_parser.encode(c);
            // Serial.print(c);
        }

        struct gnss_struct_t data;
        data.sat_count = gnss_parser.satellites.value();
        data.location_lat = gnss_parser.location.lat();
        data.location_lng = gnss_parser.location.lng();
        data.altitude_meters = gnss_parser.altitude.meters();
        data.speed_kmph = gnss_parser.speed.kmph();
        data.course_value = gnss_parser.course.value();
        data.course_deg = gnss_parser.course.deg();
        data.course_cardinal = gnss_parser.cardinal(data.course_deg);
        data.date = gnss_parser.date.value();
        data.time = gnss_parser.time.value();
        // Diluição da Precisão Horizontal
        data.hdop = gnss_parser.hdop.value();

        return data;
    }
}
