#include <FreeRTOS.h>
#include <Arduino.h>
#include "esp_adc_cal.h" // In-built

#include "epd_driver.h" // https://github.com/Xinyuan-LilyGO/LilyGo-EPD47

#include <flight_display/firasans.h> // application
#include <flight_display/queue.hpp>  // queues
#include <model/espnow_message.hpp>  // structures

uint8_t *framebuffer;
GFXfont currentFont;
long loop_count = 0;
enum alignment
{
    LEFT,
    RIGHT,
    CENTER
};
#define Black 0x00
int vref = 1100;
// fonts
#include "opensans8b.h"
#include "opensans10b.h"
#include "opensans12b.h"
#include "opensans18b.h"
#include "opensans24b.h"

void setFont(GFXfont const &font)
{
    currentFont = font;
}

void drawString(int x, int y, String text, alignment align, bool clearArea = true)
{
    char *data = const_cast<char *>(text.c_str());
    int x1, y1; // the bounds of x,y and w and h of the variable 'text' in pixels.
    int w, h;
    int xx = x, yy = y;
    get_text_bounds(&currentFont, data, &xx, &yy, &x1, &y1, &w, &h, NULL);
    if (align == RIGHT)
        x = x - w;
    if (align == CENTER)
        x = x - w / 2;
    int cursor_y = y + h;
    if (clearArea)
    {
        epd_clear_area({.x = x - 10,
                        .y = y - 10,
                        .width = w + 20,
                        .height = h + 20});
    }
    write_string(&currentFont, data, &x, &cursor_y, framebuffer);
}

void drawString(int x, int y, String text, alignment align, Rect_t toClear)
{
    char *data = const_cast<char *>(text.c_str());
    int x1, y1; // the bounds of x,y and w and h of the variable 'text' in pixels.
    int w, h;
    int xx = x, yy = y;
    get_text_bounds(&currentFont, data, &xx, &yy, &x1, &y1, &w, &h, NULL);
    if (align == RIGHT)
        x = x - w;
    if (align == CENTER)
        x = x - w / 2;
    int cursor_y = y + h;

    epd_clear_area(toClear);

    write_string(&currentFont, data, &x, &cursor_y, framebuffer);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool clearArea = false)
{
    if (clearArea)
    {
        epd_clear_area({.x = x,
                        .y = y,
                        .width = w,
                        .height = h});
    }
    epd_fill_rect(x, y, w, h, color, framebuffer);
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool clearArea = false)
{
    if (clearArea)
    {
        epd_clear_area({.x = x,
                        .y = y,
                        .width = w,
                        .height = h});
    }
    epd_draw_rect(x, y, w, h, color, framebuffer);
}

void DrawBattery(int x, int y)
{
    uint8_t percentage = 100;
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    {
        // Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
        vref = adc_chars.vref;
    }
    float voltage = analogRead(36) / 4096.0 * 6.566 * (vref / 1000.0);
    if (voltage > 1)
    { // Only display if there is a valid reading
        // Serial.println("\nVoltage = " + String(voltage));
        percentage = 2836.9625 * pow(voltage, 4) - 43987.4889 * pow(voltage, 3) + 255233.8134 * pow(voltage, 2) - 656689.7123 * voltage + 632041.7303;
        if (voltage >= 4.20)
            percentage = 100;
        if (voltage <= 3.20)
            percentage = 0; // orig 3.5
        drawRect(x + 25, y - 14, 40, 15, Black, true);
        fillRect(x + 65, y - 10, 4, 7, Black, true);
        fillRect(x + 27, y - 12, 36 * percentage / 100.0, 11, Black, true);
        drawString(x + 85, y - 14, String(percentage) + "%  " + String(voltage, 1) + "v", LEFT, true);
    }
}

void setup_display()
{
    epd_init();
    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer)
        Serial.println("Memory alloc failed!");
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

    epd_poweron();
    epd_clear();
}

void disp_loop_count(int x, int y)
{
    setFont(OpenSans18B);
    drawString(x, y, String(loop_count), LEFT, true);
}

// Desenha elementos gráficos permanentes
void DrawLabelAndElement()
{
    setFont(OpenSans12B);
    drawString(200, 50, "Altitude", LEFT, true);
    setFont(OpenSans12B);
    drawString(450, 100, "mts", RIGHT);
}

void DrawAltitude(float altitude)
{
    if (altitude > 0)
    {
        setFont(OpenSans24B);
        drawString(200, 100, String(altitude), LEFT);
    }
}

void playground()
{
}

// 4.7" e-paper display is 960x540 resolution
// void display_task(void *pvParameters)
void display_task()
{
    struct metrics_data incomingData;
    // Espera até 10 ticks para seguir o fluxo de execução
    if (xQueueReceive(xQueueMetrics, &incomingData, (TickType_t)100) == pdTRUE)
    {
        // Serial.println("Received data from queue");
        Serial.println(incomingData.altitude);
    }

    epd_poweron();

    loop_count++;

    DrawAltitude(incomingData.altitude);

    disp_loop_count(300, 300);

    setFont(OpenSans10B);
    DrawBattery(10, 20);

    epd_draw_grayscale_image(epd_full_screen(), framebuffer);

    epd_poweroff();

    // Serial.println(loop_count);
}
