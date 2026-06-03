#define DHT_PIN 8

// RTC Module
#define RTC_CLK_PIN 5
#define RTC_DAT_PIN 4
#define RTC_RST_PIN 9

#define BUZZER_PIN 6

#define LCD_SDA_PIN A4
#define LCD_SCL_PIN A5

// LCD Display
#define LCD_I2C_ADDRESS 0x27
#define LCD_ROWS 2
#define LCD_COLS 16

#define BTN1_PIN 7
#define BTN2_PIN 10
#define BTN3_PIN 11
#define BTN4_PIN 12

#define BTN1_PRESSED 0b00000001
#define BTN2_PRESSED 0b00000010
#define BTN3_PRESSED 0b00000100
#define BTN4_PRESSED 0b00001000

// interval for reading the values from sensors
#define SENSORS_READ_INTERVAL 1 * 60

// duration for entering the factory reset state
#define FACTORY_RESET_INTERVAL 3 * 1000

// for testing purposes
// #define __DATE__ "1.1.2000"
// #define __TIME__ "00:00:00"
