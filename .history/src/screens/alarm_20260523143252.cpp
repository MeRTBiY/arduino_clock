#include <Arduino.h>
#include "config.h"
#include "context.h"
#include "screens.h"
#include "screens/common.h"

enum screen alarm_screen(struct context *ctx) {
    digitalWrite(BUZZER_PIN, HIGH);

    ctx->lcd->setCursor(0, 0);
    ctx->lcd->print("  *** ALARM ***  ");
    char buf[17];
    snprintf(buf, sizeof(buf), "    %02d:%02d       ",
             ctx->alarm_hours, ctx->alarm_minutes);
    ctx->lcd->setCursor(0, 1);
    ctx->lcd->print(buf);

    if (btn_pressed(BTN3_PIN)) {
        wait_release(BTN3_PIN);
        digitalWrite(BUZZER_PIN, LOW);
        ctx->lcd->clear();
        return CLOCK_SCR;
    }

    if (btn_pressed(BTN2_PIN)) {
        wait_release(BTN2_PIN);
        digitalWrite(BUZZER_PIN, LOW);
        ctx->snooze_active = true;
        ctx->snooze_time   = millis();
        ctx->lcd->clear();
        return CLOCK_SCR;
    }

    delay(200);
    return ALARM_SCR;
}