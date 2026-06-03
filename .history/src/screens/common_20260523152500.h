#ifndef SCREENS_COMMON_H
#define SCREENS_COMMON_H

#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "context.h"

#define EEPROM_ALARM_HOURS   0
#define EEPROM_ALARM_MINUTES 1
#define EEPROM_ALARM_ENABLED 2

inline void load_alarm(struct context *ctx) {
    ctx->alarm_hours   = EEPROM.read(EEPROM_ALARM_HOURS);
    ctx->alarm_minutes = EEPROM.read(EEPROM_ALARM_MINUTES);
    ctx->alarm_enabled = EEPROM.read(EEPROM_ALARM_ENABLED);
    if (ctx->alarm_hours > 23)   ctx->alarm_hours = 0;
    if (ctx->alarm_minutes > 59) ctx->alarm_minutes = 0;
}

inline void save_alarm(struct context *ctx) {
    EEPROM.write(EEPROM_ALARM_HOURS,   ctx->alarm_hours);
    EEPROM.write(EEPROM_ALARM_MINUTES, ctx->alarm_minutes);
    EEPROM.write(EEPROM_ALARM_ENABLED, ctx->alarm_enabled);
}

inline void update_led(struct context *ctx) {
    digitalWrite(RGB_GREEN_PIN, ctx->alarm_enabled ? HIGH : LOW);
    digitalWrite(RGB_RED_PIN,   ctx->alarm_enabled ? LOW  : HIGH);
}

inline bool btn_pressed(int pin) {
    if (digitalRead(pin) == HIGH) {
        delay(50);
        return digitalRead(pin) == HIGH;
    }
    return false;
}

inline void wait_release(int pin) {
    while (digitalRead(pin) == HIGH) delay(10);
    delay(50);
}

#endif // SCREENS_COMMON_H