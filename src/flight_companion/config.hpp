#include <map>

// -----------------------------------------------------------------------------
// Version
// -----------------------------------------------------------------------------

#define APP_NAME "COPILOTO"
#define APP_VERSION "1.1.0"


// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

// PIN AND ADDRESS DEFINITIONS

// GPIO - General Purpose Input/Output

#if defined(ESP32_WROOM_32)

// GPIO 13
#define GPIO_13             13
#define PORT_PHISICAL_20    GPIO_13
#define PORT_ADC2_4         GPIO_13
#define PORT_HSPI_ID        GPIO_13
#define PORT_TOUCH4         GPIO_13

// GPIO 15
#define GPIO_15             15
#define PORT_PHISICAL_21    GPIO_15
#define PORT_ADC2_3         GPIO_15
#define PORT_TOUCH3         GPIO_15

// GPIO 21
#define GPIO_21             21
#define PORT_PHISICAL_42    GPIO_21
#define PORT_SDA            GPIO_21
#define PORT_V_SPI_HD       GPIO_21

// GPIO 33
#define GPIO_33             33
#define PORT_PHISICAL_13    GPIO_33
#define PORT_ADC1_5         GPIO_33
#define PORT_TOUCH8         GPIO_33
#define PORT_XTAL32         GPIO_33

// CUSTOM PIN DEFINITION
#define VOLTAGE_PIN         GPIO_33
#define BUZZER_PIN          GPIO_15
#define PIN_I2C_SDA         GPIO_21
#define PIN_I2C_SCL         22
#define GPS_RX_PIN          34
#define GPS_TX_PIN          12

// Display mudou o endereço sozinho quando liguei todos o componentes juntos, ficar de olho nisso...
#define SCREEN_ADDRESS      0x3C
// #define SCREEN_ADDRESS      0x3D

#define SCREEN_WIDTH        128 // OLED display width, in pixels
#define SCREEN_HEIGHT       64 // OLED display height, in pixels
#define OLED_RESET          -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#define BMP280_ADDRESS      0x76

#define USE_ESPNOW

// Menor = Menos prioridade
#define BUZZ_TASK_PRIORITY  (20 |portPRIVILEGE_BIT)
#define COPI_TASK_PRIORITY  (15 |portPRIVILEGE_BIT)
#define BARO_TASK_PRIORITY  (10 |portPRIVILEGE_BIT)
#define GNSS_TASK_PRIORITY  (5 |portPRIVILEGE_BIT)
#define ACEL_TASK_PRIORITY  (1 |portPRIVILEGE_BIT)

#endif // if defined(ESP32_WROOM_32)



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
// CORE
// -----------------------------------------------------------------------------

#define CORE_0 0
#define CORE_1 1
#define ACEL_READ_RATE 2000 // 1s
#define BUZZER_RATE 100 // 100ms
#define GNSS_READ_RATE 5000 // 5s
#define COPILOT_READ_RATE 500 // 1s
#define BARO_READ_RATE 1000 // 1s

// -----------------------------------------------------------------------------
// GPS
// -----------------------------------------------------------------------------

#define GPS_SERIAL_NUM 1
#define GPS_BAUDRATE 9600

// -----------------------------------------------------------------------------
// BMP280
// -----------------------------------------------------------------------------

#define VARIO_BMP280_SAMPLES 5

// -----------------------------------------------------------------------------
// VARIO
// -----------------------------------------------------------------------------

#define VARIO_SINK_THRESHOLD_SINK -1.0 // default -1.0 ms/s
#define VARIO_SINK_THRESHOLD_LIFT 0.3  // default 0.3 ms/s

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

#define BUZZER_CHANNEL 0
#define VARIO_BUZZER_LOG_ENABLED

// -----------------------------------------------------------------------------
// WEBSERVER
// -----------------------------------------------------------------------------

// #define RESTART_IF_MDNS_FAIL // Reinicia o ESP32 se o MDNS falhar
