#include <FreeRTOS.h>

#include <TinyGPS++.h>
#include <SparkFun_Ublox_Arduino_Library.h>

#include <neo6m.hpp>

HardwareSerial SerialGNSS(1);

void setup_gnss()
{
    Wire.begin(21, 22);
    SerialGNSS.begin(9600, SERIAL_8N1, 34, 12);
    Serial.println("\n[GNSS] baud rate.............: " + String(SerialGNSS.baudRate()));
    Serial.println("[GNSS] availability check....: " + SerialGNSS.available() > 0 ? "OK" : "FAILED");
}

// gnss_struct_t read_gnss()
// {
//     TinyGPSPlus gnss_parser;
//     while (SerialGNSS.available())
//     {
//         char c = SerialGNSS.read();
//         gnss_parser.encode(c);
//     }

//     struct gnss_struct_t data;

//     if ((gnss_parser.satellites.isUpdated()) and gnss_parser.satellites.isValid())
//     {
//         data.sat_count = gnss_parser.satellites.value();
//     }

//     if ((gnss_parser.location.isUpdated()) and gnss_parser.location.isValid())
//     {
//         data.location_lat = gnss_parser.location.lat();
//         data.location_lng = gnss_parser.location.lng();
//     }

//     if ((gnss_parser.altitude.isUpdated()) and gnss_parser.altitude.isValid())
//     {
//         data.altitude_meters = gnss_parser.altitude.meters();
//     }

//     if ((gnss_parser.speed.isUpdated()) and gnss_parser.speed.isValid())
//     {
//         data.speed_kmph = gnss_parser.speed.kmph();
//     }

//     if ((gnss_parser.course.isUpdated()) and gnss_parser.course.isValid())
//     {
//         data.course_value = gnss_parser.course.value();
//         data.course_deg = gnss_parser.course.deg();
//         data.course_cardinal = gnss_parser.cardinal(data.course_deg);
//     }

//     if ((gnss_parser.date.isUpdated()) and gnss_parser.date.isValid())
//     {
//         data.date = gnss_parser.date.value();
//     }

//     if ((gnss_parser.time.isUpdated()) and gnss_parser.time.isValid())
//     {
//         data.time = gnss_parser.time.value();
//     }

//     if ((gnss_parser.hdop.isUpdated()) and gnss_parser.hdop.isValid())
//     {
//         // Diluição da Precisão Horizontal
//         data.hdop = gnss_parser.hdop.value();
//     }

//     return data;
// }

TinyGPSPlus gps;
HardwareSerial GPS(1);

SFE_UBLOX_GPS myGPS;

int state = 1;

gnss_struct_t read_gnss()
{
    Serial.println("[GNSS] read_gnss");
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
        SerialGNSS.begin(9600, SERIAL_8N1, 34, 12);
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
        SerialGNSS.begin(9600, SERIAL_8N1, 34, 12);
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
        TinyGPSPlus gnss_parser;
        while (SerialGNSS.available())
        {
            char c = SerialGNSS.read();
            gnss_parser.encode(c);
        }

        struct gnss_struct_t data;

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

        return data;
    }
}