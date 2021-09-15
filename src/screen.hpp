#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void init_screen()
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.display(); // Show initial text
}

void update_screen_a()
{
  display.clearDisplay();
  display.setTextSize(1); // 1 = 6x8, 2 = 12x16, 3 = 18x24
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(baro_cache.temperature, 1);
  display.print(" c      ");
  display.setTextSize(1);
  display.print(baro_cache.pressure, 1);
  display.println(" hpa");
  display.setTextSize(2);
  display.setCursor(0, 30);
  display.print(baro_cache.altitude, 1);
  display.println(" m");
  display.display();
}
