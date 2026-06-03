#include <Arduino.h>
#include <RtcDS1302.h>
#include "rtc_wrapper.h"
#include "config.h"

// пины для DS1302
static ThreeWire wire(RTC_CLK_PIN, RTC_DAT_PIN, RTC_RST_PIN);
static RtcDS1302<ThreeWire> rtc(wire);

void clock_init() {
    rtc.Begin();
    // если RTC не работал — ставим время компиляции
    if (!rtc.IsDateTimeValid() || rtc.GetIsWriteProtected()) {
        rtc.SetIsWriteProtected(false);
        rtc.SetIsRunning(true);
        RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
        rtc.SetDateTime(compiled);
    }
}

void set_date(const byte day, const byte month, const int year) {
    RtcDateTime current = rtc.GetDateTime();
    RtcDateTime updated(year, month, day,
                        current.Hour(), current.Minute(), current.Second());
    rtc.SetDateTime(updated);
}

void set_time(const byte hours, const byte minutes, const byte seconds) {
    RtcDateTime current = rtc.GetDateTime();
    RtcDateTime updated(current.Year(), current.Month(), current.Day(),
                        hours, minutes, seconds);
    rtc.SetDateTime(updated);
}

void set_datetime(const byte day, const byte month, const int year,
                  const byte hours, const byte minutes, const byte seconds) {
    RtcDateTime updated(year, month, day, hours, minutes, seconds);
    rtc.SetDateTime(updated);
}

byte get_day()     { return rtc.GetDateTime().Day(); }
byte get_month()   { return rtc.GetDateTime().Month(); }
int  get_year()    { return rtc.GetDateTime().Year(); }
byte get_hours()   { return rtc.GetDateTime().Hour(); }
byte get_minutes() { return rtc.GetDateTime().Minute(); }
byte get_seconds() { return rtc.GetDateTime().Second(); }

struct dt now() {
    RtcDateTime t = rtc.GetDateTime();
    struct dt result;
    result.day     = t.Day();
    result.month   = t.Month();
    result.year    = t.Year();
    result.hours   = t.Hour();
    result.minutes = t.Minute();
    result.seconds = t.Second();
    return result;
}