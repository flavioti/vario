// -----------------------------------------------------------------------------
// Version
// -----------------------------------------------------------------------------

#define APP_NAME "IOT_VARIO"
#define APP_VERSION "0.1.0"

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

// TLORA_V1 definido no platformio.ini
#if defined(TLORA_V1)
#define GPS_RX_PIN 34
#define GPS_TX_PIN 12
// 1st I2C bus on the T-Beam v05+
// https://github.com/lyusupov/SoftRF/issues/32
#define SOC_GPIO_PIN_TBEAM_SDA 13
#define SOC_GPIO_PIN_TBEAM_SCL 2
// Hardware pin definitions for TTGO V2 Board with OLED SSD1306 0,96" I2C Display
#define TTGO_V2_OLED_PIN_RST U8X8_PIN_NONE // connected to CPU RST/EN
#define TTGO_V2_OLED_PIN_SDA 21
#define TTGO_V2_OLED_PIN_SCL 22
#endif

// -----------------------------------------------------------------------------
// OLED
// -----------------------------------------------------------------------------

#define USE_SCREEN

#if defined(USE_SCREEN)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#endif

// -----------------------------------------------------------------------------
// GPS
// -----------------------------------------------------------------------------

#define USE_GPS

#if defined(USE_GPS)
#define GPS_SERIAL_NUM 1
#define GPS_BAUDRATE 9600
#define GPS_LOG_ENABLED
#define I2C_SDA 21
#define I2C_SCL 22
#define GPS_RX_PIN 34
#define GPS_TX_PIN 12
#endif

// -----------------------------------------------------------------------------
// BMP280
// -----------------------------------------------------------------------------

#define USE_BMP280

#if defined(USE_BMP280)
#define VARIO_BMP280_LOG_ENABLED
#define VARIO_BMP280_I2C_ADDRESS 0x76
// #define VARIO_BMP280_I2C_ADDRESS 0x77
#define VARIO_BMP280_SAMPLES 10
#endif

// -----------------------------------------------------------------------------
// BUZZER
// -----------------------------------------------------------------------------

// #define USE_BUZZER 1

// -----------------------------------------------------------------------------
// ENERGIA
// -----------------------------------------------------------------------------

#define USE_ENERGIA

// -----------------------------------------------------------------------------
// OTA
// -----------------------------------------------------------------------------

#define USE_OTA

#if defined(USE_OTA)
// #define RESTART_IF_MDNS_FAIL // Reinicia o ESP32 se o MDNS falhar
#endif

// -----------------------------------------------------------------------------
// WIFI
// -----------------------------------------------------------------------------

#define USE_WIFI

#if defined(USE_WIFI) or defined(USE_OTA)
#define WIFI_SSID std::getenv("WIFI_SSID")
#define WIFI_PASSWORD std::getenv("WIFI_PASS")
#endif

// -----------------------------------------------------------------------------
// POST METRICS
// -----------------------------------------------------------------------------

#define USE_POST_METRICS

#if defined(USE_POST_METRICS) and defined(USE_WIFI)
#define METRICS_URL "http://192.168.99.14:8080/"
#endif
