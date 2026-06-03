#include <Arduino.h>

int main() {
    init();
    Serial.begin(9600);
    pinMode(12, INPUT);
    
    for(;;) {
        Serial.println(digitalRead(12));
        delay(200);
    }
}