#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void init_screen()
{
  // TODO Tentar detectar a tela 3 vezes antes de considerar que não foi detectada
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.display();
}

void update_screen_a()
{
  display.clearDisplay();
  display.setTextSize(1); // 1 = 6x8, 2 = 12x16, 3 = 18x24
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("BTE");
  display.print(baro_cache.temperature, 1);
  display.print(" ");

  display.print("BPR");
  display.print(baro_cache.pressure, 1);
  display.print(" ");

  display.print("BAL");
  display.print(baro_cache.altitude, 1);
  display.print(" ");

  display.print("GLO");
  display.print(geo_cache.longitude, 1); //8
  display.print(" ");

  display.print("GLA");
  display.print(geo_cache.latitude, 1); //8
  display.print(" ");

  display.print("GAL ");
  display.print(geo_cache.altitude, 1);
  display.print(" ");

  display.print("GSA");
  display.print(geo_cache.satellites, 1);
  display.print(" ");

  display.print("LOO");
  display.print(sys_cache.loop_counter);
  display.print(" ");

  display.print("SBV");
  display.print(sys_cache.battery_voltage);
  display.print(" ");

  display.print("SBP");
  display.print(sys_cache.battery_percentage);
  display.print(" ");

  display.display();
}
