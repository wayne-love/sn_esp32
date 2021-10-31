#include <Arduino.h>

class Blinker {
    public:
        Blinker(int pin);
        void setInterval(int i);
        void tick();
  

    private:
        bool state = false;
        int interval = 1000;
        ulong lastUpdate = millis();
        bool ledOn = false;
        int ledPin;
};

