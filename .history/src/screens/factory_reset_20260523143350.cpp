#include <Arduino.h>
#include "config.h"
#include "context.h"
#include "screens.h"
#include "screens/common.h"
#include "rtc_wrapper.h"

enum screen factory_reset_screen(struct context *ctx) {
    digitalWrite(BUZZER_PIN, LOW);
    ctx->lcd->setCursor(0, 0);
    ctx->lcd->print("Factory Reset...");
    ctx->lcd->setCursor(0, 1);
    ctx->lcd->print("Please wait...  ");

    set_datetime(1, 1, 2000, 0, 0, 0);
    ctx->alarm_hours   = 0;
    ctx->alarm_minutes = 0;
    ctx->alarm_enabled = false;
    ctx->snooze_active = false;
    save_alarm(ctx);
    update_led(ctx);

    delay(2000);
    ctx->lcd->clear();
    return CLOCK_SCR;
}