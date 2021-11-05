#ifndef SPANETCONTROLLER_H
#define SPANETCONTROLLER_H

#include <Arduino.h>
#include <RemoteDebug.h>
#include <list>



#define UPDATEFREQUENCY 60000

extern RemoteDebug Debug;

class SpaNetController {
    private:
        float amps;
        int volts;
        
        float hpump_amb_temperature;

        bool lightsOn;

        ulong _nextUpdate=millis();
        ulong lastCommand = millis();

        void (*update)(SpaNetController *) = NULL;

        bool parseStatus(String str);
        String sendCommand(String cmd);
        bool pollStatus();

        struct command {
            String topic;
            String payload;
        };

        std::list<command> commands;
        
        void processCommands();
        void getRegisters();

      
    public:
        float getAmps();
        int getVolts();
        float getHpumpAmbTemp();
        bool isLightsOn();
        
        bool setLightsOn(bool flag);
        
        SpaNetController();
        ~SpaNetController();

        void tick();
        void subscribeUpdate(void (*u)(SpaNetController *));
        void pushCommand(String topic, String payload);
};

#endif