#include <Arduino.h>

int main() {
    init();
    Serial.begin(9600);
    pinMode(11, INPUT);
    
    for(;;) {
        Serial.println(digitalRead(11));
        delay(200);
    }
}