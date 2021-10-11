#include <WiFi.h>
#include <flight_companion/config.hpp>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <flight_companion/secrets.hpp>
#include <nvs_flash.h>
#include "esp_log.h"

void connect_wifi2()
{
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    WiFi.disconnect();

    ESP_LOGD("NETWORK", "Connecting to WiFi");
    // Serial.println("Connecting to WiFi");

    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected to WiFi %s network with IP Address %s\n", WIFI_SSID, WiFi.localIP());
}

// -----------------------------------------------------------------------------
// WEB SERVER
// -----------------------------------------------------------------------------

int contador_ms = 0;
WebServer web_server(80);

const char *loginIndex =
    "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
    "<tr>"
    "<td colspan=2>"
    "<center><font size=4><b>ESP32 - identifique-se</b></font></center>"
    "<br>"
    "</td>"
    "<br>"
    "<br>"
    "</tr>"
    "<td>Login:</td>"
    "<td><input type='text' size=25 name='userid'><br></td>"
    "</tr>"
    "<br>"
    "<br>"
    "<tr>"
    "<td>Senha:</td>"
    "<td><input type='Password' size=25 name='pwd'><br></td>"
    "<br>"
    "<br>"
    "</tr>"
    "<tr>"
    "<td><input type='submit' onclick='check(this.form)' value='Identificar'></td>"
    "</tr>"
    "</table>"
    "</form>"
    "<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Login ou senha inválidos')"
    "}"
    "}"
    "</script>";

const char *serverIndex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
    "</form>"
    "<div id='prg'>Progresso: 0%</div>"
    "<script>"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    " $.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('Progresso: ' + Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('Sucesso!')"
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>";

String getMetrics()
{
    String p = ""; // acumulador de dados

    int sketch_size = ESP.getSketchSize();
    int flash_size = ESP.getFreeSketchSpace();
    int available_size = flash_size - sketch_size;

    setMetric(&p, "esp32_uptime", String(millis()));
    setMetric(&p, "esp32_wifi_rssi", String(WiFi.RSSI()));
    setMetric(&p, "esp32_heap_size", String(ESP.getHeapSize()));
    setMetric(&p, "esp32_free_heap", String(xPortGetFreeHeapSize()));
    setMetric(&p, "esp32_min_ever_free_heap", String(xPortGetMinimumEverFreeHeapSize()));
    setMetric(&p, "esp32_sketch_size", String(sketch_size));
    setMetric(&p, "esp32_flash_size", String(flash_size));
    setMetric(&p, "esp32_available_size", String(available_size));
    setMetric(&p, "esp32_internal_temperature", String(sys_cache.esp32_temperature));

    setMetric(&p, "esp32_battery_voltage", String(sys_cache.battery_voltage));
    setMetric(&p, "esp32_battery_percentage", String(sys_cache.battery_percentage));
    setMetric(&p, "esp32_power_down_voltage", String(sys_cache.power_down_voltage));

    setMetric(&p, "esp32_baro_temperature", String(baro_cache.temperature, 1));
    setMetric(&p, "esp32_baro_pressure", String(baro_cache.pressure, 1));
    setMetric(&p, "esp32_baro_altitude", String(baro_cache.altitude, 1));
    setMetric(&p, "esp32_baro_vario", String(baro_cache.vario, 6));

    setMetric(&p, "esp32_geo_altitude", String(geo_cache.altitude, 1));
    setMetric(&p, "esp32_geo_latitude", String(geo_cache.latitude, 6));
    setMetric(&p, "esp32_geo_longitude", String(geo_cache.longitude, 6));
    setMetric(&p, "esp32_geo_satellites", String(geo_cache.satellites));

    setMetric(&p, "esp32_mpu_temp", String(mpu_cache.temp));
    setMetric(&p, "esp32_mpu_gx", String(mpu_cache.gx));
    setMetric(&p, "esp32_mpu_gy", String(mpu_cache.gy));
    setMetric(&p, "esp32_mpu_gz", String(mpu_cache.gz));
    setMetric(&p, "esp32_mpu_ax", String(mpu_cache.ax));
    setMetric(&p, "esp32_mpu_ay", String(mpu_cache.ay));
    setMetric(&p, "esp32_mpu_az", String(mpu_cache.az));
    setMetric(&p, "esp32_mpu_delta_z", String(mpu_cache.delta_z));

    return p;
}

void handleMetrics()
{
    digitalWrite(LED_BUILTIN, 1);
    web_server.send(200, "text/plain", getMetrics());
    digitalWrite(LED_BUILTIN, 0);
}

void config_web_server(void)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        /* Usa MDNS para resolver o DNS */
        bool MDNS_ok = MDNS.begin("esp32");
        Serial.println(MDNS_ok ? "MDNS respondendo" : "MDNS falhou");
#ifdef RESTART_IF_MDNS_FAIL
        if (!MDNS_ok)
        {
            //http://esp32.local
            Serial.println("Erro ao configurar mDNS. O ESP32 vai reiniciar em 1s...");
            delay(1000);
            ESP.restart();
        }
#endif

        Serial.println("mDNS configurado e inicializado");

        /* Configura as páginas de login e upload de firmware OTA */
        web_server.on("/", HTTP_GET, []()
                      {
                          web_server.sendHeader("Connection", "close");
                          web_server.send(200, "text/html", loginIndex);
                      });

        web_server.on("/serverIndex", HTTP_GET, []()
                      {
                          web_server.sendHeader("Connection", "close");
                          web_server.send(200, "text/html", serverIndex);
                      });

        /* Define tratamentos do update de firmware OTA */
        web_server.on(
            "/update", HTTP_POST, []()
            {
                web_server.sendHeader("Connection", "close");
                web_server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
                ESP.restart();
            },
            []()
            {
                HTTPUpload &upload = web_server.upload();

                if (upload.status == UPLOAD_FILE_START)
                {
                    /* Inicio do upload de firmware OTA */
                    Serial.printf("Updating with %s\n", upload.filename.c_str());
                    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
                        Update.printError(Serial);
                }
                else if (upload.status == UPLOAD_FILE_WRITE)
                {
                    /* Escrevendo firmware enviado na flash do ESP32 */
                    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                        Update.printError(Serial);
                }
                else if (upload.status == UPLOAD_FILE_END)
                {
                    /* Final de upload */
                    if (Update.end(true))
                        Serial.printf("Sucesso no update de firmware: %u\nReiniciando ESP32...\n", upload.totalSize);
                    else
                        Update.printError(Serial);
                }
            });

        web_server.on("/metrics", HTTP_GET, handleMetrics);

        web_server.begin();
    }
    else
    {
        Serial.println("NETWORK - OTA server not initialized");
    }
}

void handle_client()
{
    web_server.handleClient();
}
