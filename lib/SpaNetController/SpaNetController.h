#ifndef SPANETCONTROLLER_H
#define SPANETCONTROLLER_H

#include <Arduino.h>
#include <RemoteDebug.h>
#include <list>



#define UPDATEFREQUENCY 60000

extern RemoteDebug Debug;

class Register {
    public:
        Register(int req);


        bool updateRegister(const char update[]);
        bool isValid();
        char* getField(int field);

    private:
        int requiredFields;
        char reg[256];
        char* element[64];
        bool valid=false;


};



class SpaNetController {
    public:
        enum heat_pump_modes {automatic=0, heat=1, cool=2, off=3};
        
        float   getAmps();
        int     getVolts();
        float   getHpumpAmbTemp();
        float   getHpumpConTemp();
        float   getWaterTemp();
        bool    isHeatingOn();
        bool    isUVOn();
        bool    isSanatiseRunning();
        int     getSerialNo();

        float   getWaterTempSetPoint();
        bool    setWaterTempSetPoint(float temp);

        heat_pump_modes getHeatPumpMode();
        bool setHeatPumpMode(heat_pump_modes mode);

        bool isAuxHeatingEnabled();
        bool setAuxHeatingEnabled(bool enabled);

        bool isLightsOn();
        bool toggleLights();

        SpaNetController();
        ~SpaNetController();

        void tick();

        void subscribeUpdate(void (*u)(SpaNetController *));

        void forceUpdate();

        bool initialised();

    private:
        bool init = false;

        float amps; 
        int volts;
        float hpump_amb_temperature;
        float hpump_con_temperature;
        bool lightsOn;
        float waterTemperature;
        float waterTemperatureSetPoint;
        heat_pump_modes heatPumpMode;
        bool auxHeatElement;
        bool heatingActive;
        bool uvActive;
        bool sanatiseActive;

        int serialNo;

        Register registers[13]={1,33,30,31,29,31,34,15,15,15,17,33,18};

        ulong _nextUpdate=millis();
        ulong lastCommand = millis();

        void (*update)(SpaNetController *) = NULL;

        bool parseStatus(String str);
        String sendCommand(String cmd);
        bool pollStatus();

        std::list<String> commands;
        
        void processCommands();
        void getRegisters();


      

};

#endif