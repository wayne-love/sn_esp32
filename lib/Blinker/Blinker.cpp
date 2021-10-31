#include "Blinker.h"

Blinker::Blinker(int pin){
    pinMode(pin, OUTPUT);
    ledPin = pin;
};

void Blinker::setInterval(int i) {
    interval = i;
};

void Blinker::tick(){
    ulong currentTime = millis();

    if (currentTime>lastUpdate+interval){
        ledOn = !ledOn;
        lastUpdate = currentTime;
        if (ledOn) {
            digitalWrite(ledPin, HIGH);
        } else {
            digitalWrite(ledPin, LOW);
        }
    }
};