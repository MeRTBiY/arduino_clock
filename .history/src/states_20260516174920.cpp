#include <Arduino.h>
#include <EEPROM.h>
#include "states.h"
#include "config.h"
#include "lcd_wrapper.h"
#include "rtc_wrapper.h"
#include "sensors.h"

// ============================================================
// EEPROM адреса для сохранения будильника
// ============================================================
#define EEPROM_ALARM_HOURS   0
#define EEPROM_ALARM_MINUTES 1
#define EEPROM_ALARM_ENABLED 2

// ============================================================
// Внутренние переменные — static значит видны только здесь
// ============================================================
static byte alarm_hours   = 0;
static byte alarm_minutes = 0;
static bool alarm_enabled = false;
static bool alarm_ringing = false;
static bool snooze_active = false;
static unsigned long snooze_time = 0;

// для определения длинного нажатия BTN4
static unsigned long btn4_press_time = 0;
static bool btn4_was_pressed = false;

// ============================================================
// Вспомогательные функции
// ============================================================

// загрузить будильник из EEPROM
static void load_alarm() {
    alarm_hours   = EEPROM.read(EEPROM_ALARM_HOURS);
    alarm_minutes = EEPROM.read(EEPROM_ALARM_MINUTES);
    alarm_enabled = EEPROM.read(EEPROM_ALARM_ENABLED);

    // защита от мусора в EEPROM
    if (alarm_hours > 23)   alarm_hours = 0;
    if (alarm_minutes > 59) alarm_minutes = 0;
}

// сохранить будильник в EEPROM
static void save_alarm() {
    EEPROM.write(EEPROM_ALARM_HOURS,   alarm_hours);
    EEPROM.write(EEPROM_ALARM_MINUTES, alarm_minutes);
    EEPROM.write(EEPROM_ALARM_ENABLED, alarm_enabled);
}

// проверить нажата ли кнопка (INPUT_PULLUP — LOW значит нажата)
static bool btn_pressed(int pin) {
    if (digitalRead(pin) == LOW) {
        delay(50); // защита от дребезга
        return digitalRead(pin) == LOW;
    }
    return false;
}

// ждать пока кнопка отпущена
static void wait_release(int pin) {
    while (digitalRead(pin) == LOW) delay(10);
    delay(50);
}

// вывести время на LCD
static void display_time() {
    char buf[17];
    struct dt t = now();
    // форматируем с ведущими нулями
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
             t.hours, t.minutes, t.seconds);
    lcd_print_at(0, 4, buf);
}

// вывести статус будильника
static void display_alarm_status() {
    char buf[17];
    if (alarm_enabled) {
        snprintf(buf, sizeof(buf), "ALM %02d:%02d      ",
                 alarm_hours, alarm_minutes);
    } else {
        snprintf(buf, sizeof(buf), "ALM OFF         ");
    }
    lcd_print_at(1, 0, buf);
}

// ============================================================
// СОСТОЯНИЕ: CLOCK — показывает текущее время
// ============================================================
enum state state_clock() {
    // загружаем будильник при первом входе
    static bool first_run = true;
    if (first_run) {
        load_alarm();
        first_run = false;
        lcd_clear();
        lcd_print_at(0, 0, "    00:00:00    ");
    }

    // обновляем время на экране
    display_time();
    display_alarm_status();

    // проверяем будильник
    struct dt t = now();
    if (alarm_enabled && !snooze_active &&
        t.hours == alarm_hours && t.minutes == alarm_minutes && t.seconds == 0) {
        alarm_ringing = true;
        return ALARM;
    }

    // snooze — проверяем не истёк ли
    if (snooze_active) {
        if (millis() - snooze_time >= 5UL * 60UL * 1000UL) {
            snooze_active = false;
            alarm_ringing = true;
            return ALARM;
        }
    }

    // BTN4 — переключить состояние или FACTORY_RESET
    if (digitalRead(BTN4_PIN) == LOW) {
        if (!btn4_was_pressed) {
            btn4_was_pressed = true;
            btn4_press_time = millis();
        }
        // зажали 3 секунды — factory reset
        if (millis() - btn4_press_time >= FACTORY_RESET_INTERVAL) {
            wait_release(BTN4_PIN);
            btn4_was_pressed = false;
            return FACTORY_RESET;
        }
    } else if (btn4_was_pressed) {
        // короткое нажатие — переключить на дату
        btn4_was_pressed = false;
        lcd_clear();
        return SHOW_DATE;
    }

    // BTN3 — войти в настройку времени
    if (btn_pressed(BTN3_PIN)) {
        wait_release(BTN3_PIN);
        // настройка часов
        lcd_clear();
        struct dt t2 = now();
        byte h = t2.hours;
        byte m = t2.minutes;

        // настройка часов
        lcd_print_at(0, 0, "Set time:");
        while (true) {
            char buf[17];
            snprintf(buf, sizeof(buf), "H:%02d  M:%02d      ", h, m);
            lcd_print_at(1, 0, buf);

            if (btn_pressed(BTN2_PIN)) { // увеличить часы
                wait_release(BTN2_PIN);
                h = (h + 1) % 24;
            }
            if (btn_pressed(BTN1_PIN)) { // уменьшить часы
                wait_release(BTN1_PIN);
                h = (h + 23) % 24;
            }
            if (btn_pressed(BTN3_PIN)) { // перейти к минутам
                wait_release(BTN3_PIN);
                break;
            }
            delay(100);
        }

        // настройка минут
        lcd_print_at(0, 0, "Set minutes:    ");
        while (true) {
            char buf[17];
            snprintf(buf, sizeof(buf), "H:%02d  M:%02d      ", h, m);
            lcd_print_at(1, 0, buf);

            if (btn_pressed(BTN2_PIN)) {
                wait_release(BTN2_PIN);
                m = (m + 1) % 60;
            }
            if (btn_pressed(BTN1_PIN)) {
                wait_release(BTN1_PIN);
                m = (m + 59) % 60;
            }
            if (btn_pressed(BTN3_PIN)) { // подтвердить
                wait_release(BTN3_PIN);
                break;
            }
            delay(100);
        }

        set_time(h, m, 0);
        lcd_clear();
    }

    delay(200);
    return CLOCK;
}

// ============================================================
// СОСТОЯНИЕ: SHOW_DATE — показывает дату
// ============================================================
enum state state_show_date() {
    static bool first_run = true;
    if (first_run) {
        first_run = false;
        lcd_clear();
    }

    // показываем дату
    struct dt t = now();
    char buf[17];
    snprintf(buf, sizeof(buf), "  %02d.%02d.%04d   ",
             t.day, t.month, t.year);
    lcd_print_at(0, 0, buf);
    lcd_print_at(1, 0, "                ");

    // BTN4 — переключить на ENV
    if (digitalRead(BTN4_PIN) == LOW) {
        if (!btn4_was_pressed) {
            btn4_was_pressed = true;
            btn4_press_time = millis();
        }
        if (millis() - btn4_press_time >= FACTORY_RESET_INTERVAL) {
            wait_release(BTN4_PIN);
            btn4_was_pressed = false;
            first_run = true;
            return FACTORY_RESET;
        }
    } else if (btn4_was_pressed) {
        btn4_was_pressed = false;
        first_run = true;
        lcd_clear();
        return SHOW_ENV;
    }

    // BTN3 — настройка даты
    if (btn_pressed(BTN3_PIN)) {
        wait_release(BTN3_PIN);
        struct dt t2 = now();
        byte d  = t2.day;
        byte mo = t2.month;
        int  y  = t2.year;

        // настройка дня
        lcd_print_at(0, 0, "Set day:        ");
        while (true) {
            char b[17];
            snprintf(b, sizeof(b), "D:%02d M:%02d Y:%04d", d, mo, y);
            lcd_print_at(1, 0, b);
            if (btn_pressed(BTN2_PIN)) { wait_release(BTN2_PIN); d = (d % 31) + 1; }
            if (btn_pressed(BTN1_PIN)) { wait_release(BTN1_PIN); d = (d <= 1) ? 31 : d - 1; }
            if (btn_pressed(BTN3_PIN)) { wait_release(BTN3_PIN); break; }
            delay(100);
        }

        // настройка месяца
        lcd_print_at(0, 0, "Set month:      ");
        while (true) {
            char b[17];
            snprintf(b, sizeof(b), "D:%02d M:%02d Y:%04d", d, mo, y);
            lcd_print_at(1, 0, b);
            if (btn_pressed(BTN2_PIN)) { wait_release(BTN2_PIN); mo = (mo % 12) + 1; }
            if (btn_pressed(BTN1_PIN)) { wait_release(BTN1_PIN); mo = (mo <= 1) ? 12 : mo - 1; }
            if (btn_pressed(BTN3_PIN)) { wait_release(BTN3_PIN); break; }
            delay(100);
        }

        // настройка года
        lcd_print_at(0, 0, "Set year:       ");
        while (true) {
            char b[17];
            snprintf(b, sizeof(b), "D:%02d M:%02d Y:%04d", d, mo, y);
            lcd_print_at(1, 0, b);
            if (btn_pressed(BTN2_PIN)) { wait_release(BTN2_PIN); y++; }
            if (btn_pressed(BTN1_PIN)) { wait_release(BTN1_PIN); y--; }
            if (btn_pressed(BTN3_PIN)) { wait_release(BTN3_PIN); break; }
            delay(100);
        }

        set_date(d, mo, y);
        lcd_clear();
    }

    delay(200);
    return SHOW_DATE;
}

// ============================================================
// СОСТОЯНИЕ: SHOW_ENV — температура и влажность
// ============================================================
enum state state_show_env() {
    static bool first_run = true;
    if (first_run) {
        first_run = false;
        lcd_clear();
        lcd_print_at(0, 0, "Temp:           ");
        lcd_print_at(1, 0, "Humi:           ");
    }

    // читаем каждые SENSORS_READ_INTERVAL секунд
    static unsigned long last_read = 0;
    static float temp = 0;
    static int   humi = 0;

    if (millis() - last_read >= (unsigned long)SENSORS_READ_INTERVAL * 1000UL) {
        last_read = millis();
        temp = get_temperature();
        humi = get_humidity();

        char buf[17];
        snprintf(buf, sizeof(buf), "Temp: %.1f C    ", temp);
        lcd_print_at(0, 0, buf);
        snprintf(buf, sizeof(buf), "Humi: %d %%      ", humi);
        lcd_print_at(1, 0, buf);
    }

    // BTN4 — переключить на CLOCK
    if (digitalRead(BTN4_PIN) == LOW) {
        if (!btn4_was_pressed) {
            btn4_was_pressed = true;
            btn4_press_time = millis();
        }
        if (millis() - btn4_press_time >= FACTORY_RESET_INTERVAL) {
            wait_release(BTN4_PIN);
            btn4_was_pressed = false;
            first_run = true;
            return FACTORY_RESET;
        }
    } else if (btn4_was_pressed) {
        btn4_was_pressed = false;
        first_run = true;
        lcd_clear();
        return CLOCK;
    }

    // BTN3 — настройка будильника
    if (btn_pressed(BTN3_PIN)) {
        wait_release(BTN3_PIN);
        lcd_clear();

        byte h = alarm_hours;
        byte m = alarm_minutes;

        // настройка часов будильника
        lcd_print_at(0, 0, "Set alarm H:    ");
        while (true) {
            char buf[17];
            snprintf(buf, sizeof(buf), "H:%02d  M:%02d      ", h, m);
            lcd_print_at(1, 0, buf);
            if (btn_pressed(BTN2_PIN)) { wait_release(BTN2_PIN); h = (h + 1) % 24; }
            if (btn_pressed(BTN1_PIN)) { wait_release(BTN1_PIN); h = (h + 23) % 24; }
            if (btn_pressed(BTN3_PIN)) { wait_release(BTN3_PIN); break; }
            delay(100);
        }

        // настройка минут будильника
        lcd_print_at(0, 0, "Set alarm M:    ");
        while (true) {
            char buf[17];
            snprintf(buf, sizeof(buf), "H:%02d  M:%02d      ", h, m);
            lcd_print_at(1, 0, buf);
            if (btn_pressed(BTN2_PIN)) { wait_release(BTN2_PIN); m = (m + 1) % 60; }
            if (btn_pressed(BTN1_PIN)) { wait_release(BTN1_PIN); m = (m + 59) % 60; }
            if (btn_pressed(BTN3_PIN)) { wait_release(BTN3_PIN); break; }
            delay(100);
        }

        alarm_hours   = h;
        alarm_minutes = m;
        alarm_enabled = true;
        save_alarm();
        lcd_clear();
        first_run = true;
    }

    delay(200);
    return SHOW_ENV;
}

// ============================================================
// СОСТОЯНИЕ: ALARM — будильник звенит
// ============================================================
enum state state_alarm() {
    digitalWrite(BUZZER_PIN, HIGH);

    lcd_clear();
    lcd_print_at(0, 0, "  *** ALARM ***  ");

    char buf[17];
    snprintf(buf, sizeof(buf), "    %02d:%02d       ",
             alarm_hours, alarm_minutes);
    lcd_print_at(1, 0, buf);

    // BTN3 — выключить будильник
    if (btn_pressed(BTN3_PIN)) {
        wait_release(BTN3_PIN);
        digitalWrite(BUZZER_PIN, LOW);
        alarm_ringing = false;
        lcd_clear();
        return CLOCK;
    }

    // BTN2 — snooze на 5 минут
    if (btn_pressed(BTN2_PIN)) {
        wait_release(BTN2_PIN);
        digitalWrite(BUZZER_PIN, LOW);
        alarm_ringing = false;
        snooze_active = true;
        snooze_time   = millis();
        lcd_clear();
        return CLOCK;
    }

    delay(200);
    return ALARM;
}

// ============================================================
// СОСТОЯНИЕ: FACTORY_RESET — сброс настроек
// ============================================================
enum state state_factory_reset() {
    digitalWrite(BUZZER_PIN, LOW);

    lcd_clear();
    lcd_print_at(0, 0, "Factory Reset...");
    lcd_print_at(1, 0, "Please wait...  ");

    // сброс времени
    set_datetime(1, 1, 2000, 0, 0, 0);

    // сброс будильника
    alarm_hours   = 0;
    alarm_minutes = 0;
    alarm_enabled = false;
    alarm_ringing = false;
    snooze_active = false;
    save_alarm();

    delay(2000);
    lcd_clear();
    return CLOCK;
}