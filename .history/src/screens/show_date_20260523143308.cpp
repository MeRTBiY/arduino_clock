#include <Arduino.h>
#include "config.h"
#include "context.h"
#include "screens.h"
#include "screens/common.h"
#include "rtc_wrapper.h"

enum screen show_date_screen(struct context *ctx) {
    struct dt t = now();
    char buf[17];
    snprintf(buf, sizeof(buf), "  %02d.%02d.%04d   ",
             t.day, t.month, t.year);
    ctx->lcd->setCursor(0, 0);
    ctx->lcd->print(buf);
    ctx->lcd->setCursor(0, 1);
    ctx->lcd->print("                ");

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
        return SHOW_ENV_SCR;
    }

    if (btn_pressed(BTN3_PIN)) {
        wait_release(BTN3_PIN);
        struct dt t2 = now();
        byte d = t2.day; byte mo = t2.month; int y = t2.year;

        ctx->lcd->setCursor(0, 0);
        ctx->lcd->print("Set day:        ");
        while (true) {
            snprintf(buf, sizeof(buf), "D:%02d M:%02d Y:%04d", d, mo, y);
            ctx->lcd->setCursor(0, 1); ctx->lcd->print(buf);
            if (btn_pressed(BTN2_PIN)) { wait_release(BTN2_PIN); d = (d % 31) + 1; }
            if (btn_pressed(BTN1_PIN)) { wait_release(BTN1_PIN); d = (d <= 1) ? 31 : d - 1; }
            if (btn_pressed(BTN3_PIN)) { wait_release(BTN3_PIN); break; }
            delay(100);
        }

        ctx->lcd->setCursor(0, 0);
        ctx->lcd->print("Set month:      ");
        while (true) {
            snprintf(buf, sizeof(buf), "D:%02d M:%02d Y:%04d", d, mo, y);
            ctx->lcd->setCursor(0, 1); ctx->lcd->print(buf);
            if (btn_pressed(BTN2_PIN)) { wait_release(BTN2_PIN); mo = (mo % 12) + 1; }
            if (btn_pressed(BTN1_PIN)) { wait_release(BTN1_PIN); mo = (mo <= 1) ? 12 : mo - 1; }
            if (btn_pressed(BTN3_PIN)) { wait_release(BTN3_PIN); break; }
            delay(100);
        }

        ctx->lcd->setCursor(0, 0);
        ctx->lcd->print("Set year:       ");
        while (true) {
            snprintf(buf, sizeof(buf), "D:%02d M:%02d Y:%04d", d, mo, y);
            ctx->lcd->setCursor(0, 1); ctx->lcd->print(buf);
            if (btn_pressed(BTN2_PIN)) { wait_release(BTN2_PIN); y++; }
            if (btn_pressed(BTN1_PIN)) { wait_release(BTN1_PIN); y--; }
            if (btn_pressed(BTN3_PIN)) { wait_release(BTN3_PIN); break; }
            delay(100);
        }

        set_date(d, mo, y);
        ctx->lcd->clear();
    }

    delay(200);
    return SHOW_DATE_SCR;
}