#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <flight_companion/config.hpp>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

bool setup_screen()
{
    // Mesmo sem display conectado, essa função retorna verdadeiro... serve pra nada.
    bool display_status = display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    if (display_status)
    {
        Serial.println("[SSD1306] status ............: OK");

        // Tela padrão inicial
        display.display();
        display.clearDisplay();
        display.setTextSize(3); // 1 = 6x8, 2 = 12x16, 3 = 18x24
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("COISA");
        display.println("TECH");
        display.display();

        vTaskDelay(1000 / portTICK_PERIOD_MS);

        display.clearDisplay();
        display.setTextSize(1); // 1 = 6x8, 2 = 12x16, 3 = 18x24
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.printf("%s %s ", APP_NAME, APP_VERSION);
        display.display();
    }
    else
    {
        Serial.println("[SSD1306] status ............: FAILED");
    }

    return display_status;
}

void display_vario(float vario)
{
    display.clearDisplay();
    display.setTextSize(1); // 1 = 6x8, 2 = 12x16, 3 = 18x24
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(vario);
    display.display();
}
