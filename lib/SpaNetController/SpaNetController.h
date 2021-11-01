#ifndef SPANETCONTROLLER_H
#define SPANETCONTROLLER_H

#include <Arduino.h>
#include <RemoteDebug.h>

#define UPDATEFREQUENCY 60000

extern RemoteDebug Debug;

class SpaNetController {
    private:
        float amps;
        int volts;
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