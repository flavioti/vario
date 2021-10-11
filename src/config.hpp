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
#define I2C_SDA 21
#define I2C_SCL 22
#endif

// -----------------------------------------------------------------------------
// CORE
// -----------------------------------------------------------------------------

#define AXP192
// #define AXP20X
// #define SYSCACHE_LOG_ENABLED
// #define DISABLE_WATCH_DOG
#define CORE_0 0
#define CORE_1 1

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
// #define GPS_LOG_ENABLED
#endif

// -----------------------------------------------------------------------------
// BMP280
// -----------------------------------------------------------------------------

#define USE_BMP280

#if defined(USE_BMP280)
// #define VARIO_BMP280_LOG_ENABLED
#define VARIO_BMP280_I2C_ADDRESS 0x76
#define VARIO_BMP280_SAMPLES 5
#endif

// -----------------------------------------------------------------------------
// VARIO
// -----------------------------------------------------------------------------

#define VARIO_SINK_THRESHOLD_SINK -2.4 // default -2.4 ms/s
#define VARIO_SINK_THRESHOLD_LIFT 0.1  // default 0.2 ms/s

// Valores para considerar que o dispositivo está em movimento
#define VARIO_ACCEL_Z_THRESHOLD_SINK -70
#define VARIO_ACCEL_Z_THRESHOLD_LIFT 70

// Valores minimos e maximos para o sensor de aceleracao a serem enviados para fila
// O valor deve ser sempre menor que o valor de sink e lift
#define MIN_ACCEL_Z -20
#define MAX_ACCEL_Z 20

// -----------------------------------------------------------------------------
// BUZZER
// -----------------------------------------------------------------------------

#define USE_BUZZER

#if defined(USE_BUZZER)
#define BUZZER_PIN 15
#define VARIO_BUZZER_LOG_ENABLED
#endif

// -----------------------------------------------------------------------------
// WEBSERVER
// -----------------------------------------------------------------------------

#if defined(USE_WIFI)
#define USE_WEBSERVER

#if defined(USE_WEBSERVER)
#define RESTART_IF_MDNS_FAIL // Reinicia o ESP32 se o MDNS falhar
#endif
#endif

// -----------------------------------------------------------------------------
// WIFI
// -----------------------------------------------------------------------------

#define USE_WIFI

// -----------------------------------------------------------------------------
// TELEMETRY
// -----------------------------------------------------------------------------

#define CAPTURE_CORE_STATUS

// -----------------------------------------------------------------------------
// MPU 6050
// -----------------------------------------------------------------------------

#define USE_MPU6050
