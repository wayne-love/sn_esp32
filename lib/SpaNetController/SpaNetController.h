#ifndef SPANETCONTROLLER_H
#define SPANETCONTROLLER_H

#include <Arduino.h>


#define UPDATEFREQUENCY 5000

class SpaNetController {
    private:
        float amps;
        int volts;
        ulong _lastUpdate=millis()-UPDATEFREQUENCY;
        ulong _nextUpdate=millis();
        void (*update)() = NULL;

        bool parseStatus(String str);
        String sendCommand(String cmd);
        bool pollStatus();


    public:
        float getAmps();
        int getVolts();
        
        SpaNetController();
        ~SpaNetController();

        void tick();
        void subscribeUpdate(void (*u)());
};

#endif