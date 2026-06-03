#include <Arduino.h>
#include <DHT.h>
#include "sensors.h"
#include "config.h"

// объект DHT — глобальный потому что нужен в нескольких функциях
static DHT dht(DHT_PIN, DHT11);

// инициализация — вызывается один раз в setup()
void sensors_init() {
    dht.begin();
}

// возвращает температуру в градусах Цельсия
float get_temperature() {
    return dht.readTemperature();
}

// возвращает влажность в процентах
int get_humidity() {
    return (int)dht.readHumidity();
}