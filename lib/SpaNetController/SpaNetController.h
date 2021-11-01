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
        float hpump_amb_temperature;
        ulong _nextUpdate=millis();
        void (*update)(SpaNetController *) = NULL;

        bool parseStatus(String str);
        String sendCommand(String cmd);
        bool pollStatus();




    public:
        float getAmps();
        int getVolts();
        float getHpumpAmbTemp();
        
        SpaNetController();
        ~SpaNetController();

        void tick();
        void subscribeUpdate(void (*u)(SpaNetController *));
};

#endif