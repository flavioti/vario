#include <time.h>
#include <config.h>

#include <TinyGPS++.h>
#include <SparkFun_Ublox_Arduino_Library.h>

TinyGPSPlus gps;
// HardwareSerial GPS(1);

SFE_UBLOX_GPS myGPS;
HardwareSerial SerialGPS(1);

int state = 0;

#define RX_PIN 16 // RX2
#define TX_PIN 17 // TX2

void setup_gnss_old()
{
  Serial.println("setup_gnss_old");
  SerialGPS.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN, false);
}

void loop_g()
{
  switch (state)
  {
  case 0: // soft solution, should be sufficient and works in most (all) cases
    do
    {
      Serial.println("[GNSS] soft solution");
      if (myGPS.begin(SerialGPS))
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
    SerialGPS.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN, false);
    if (myGPS.begin(SerialGPS))
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
    SerialGPS.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN, false);
    if (myGPS.begin(SerialGPS))
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
    while (SerialGPS.available())
    {
      char c = SerialGPS.read();
      gps.encode(c);
      Serial.print(c);
    }

    Serial.println("Sat\tAlt");
    Serial.printf("%i\t%f\t\n", gps.satellites.value(), gps.altitude.meters());
  }
}
