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

void setup() {
    Serial.begin(9600);
    Wire.begin();
    
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.print("Найден I2C адрес: 0x");
            Serial.println(addr, HEX);
        }
    }
}

void loop() {}