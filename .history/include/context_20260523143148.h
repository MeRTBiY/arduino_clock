#ifndef CONTEXT_H
#define CONTEXT_H

#include <Arduino.h>
#include <I2C_LCD.h>

struct context {
    I2C_LCD *lcd;
    byte alarm_hours;
    byte alarm_minutes;
    bool alarm_enabled;
    bool snooze_active;
    unsigned long snooze_time;
    unsigned long btn4_press_time;
    bool btn4_was_pressed;
};

#endif // CONTEXT_H