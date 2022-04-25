// 4.7" e-paper display is 960x540 resolution

#include <FreeRTOS.h>
#include <Arduino.h>
#include <esp_log.h>
#include "esp_adc_cal.h" // In-built

#include "epd_driver.h" // https://github.com/Xinyuan-LilyGO/LilyGo-EPD47

#include <flight_display/queue.hpp>     // queues
#include <model/espnow_message.hpp>     // structures
#include <model/display_structures.hpp> // structures
#include <flight_display/font_files/opensans8.h>
#include <flight_display/font_files/opensans8b.h>
#include <flight_display/font_files/opensans10b.h>
#include <flight_display/font_files/opensans12b.h>
#include <flight_display/font_files/opensans18b.h>
#include <flight_display/font_files/opensans24b.h>
#include <flight_display/font_files/opensans26.h>
#include <flight_display/font_files/opensans26b.h>

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

void setFont(GFXfont const &font)
{
    currentFont = font;
}

void drawStringToFrameBuffer(int x, int y, String text, alignment align, bool clearArea = true)
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

void drawStringToFrameBuffer(int x, int y, String text, alignment align, Rect_t toClear)
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

void drawString(int x, int y, String text, alignment align)
{
    char *data = const_cast<char *>(text.c_str());
    int cursor_x = x;
    int cursor_y = y;
    write_string(&currentFont, data, &cursor_x, &cursor_y, NULL);
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

void DrawEspecificBattery(smart_int_type_struct &percentage, smart_float_type_struct &voltage)
{
    if (percentage.updated)
    {
        int perc = percentage.getValue();
        int x = 10;
        int y = 30;
        setFont(OpenSans10B);
        drawRect(x + 25, y - 14, 40, 15, Black, true);
        fillRect(x + 65, y - 10, 4, 7, Black, true);
        fillRect(x + 27, y - 12, 36 * perc / 100.0, 11, Black, true);
        drawString(x + 85, y - 14, String(perc) + "%  " + voltage.toString() + "v", LEFT);
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

FontProperties props = {
    .fg_color = 0,
    .bg_color = 15,
    .fallback_glyph = 0,
    .flags = DRAW_BACKGROUND};

void DrawSmartInt(int x, int y, int w, int h, smart_int_type_struct &field, bool redraw)
{
    if ((redraw) or (field.updated))
    {
        epd_clear_area({.x = x, .y = y - 50, .width = w, .height = h});

        setFont(OpenSans26);
        int cursor_x = x;
        int cursor_y = y;
        String text = field.toString();
        char *data = const_cast<char *>(text.c_str());
        // write_string(&currentFont, data, &cursor_x, &cursor_y, NULL);
        // writeln(&currentFont, data, &cursor_x, &cursor_y, NULL);
        write_mode(&currentFont, data, &cursor_x, &cursor_y, NULL, BLACK_ON_WHITE, NULL);
        // Serial.println("x " + String(aaa.x) + " y " + String(aaa.y) + " w " + String(aaa.width) + " h " + String(aaa.height));
        // ESP_LOGE("TTT", "SSSSSSSSSSSS");
    }
}

void DrawSmartFlt(int x, int y, int w, int h, smart_float_type_struct &field, bool redraw)
{
    if ((redraw) or (field.updated))
    {
        int offset = 50;
        Rect_t area = {.x = x, .y = y - offset, .width = w, .height = h};
        setFont(OpenSans26);
        int cursor_x = x;
        int cursor_y = y;
        String text = field.toString();
        char *data = const_cast<char *>(text.c_str());
        epd_clear_area(area);
        write_string(&currentFont, data, &cursor_x, &cursor_y, NULL);
    }
}

void DrawConnectionStatus(unsigned long last_data_received_millis)
{
    int x = 300;
    int y = 15;
    if ((millis() - last_data_received_millis) > 20000)
    {
        setFont(OpenSans10B);
        drawString(x, y, " Offline " + String((millis() - last_data_received_millis) / 1000) + "s", CENTER);
    }
    else
    {
        setFont(OpenSans10B);
        drawString(x, y, " Online  ", CENTER);
    }
}

metrics_data cache_data;
DisplayBuffer display_buffer;

void calc_battery()
{
    uint8_t percentage = 100;
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    {
        vref = adc_chars.vref;
    }
    float voltage = analogRead(36) / 4096.0 * 6.566 * (vref / 1000.0);
    if (voltage > 1)
    {
        percentage = 2836.9625 * pow(voltage, 4) - 43987.4889 * pow(voltage, 3) + 255233.8134 * pow(voltage, 2) - 656689.7123 * voltage + 632041.7303;
        if (voltage >= 4.20)
            percentage = 100;
        if (voltage <= 3.20)
            percentage = 0; // orig 3.5
    }
    display_buffer.sysinfo.battery_p.setValue(percentage);
    display_buffer.sysinfo.voltage.setValue(voltage);
}

// Coleta métricas externas e internas e
// e prepara para facilitar a exibição
void collect_metrics()
{
    struct metrics_data incomingData;

    if (xQueueReceive(xQueueMetrics, &incomingData, (TickType_t)0) == pdTRUE)
    {
        Serial.println(incomingData.toString());

        display_buffer.sysinfo.last_data_received_millis = millis();

        display_buffer.baro.altitude.setValue(incomingData.baro_data.altitude);
        display_buffer.baro.temperature.setValue(incomingData.baro_data.temperature);
        display_buffer.baro.pressure.setValue(incomingData.baro_data.pressure);
        display_buffer.baro.vario.setValue(incomingData.baro_data.vario);

        display_buffer.gnss.altitude.setValue(incomingData.gnss_data.altitude_meters);
        display_buffer.gnss.latitude.setValue(incomingData.gnss_data.location_lat);
        display_buffer.gnss.longitude.setValue(incomingData.gnss_data.location_lng);
    }
}

unsigned long elapsed_time_5_seconds = 0;
unsigned long elapsed_time_A = 0;

void display_mutable_elements()
{
    bool redraw = false;
    if (elapsed_time_A < millis())
    {
        epd_clear_area(epd_full_screen());

        setFont(OpenSans10B);
        drawString(400, 90, "Altitude 1", LEFT);
        drawString(400, 190, "Temperatura", LEFT);
        drawString(400, 290, "Pressão", LEFT);
        drawString(400, 390, "Vario m/s", LEFT);
        drawString(600, 90, "Altitude 2", LEFT);
        drawString(600, 190, "Longitude", LEFT);
        drawString(600, 290, "Latitude", LEFT);
        drawString(600, 390, "Satelites", LEFT);

        redraw = true;
        elapsed_time_A = millis() + (60000 * 5);
    }

    DrawSmartInt(400, 150, 150, 60, display_buffer.baro.altitude, redraw);
    DrawSmartInt(400, 250, 150, 60, display_buffer.baro.temperature, redraw);
    DrawSmartInt(400, 350, 150, 60, display_buffer.baro.pressure, redraw);
    DrawSmartFlt(400, 450, 200, 60, display_buffer.baro.vario, redraw);

    DrawSmartFlt(600, 150, 150, 60, display_buffer.gnss.altitude, redraw);
    DrawSmartFlt(600, 250, 150, 60, display_buffer.gnss.longitude, redraw);
    DrawSmartFlt(600, 350, 150, 60, display_buffer.gnss.latitude, redraw);
    DrawSmartInt(600, 450, 150, 60, display_buffer.gnss.satellites, redraw);

    DrawEspecificBattery(display_buffer.sysinfo.battery_p, display_buffer.sysinfo.voltage);

    if (elapsed_time_5_seconds < millis())
    {
        // DrawConnectionStatus(display_buffer.sysinfo.last_data_received_millis);
        elapsed_time_5_seconds = millis() + 5000; // 5 segundos
    }

    Serial.println("Drawing...");
    epd_draw_grayscale_image(epd_full_screen(), framebuffer);
}
