#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "context.h"
#include "screens.h"
#include "screens/common.h"

enum screen init_screen(struct context *ctx) {
    init();
    Wire.begin();
    Serial.begin(BAUD_RATE);

    ctx->lcd->begin(LCD_COLS, LCD_ROWS);
    ctx->lcd->backlight();
    ctx->lcd->setCursor(0, 0);
    ctx->lcd->print("  Alarm Clock   ");
    ctx->lcd->setCursor(0, 1);
    ctx->lcd->print("  Starting...   ");

    ctx->alarm_hours     = 0;
    ctx->alarm_minutes   = 0;
    ctx->alarm_enabled   = false;
    ctx->snooze_active   = false;
    ctx->snooze_time     = 0;
    ctx->btn4_press_time = 0;
    ctx->btn4_was_pressed = false;

    load_alarm(ctx);
    update_led(ctx);
    delay(1500);
    ctx->lcd->clear();
    return CLOCK_SCR;
}