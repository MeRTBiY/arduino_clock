#include <Arduino.h>
#include <EEPROM.h>
#include "screens.h"
#include "context.h"
#include "config.h"
#include "rtc_wrapper.h"
#include "sensors.h"

// EEPROM адреса
#define EEPROM_ALARM_HOURS   0
#define EEPROM_ALARM_MINUTES 1
#define EEPROM_ALARM_ENABLED 2

// внутренние переменные — static
static byte alarm_hours   = 0;
static byte alarm_minutes = 0;
static bool alarm_enabled = false;
static bool snooze_active = false;
static unsigned long snooze_time = 0;
static unsigned long btn4_press_time = 0;
static bool btn4_was_pressed = false;

// загрузить будильник из EEPROM
static void load_alarm() {
    alarm_hours   = EEPROM.read(EEPROM_ALARM_HOURS);
    alarm_minutes = EEPROM.read(EEPROM_ALARM_MINUTES);
    alarm_enabled = EEPROM.read(EEPROM_ALARM_ENABLED);
    if (alarm_hours > 23)   alarm_hours = 0;
    if (alarm_minutes > 59) alarm_minutes = 0;
}

// сохранить будильник в EEPROM
static void save_alarm() {
    EEPROM.write(EEPROM_ALARM_HOURS,   alarm_hours);
    EEPROM.write(EEPROM_ALARM_MINUTES, alarm_minutes);
    EEPROM.write(EEPROM_ALARM_ENABLED, alarm_enabled);
}

// обновить статусный светодиод
static void update_led() {
    digitalWrite(RGB_GREEN_PIN, alarm_enabled ? HIGH : LOW);
    digitalWrite(RGB_RED_PIN,   alarm_enabled ? LOW  : HIGH);
}

// проверить нажатие кнопки
static bool btn_pressed(int pin) {
    if (digitalRead(pin) == LOW) {
        delay(50);
        return digitalRead(pin) == LOW;
    }
    return false;
}

// ждать отпускания кнопки
static void wait_release(int pin) {
    while (digitalRead(pin) == LOW) delay(10);
    delay(50);
}

// ============================================================
// INIT SCREEN
// ============================================================
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

// ============================================================
// CLOCK SCREEN
// ============================================================
enum screen clock_screen(struct context *ctx) {
    // показываем время
    struct dt t = now();
    char buf[17];
    snprintf(buf, sizeof(buf), "    %02d:%02d:%02d    ",
             t.hours, t.minutes, t.seconds);
    ctx->lcd->setCursor(0, 0);
    ctx->lcd->print(buf);

    // статус будильника
    if (alarm_enabled) {
        snprintf(buf, sizeof(buf), "ALM %02d:%02d      ",
                 alarm_hours, alarm_minutes);
    } else {
        snprintf(buf, sizeof(buf), "ALM OFF         ");
    }
    ctx->lcd->setCursor(0, 1);
    ctx->lcd->print(buf);

    // проверяем будильник
    if (alarm_enabled && !snooze_active &&
        t.hours == alarm_hours && t.minutes == alarm_minutes && t.seconds == 0) {
        return ALARM_SCR;
    }

    // snooze
    if (snooze_active && millis() - snooze_time >= 5UL * 60UL * 1000UL) {
        snooze_active = false;
        return ALARM_SCR;
    }

    // BTN4 — переключить или factory reset
    if (digitalRead(BTN4_PIN) == HIGH) {
        if (!btn4_was_pressed) {
            btn4_was_pressed = true;
            btn4_press_time = millis();
        }
        if (millis() - btn4_press_time >= FACTORY_RESET_INTERVAL) {
            wait_release(BTN4_PIN);
            btn4_was_pressed = false;
            return FACTORY_RESET_SCR;
        }
    } else if (btn4_was_pressed) {
        btn4_was_pressed = false;
        ctx->lcd->clear();
        return SHOW_DATE_SCR;
    }

    // BTN3 — настройка времени
    if (btn_pressed(BTN3_PIN)) {
        wait_release(BTN3_PIN);
        ctx->lcd->clear();
        struct dt t2 = now();
        byte h = t2.hours;
        byte m = t2.minutes;

        // настройка часов
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

        // настройка минут
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

// ============================================================
// SHOW DATE SCREEN
// ============================================================
enum screen show_date_screen(struct context *ctx) {
    struct dt t = now();
    char buf[17];
    snprintf(buf, sizeof(buf), "  %02d.%02d.%04d   ",
             t.day, t.month, t.year);
    ctx->lcd->setCursor(0, 0);
    ctx->lcd->print(buf);
    ctx->lcd->setCursor(0, 1);
    ctx->lcd->print("                ");

    // BTN4
    if (digitalRead(BTN4_PIN) == HIGH) {
        if (!btn4_was_pressed) {
            btn4_was_pressed = true;
            btn4_press_time = millis();
        }
        if (millis() - btn4_press_time >= FACTORY_RESET_INTERVAL) {
            wait_release(BTN4_PIN);
            btn4_was_pressed = false;
            return FACTORY_RESET_SCR;
        }
    } else if (btn4_was_pressed) {
        btn4_was_pressed = false;
        ctx->lcd->clear();
        return SHOW_ENV_SCR;
    }

    // BTN3 — настройка даты
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

// ============================================================
// SHOW ENV SCREEN
// ============================================================
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

    // BTN4
    if (digitalRead(BTN4_PIN) == HIGH) {
        if (!btn4_was_pressed) {
            btn4_was_pressed = true;
            btn4_press_time = millis();
        }
        if (millis() - btn4_press_time >= FACTORY_RESET_INTERVAL) {
            wait_release(BTN4_PIN);
            btn4_was_pressed = false;
            return FACTORY_RESET_SCR;
        }
    } else if (btn4_was_pressed) {
        btn4_was_pressed = false;
        ctx->lcd->clear();
        return CLOCK_SCR;
    }

    // BTN3 — настройка будильника
    if (btn_pressed(BTN3_PIN)) {
        wait_release(BTN3_PIN);
        ctx->lcd->clear();
        byte h = alarm_hours;
        byte m = alarm_minutes;

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

        alarm_hours   = h;
        alarm_minutes = m;
        alarm_enabled = true;
        save_alarm();
        update_led();
        ctx->lcd->clear();
    }

    delay(200);
    return SHOW_ENV_SCR;
}

// ============================================================
// ALARM SCREEN
// ============================================================
enum screen alarm_screen(struct context *ctx) {
    digitalWrite(BUZZER_PIN, HIGH);

    ctx->lcd->setCursor(0, 0);
    ctx->lcd->print("  *** ALARM ***  ");
    char buf[17];
    snprintf(buf, sizeof(buf), "    %02d:%02d       ",
             alarm_hours, alarm_minutes);
    ctx->lcd->setCursor(0, 1);
    ctx->lcd->print(buf);

    // BTN3 — выключить
    if (btn_pressed(BTN3_PIN)) {
        wait_release(BTN3_PIN);
        digitalWrite(BUZZER_PIN, LOW);
        ctx->lcd->clear();
        return CLOCK_SCR;
    }

    // BTN2 — snooze
    if (btn_pressed(BTN2_PIN)) {
        wait_release(BTN2_PIN);
        digitalWrite(BUZZER_PIN, LOW);
        snooze_active = true;
        snooze_time   = millis();
        ctx->lcd->clear();
        return CLOCK_SCR;
    }

    delay(200);
    return ALARM_SCR;
}

// ============================================================
// FACTORY RESET SCREEN
// ============================================================
enum screen factory_reset_screen(struct context *ctx) {
    digitalWrite(BUZZER_PIN, LOW);
    ctx->lcd->setCursor(0, 0);
    ctx->lcd->print("Factory Reset...");
    ctx->lcd->setCursor(0, 1);
    ctx->lcd->print("Please wait...  ");

    set_datetime(1, 1, 2000, 0, 0, 0);
    alarm_hours   = 0;
    alarm_minutes = 0;
    alarm_enabled = false;
    snooze_active = false;
    save_alarm();
    update_led();

    delay(2000);
    ctx->lcd->clear();
    return CLOCK_SCR;
}