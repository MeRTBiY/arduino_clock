#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "context.h"
#include "screens.h"

enum screen init_screen(struct context *ctx) {
    init();
    Wire.begin();
    Serial.begin(BAUD_RATE);
    Serial.println("> Init Screen");

    ctx->lcd->begin(LCD_COLS, LCD_ROWS);
    ctx->lcd->backlight();
    ctx->lcd->setCursor(0, 0);
    ctx->lcd->print("  Alarm Clock   ");
    ctx->lcd->setCursor(0, 1);
    ctx->lcd->print("  Starting...   ");

    load_alarm();
    update_led();
    delay(1500);
    ctx->lcd->clear();
    return CLOCK_SCR;
}