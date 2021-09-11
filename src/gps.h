#include <TinyGPS++.h>
#include <config.h>
#include <time.h>

TinyGPSPlus gps;
HardwareSerial GPS(1);

void init_gps()
{
  Serial.begin(9600);
  Wire.begin(21, 22);
  GPS.begin(9600, SERIAL_8N1, 34, 12); //17-TX 18-RX
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GPS.available())
      gps.encode(GPS.read());
  } while (millis() - start < ms);
}

void gps_loop()
{
  Serial.print("GPS Latitude  : ");
  Serial.println(gps.location.lat(), 5);
  Serial.print("GPS Longitude : ");
  Serial.println(gps.location.lng(), 4);
  Serial.print("GPS Satellites: ");
  Serial.println(gps.satellites.value());
  Serial.print("GPS Altitude  : ");
  Serial.print(gps.altitude.feet() / 3.2808);
  Serial.println("M");
  if (gps.date.isValid())
  {
    Serial.printf("GPS Timestamp : %d-%d-%dT%d:%d:%dZ\n",
                  gps.date.year(),
                  gps.date.month(),
                  gps.date.day(),
                  gps.time.hour(),
                  gps.time.minute(),
                  gps.time.second());
  }
  Serial.print("GPS Speed     : ");
  Serial.println(gps.speed.kmph());

  smartDelay(1000);

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}
