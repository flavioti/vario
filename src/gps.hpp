#include <TinyGPS++.h>
#include <axp20x.h>
#include <config.h>
#include <time.h>
#include <cache_global.hpp>

TinyGPSPlus gps;
HardwareSerial GPS(1);

#include <SparkFun_Ublox_Arduino_Library.h>
SFE_UBLOX_GPS myGPS;
int state = 0; // steps through states
HardwareSerial SerialGPS(1);

void setup_g()
{
  while (!Serial)
    ; // Wait for user to open the terminal
  Serial.println("Connected to Serial");
  Wire.begin(I2C_SDA, I2C_SCL);

  SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("All comms started");
  delay(100);
}

void loop_g()
{
  Serial.println();
  Serial.print("===== STATE ");
  Serial.println(state);
  switch (state)
  {
  case 0: // soft solution, should be sufficient and works in most (all) cases
    do
    {
      // Serial.println("0");
      if (myGPS.begin(SerialGPS))
      {
        Serial.println("Connected to GPS");
        myGPS.setUART1Output(COM_TYPE_NMEA); //Set the UART port to output NMEA only
        myGPS.saveConfiguration();           //Save the current settings to flash and BBR
        Serial.println("GPS serial connected, output set to NMEA");
        myGPS.disableNMEAMessage(UBX_NMEA_GLL, COM_PORT_UART1);
        myGPS.disableNMEAMessage(UBX_NMEA_GSA, COM_PORT_UART1);
        myGPS.disableNMEAMessage(UBX_NMEA_GSV, COM_PORT_UART1);
        myGPS.disableNMEAMessage(UBX_NMEA_VTG, COM_PORT_UART1);
        myGPS.disableNMEAMessage(UBX_NMEA_RMC, COM_PORT_UART1);
        myGPS.enableNMEAMessage(UBX_NMEA_GGA, COM_PORT_UART1);
        myGPS.saveConfiguration(); //Save the current settings to flash and BBR
        break;
      }
      delay(1000);
    } while (1);
    Serial.println("GPS Saved config");
    state++;
    break;

  case 1: // hardReset
    Serial.println("GPS Issuing hardReset (cold start)");
    myGPS.hardReset();
    delay(3000);
    SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
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
    delay(3000); // takes more than one second... a loop to resync would be best
    SerialGPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    if (myGPS.begin(SerialGPS))
    {
      Serial.println("GPS Success, gps has been reset with factory settings");
      Serial.println();
      state++;
    }
    else
    {
      Serial.println("*** GPS did not respond, starting over.");
      state = 0;
    }
    break;

  case 3:
    // Iteração para coletar dados do GPS e carrega-los no tinygps
    while (SerialGPS.available())
    {
      char c = SerialGPS.read();
      gps.encode(c);
      // Serial.write(c); // print anything comes in from the GPS
    }
    geo_cache.latitude = gps.location.lat();
    geo_cache.longitude = gps.location.lng();
    geo_cache.altitude = gps.altitude.meters();
    geo_cache.satellites = gps.satellites.value();

#if defined(GPS_LOG_ENABLED)
    Serial.printf("GLO = %f *C\n", geo_cache.longitude);
    Serial.printf("GLA = %f\n", geo_cache.latitude);
    Serial.printf("GAL = %f\n", geo_cache.altitude);
    Serial.printf("GSA = %i\n", geo_cache.satellites);
#endif
  }
}
