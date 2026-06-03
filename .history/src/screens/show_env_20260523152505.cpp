#include <Arduino.h>
#include "config.h"
#include "context.h"
#include "screens.h"
#include "screens/common.h"
#include "sensors.h"

enum screen show_env_screen(struct context *ctx) {
    static unsigned long last_read = 0;
    static float temp = 0;
    static int   humi = 0;

    if (millis() - last_read >= (unsigned long)SENSORS_READ_INTERVAL * 1000UL) {
        last_read = millis();
        temp = get_temperature();
        humi = get_humidity();
    }

    char buf[17];
    snprintf(buf, sizeof(buf), "Temp: %.1f C    ", temp);
    ctx->lcd->setCursor(0, 0);
    ctx->lcd->print(buf);
    snprintf(buf, sizeof(buf), "Humi: %d %%      ", humi);
    ctx->lcd->setCursor(0, 1);
    ctx->lcd->print(buf);

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
        return CLOCK_SCR;
    }

    if (btn_pressed(BTN3_PIN)) {
        wait_release(BTN3_PIN);
        ctx->lcd->clear();
        byte h = ctx->alarm_hours;
        byte m = ctx->alarm_minutes;

        ctx->lcd->setCursor(0, 0);
        ctx->lcd->print("Set alarm H:    ");
        while (true) {
            snprintf(buf, sizeof(buf), "H:%02d  M:%02d      ", h, m);
            ctx->lcd->setCursor(0, 1); ctx->lcd->print(buf);
            if (btn_pressed(BTN2_PIN)) { wait_release(BTN2_PIN); h = (h + 1) % 24; }
            if (btn_pressed(BTN1_PIN)) { wait_release(BTN1_PIN); h = (h + 23) % 24; }
            if (btn_pressed(BTN3_PIN)) { wait_release(BTN3_PIN); break; }
            delay(100);
        }

        ctx->lcd->setCursor(0, 0);
        ctx->lcd->print("Set alarm M:    ");
        while (true) {
            snprintf(buf, sizeof(buf), "H:%02d  M:%02d      ", h, m);
            ctx->lcd->setCursor(0, 1); ctx->lcd->print(buf);
            if (btn_pressed(BTN2_PIN)) { wait_release(BTN2_PIN); m = (m + 1) % 60; }
            if (btn_pressed(BTN1_PIN)) { wait_release(BTN1_PIN); m = (m + 59) % 60; }
            if (btn_pressed(BTN3_PIN)) { wait_release(BTN3_PIN); break; }
            delay(100);
        }

        ctx->alarm_hours   = h;
        ctx->alarm_minutes = m;
        ctx->alarm_enabled = true;
        save_alarm(ctx);
        update_led(ctx);
        ctx->lcd->clear();
    }

    delay(200);
    return SHOW_ENV_SCR;
}