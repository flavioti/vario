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
#define CORE_0 0
#define CORE_1 1

// -----------------------------------------------------------------------------
// OLED
// -----------------------------------------------------------------------------

#define SCREEN_ADDRESS 0x3C

// -----------------------------------------------------------------------------
// GPS
// -----------------------------------------------------------------------------

#define GPS_SERIAL_NUM 1
#define GPS_BAUDRATE 9600

// -----------------------------------------------------------------------------
// BMP280
// -----------------------------------------------------------------------------

#define VARIO_BMP280_I2C_ADDRESS 0x76
#define VARIO_BMP280_SAMPLES 5

// -----------------------------------------------------------------------------
// VARIO
// -----------------------------------------------------------------------------

#define VARIO_SINK_THRESHOLD_SINK -1.0 // default -2.4 ms/sPS
#define VARIO_SINK_THRESHOLD_LIFT 0.1  // default 0.2 ms/s

// Valores para considerar que o dispositivo está em movimento
#define VARIO_ACCEL_Z_THRESHOLD_SINK -2000
#define VARIO_ACCEL_Z_THRESHOLD_LIFT 2000

// Valores minimos e maximos para o sensor de aceleracao a serem enviados para fila
// O valor deve ser sempre menor que o valor de sink e lift
#define MIN_ACCEL_Z VARIO_ACCEL_Z_THRESHOLD_SINK / 10
#define MAX_ACCEL_Z VARIO_ACCEL_Z_THRESHOLD_LIFT / 10

// -----------------------------------------------------------------------------
// BUZZER
// -----------------------------------------------------------------------------

#define BUZZER_PIN 15
#define VARIO_BUZZER_LOG_ENABLED

// -----------------------------------------------------------------------------
// WEBSERVER
// -----------------------------------------------------------------------------

// #define RESTART_IF_MDNS_FAIL // Reinicia o ESP32 se o MDNS falhar
