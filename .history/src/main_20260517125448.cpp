/*#include <Arduino.h>
#include "rtc_wrapper.h"
#include "sensors.h"
#include "lcd_wrapper.h"
#include "states.h"
#include "config.h"

void setup() {
    Serial.begin(9600);

    // кнопки — INPUT_PULLUP значит нажатие = LOW
    pinMode(BTN1_PIN, INPUT_PULLUP);
    pinMode(BTN2_PIN, INPUT_PULLUP);
    pinMode(BTN3_PIN, INPUT_PULLUP);
    pinMode(BTN4_PIN, INPUT_PULLUP);

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    lcd_init();
    clock_init();
    sensors_init();
}

int main() {
    setup();

    enum state current_state = CLOCK;

    for (;;) {
        switch (current_state) {
            case CLOCK:
                current_state = state_clock();
                break;
            case SHOW_DATE:
                current_state = state_show_date();
                break;
            case SHOW_ENV:
                current_state = state_show_env();
                break;
            case ALARM:
                current_state = state_alarm();
                break;
            case FACTORY_RESET:
                current_state = state_factory_reset();
                break;
        }
    }
}*/
#include <Arduino.h>
#include <Wire.h>
#include <I2C_LCD.h>
#include <RtcDS1302.h>

static ThreeWire wire(4, 5, 9); // DAT, CLK, RST
static RtcDS1302<ThreeWire> rtc(wire);
static I2C_LCD lcd(0x27);

void setup() {
    Wire.begin();
    lcd.begin(16, 2);
    lcd.backlight();
    
    rtc.Begin();
    rtc.SetIsWriteProtected(false);
    rtc.SetIsRunning(true);
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    rtc.SetDateTime(compiled);
    
    RtcDateTime now = rtc.GetDateTime();
    char buf[17];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
             now.Hour(), now.Minute(), now.Second());
    lcd.setCursor(0, 0);
    lcd.print(buf);
}

void loop() {}