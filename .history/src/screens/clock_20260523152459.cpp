#include <Arduino.h>
#include "config.h"
#include "context.h"
#include "screens.h"
#include "screens/common.h"
#include "rtc_wrapper.h"

enum screen clock_screen(struct context *ctx) {
    struct dt t = now();
    char buf[17];
    snprintf(buf, sizeof(buf), "    %02d:%02d:%02d    ",
             t.hours, t.minutes, t.seconds);
    ctx->lcd->setCursor(0, 0);
    ctx->lcd->print(buf);

    if (ctx->alarm_enabled) {
        snprintf(buf, sizeof(buf), "ALM %02d:%02d      ",
                 ctx->alarm_hours, ctx->alarm_minutes);
    } else {
        snprintf(buf, sizeof(buf), "ALM OFF         ");
    }
    ctx->lcd->setCursor(0, 1);
    ctx->lcd->print(buf);

    if (ctx->alarm_enabled && !ctx->snooze_active &&
        t.hours == ctx->alarm_hours &&
        t.minutes == ctx->alarm_minutes && t.seconds == 0) {
        return ALARM_SCR;
    }

    if (ctx->snooze_active &&
        millis() - ctx->snooze_time >= 5UL * 60UL * 1000UL) {
        ctx->snooze_active = false;
        return ALARM_SCR;
    }

    if (digitalRead(BTN4_PIN) == HIGH) {
        if (!ctx->btn4_was_pressed) {
            ctx->btn4_was_pressed = true;
            ctx->btn4_press_time = millis();
        }
        if (millis() - ctx->btn4_press_time >= FACTORY_RESET_INTERVAL) {
            wait_release(BTN4_PIN);
            ctx->btn4_was_pressed = false;
            return FACTORY_RESET_SCR;
        }
    } else if (ctx->btn4_was_pressed) {
        ctx->btn4_was_pressed = false;
        ctx->lcd->clear();
        return SHOW_DATE_SCR;
    }

    if (btn_pressed(BTN3_PIN)) {
        wait_release(BTN3_PIN);
        ctx->lcd->clear();
        struct dt t2 = now();
        byte h = t2.hours;
        byte m = t2.minutes;

        ctx->lcd->setCursor(0, 0);
        ctx->lcd->print("Set time H:     ");
        while (true) {
            snprintf(buf, sizeof(buf), "H:%02d  M:%02d      ", h, m);
            ctx->lcd->setCursor(0, 1);
            ctx->lcd->print(buf);
            if (btn_pressed(BTN2_PIN)) { wait_release(BTN2_PIN); h = (h + 1) % 24; }
            if (btn_pressed(BTN1_PIN)) { wait_release(BTN1_PIN); h = (h + 23) % 24; }
            if (btn_pressed(BTN3_PIN)) { wait_release(BTN3_PIN); break; }
            delay(100);
        }

        ctx->lcd->setCursor(0, 0);
        ctx->lcd->print("Set time M:     ");
        while (true) {
            snprintf(buf, sizeof(buf), "H:%02d  M:%02d      ", h, m);
            ctx->lcd->setCursor(0, 1);
            ctx->lcd->print(buf);
            if (btn_pressed(BTN2_PIN)) { wait_release(BTN2_PIN); m = (m + 1) % 60; }
            if (btn_pressed(BTN1_PIN)) { wait_release(BTN1_PIN); m = (m + 59) % 60; }
            if (btn_pressed(BTN3_PIN)) { wait_release(BTN3_PIN); break; }
            delay(100);
        }

        set_time(h, m, 0);
        ctx->lcd->clear();
    }

    delay(200);
    return CLOCK_SCR;
}