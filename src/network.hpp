#include <WiFi.h>
#include <config.hpp>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <utils.hpp>
#include <secrets.hpp>
#include <nvs_flash.h>

void connect_wifi2()
{
    Serial.println("Disconnecting WiFi");
    WiFi.disconnect();

    Serial.printf("WiFi mode: %i\n", WiFi.getMode());

    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi ");
    Serial.println(WIFI_SSID);
    Serial.print(" network with IP Address ");
    Serial.println(WiFi.localIP());
    Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

// -----------------------------------------------------------------------------
// OTA
// -----------------------------------------------------------------------------

int contador_ms = 0;
WebServer ota_server(80);

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

    // setMetric(&p, "esp32_boot_counter", String(getBootCounter()));
    setMetric(&p, "esp32_loop_counter", String(sys_cache.loop_counter));
    setMetric(&p, "esp32_battery_voltage", String(sys_cache.battery_voltage));
    setMetric(&p, "esp32_battery_percentage", String(sys_cache.battery_percentage));
    setMetric(&p, "esp32_power_down_voltage", String(sys_cache.power_down_voltage));
    setMetric(&p, "esp32_loop_millis", String(sys_cache.loop_millis <= 100 ? sys_cache.loop_millis : 0));
    setMetric(&p, "esp32_core_status_millis", String(sys_cache.core_status_millis));
    setMetric(&p, "esp32_gps_millis", String(sys_cache.gps_millis));
    setMetric(&p, "esp32_baro_millis", String(sys_cache.baro_millis));
    setMetric(&p, "esp32_screen_millis", String(sys_cache.screen_millis));
    setMetric(&p, "esp32_handle_client_millis", String(sys_cache.handle_client_millis <= 100 ? sys_cache.handle_client_millis : 0));

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

    // Example of nvs_get_stats() to get the number of used entries and free entries:
    // https: //docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html#_CPPv413nvs_get_statsPKcP11nvs_stats_t
    // nvs_stats_t nvs_stats;
    // nvs_get_stats(NULL, &nvs_stats);
    // setMetric(&p, "esp32_nvs_stats_used_entries", nvs_stats.used_entries);
    // setMetric(&p, "esp32_nvs_stats_free_entries", String(nvs_stats.free_entries, 0));
    // setMetric(&p, "esp32_nvs_stats_total_entries", String(nvs_stats.total_entries, 0));

    return p;
}

void handleMetrics()
{
    digitalWrite(LED_BUILTIN, 1);
    ota_server.send(200, "text/plain", getMetrics());
    digitalWrite(LED_BUILTIN, 0);
}

void config_ota(void)
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
        ota_server.on("/", HTTP_GET, []()
                      {
                          ota_server.sendHeader("Connection", "close");
                          ota_server.send(200, "text/html", loginIndex);
                      });

        ota_server.on("/serverIndex", HTTP_GET, []()
                      {
                          ota_server.sendHeader("Connection", "close");
                          ota_server.send(200, "text/html", serverIndex);
                      });

        /* Define tratamentos do update de firmware OTA */
        ota_server.on(
            "/update", HTTP_POST, []()
            {
                ota_server.sendHeader("Connection", "close");
                ota_server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
                ESP.restart();
            },
            []()
            {
                HTTPUpload &upload = ota_server.upload();

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

        ota_server.on("/metrics", HTTP_GET, handleMetrics);

        ota_server.begin();
    }
    else
    {
        Serial.println("NETWORK - OTA server not initialized");
    }
}

void handle_client()
{
    ota_server.handleClient();
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    Serial.println(event->event_id);
    // switch (event->event_id)
    // {
    // case SYSTEM_EVENT_STA_START:
    //     //WiFi connected

    //     break;

    // case SYSTEM_EVENT_STA_GOT_IP:
    //     //WiFi got IP

    //     break;

    // case SYSTEM_EVENT_STA_DISCONNECTED:
    //     //WiFi lost connection

    //     break;

    // default:
    //     break;
    // }
    return ESP_OK;
}
/* AVAILABLE EVENTS:
    SYSTEM_EVENT_WIFI_READY = 0,           //< ESP32 WiFi ready
    SYSTEM_EVENT_SCAN_DONE,                //< ESP32 finish scanning AP
    SYSTEM_EVENT_STA_START,                //< ESP32 station start
    SYSTEM_EVENT_STA_STOP,                 //< ESP32 station stop
    SYSTEM_EVENT_STA_CONNECTED,            //< ESP32 station connected to AP
    SYSTEM_EVENT_STA_DISCONNECTED,         //< ESP32 station disconnected from AP
    SYSTEM_EVENT_STA_AUTHMODE_CHANGE,      //< the auth mode of AP connected by ESP32 station changed
    SYSTEM_EVENT_STA_GOT_IP,               //< ESP32 station got IP from connected AP
    SYSTEM_EVENT_STA_LOST_IP,              //< ESP32 station lost IP and the IP is reset to 0
    SYSTEM_EVENT_STA_WPS_ER_SUCCESS,       //< ESP32 station wps succeeds in enrollee mode
    SYSTEM_EVENT_STA_WPS_ER_FAILED,        //< ESP32 station wps fails in enrollee mode
    SYSTEM_EVENT_STA_WPS_ER_TIMEOUT,       //< ESP32 station wps timeout in enrollee mode
    SYSTEM_EVENT_STA_WPS_ER_PIN,           //< ESP32 station wps pin code in enrollee mode
    SYSTEM_EVENT_AP_START,                 //< ESP32 soft-AP start
    SYSTEM_EVENT_AP_STOP,                  //< ESP32 soft-AP stop
    SYSTEM_EVENT_AP_STACONNECTED,          //< a station connected to ESP32 soft-AP
    SYSTEM_EVENT_AP_STADISCONNECTED,       //< a station disconnected from ESP32 soft-AP
    SYSTEM_EVENT_AP_STAIPASSIGNED,         //< ESP32 soft-AP assign an IP to a connected station
    SYSTEM_EVENT_AP_PROBEREQRECVED,        //< Receive probe request packet in soft-AP interface
    SYSTEM_EVENT_GOT_IP6,                  //< ESP32 station or ap or ethernet interface v6IP addr is preferred
    SYSTEM_EVENT_ETH_START,                //< ESP32 ethernet start
    SYSTEM_EVENT_ETH_STOP,                 //< ESP32 ethernet stop
    SYSTEM_EVENT_ETH_CONNECTED,            //< ESP32 ethernet phy link up
    SYSTEM_EVENT_ETH_DISCONNECTED,         //< ESP32 ethernet phy link down
    SYSTEM_EVENT_ETH_GOT_IP,               //< ESP32 ethernet got IP from connected AP
*/

void setup_network()
{
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
}
