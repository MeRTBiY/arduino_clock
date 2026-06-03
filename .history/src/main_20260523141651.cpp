#include <Arduino.h>
#include <Wire.h>
#include <I2C_LCD.h>

#include "config.h"
#include "context.h"
#include "screens.h"
#include "rtc_wrapper.h"
#include "sensors.h"

int main() {
    // создаём LCD и контекст
    I2C_LCD lcd(LCD_I2C_ADDRESS);
    struct context ctx = { &lcd };

    // настройка пинов
    pinMode(BTN1_PIN, INPUT);
    pinMode(BTN2_PIN, INPUT);
    pinMode(BTN3_PIN, INPUT);
    pinMode(BTN4_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    pinMode(RGB_GREEN_PIN, OUTPUT);
    pinMode(RGB_RED_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RGB_GREEN_PIN, LOW);
    digitalWrite(RGB_RED_PIN, LOW);

    // инициализация модулей
    clock_init();
    sensors_init();

    // главный цикл
    enum screen current = INIT_SCR;
    for (;;) {
        switch (current) {
            case INIT_SCR:
                current = init_screen(&ctx);
                break;
            case CLOCK_SCR:
                current = clock_screen(&ctx);
                break;
            case SHOW_DATE_SCR:
                current = show_date_screen(&ctx);
                break;
            case SHOW_ENV_SCR:
                current = show_env_screen(&ctx);
                break;
            case ALARM_SCR:
                current = alarm_screen(&ctx);
                break;
            case FACTORY_RESET_SCR:
                current = factory_reset_screen(&ctx);
                break;
        }
    }
}