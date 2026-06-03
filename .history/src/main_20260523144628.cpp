#include <Arduino.h>
#include <Wire.h>
#include <I2C_LCD.h>
#include <RtcDS1302.h>
#include "config.h"

static ThreeWire wire(RTC_DAT_PIN, RTC_CLK_PIN, RTC_RST_PIN);
static RtcDS1302<ThreeWire> rtc(wire);
static I2C_LCD lcd(LCD_I2C_ADDRESS);

int main() {
    init();
    Wire.begin();
    Serial.begin(9600);

    lcd.begin(16, 2);
    lcd.backlight();

    rtc.Begin();
    rtc.SetIsWriteProtected(false);
    rtc.SetIsRunning(true);
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    rtc.SetDateTime(compiled);

    for(;;) {
        RtcDateTime t = rtc.GetDateTime();
        char buf[17];
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
                 t.Hour(), t.Minute(), t.Second());
        lcd.setCursor(0, 0);
        lcd.print(buf);

        Serial.println(buf);
        delay(1000);
    }
}