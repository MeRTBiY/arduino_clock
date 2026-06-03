#include <Arduino.h>
#include <I2C_LCD.h>
#include "lcd_wrapper.h"
#include "config.h"

// объект LCD — static, виден только в этом файле
static I2C_LCD lcd(LCD_I2C_ADDRESS);

void lcd_init() {
    Wire.begin();
    lcd.begin(LCD_COLS, LCD_ROWS);
    lcd.backlight();
}

void lcd_clear() {
    lcd.clear();
}

void lcd_set_cursor(int y, int x) {
    lcd.setCursor(x, y);
}

void lcd_print(char* text) {
    lcd.print(text);
}

void lcd_print_at(int y, int x, char* text) {
    lcd.setCursor(x, y);
    lcd.print(text);
}

void lcd_backlight(bool state) {
    if (state) lcd.backlight();
    else lcd.noBacklight();
}