#ifndef SPANETCONTROLLER_H
#define SPANETCONTROLLER_H

#include <Arduino.h>
#include <RemoteDebug.h>
#include <list>

#define UPDATEFREQUENCY 60000 //(ms) Frequency to poll controller when idle.

extern RemoteDebug Debug; //Must be defined / called in main.c 


class Register {
    public:
        /**
         * @brief Construct a new Register object
         * 
         * @param req Number of fields that we are expecting
         */
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



class Pump {
    public:
    #define PUMP_MODES_COUNT 5 //Maximum number of pumps supported.
        static const char *pump_modes[PUMP_MODES_COUNT];  // Off = 0, On = 1, Auto = 4

        void initialise(bool installed, bool autoOperation);
        bool isInstalled();
        bool isAutoModeSupported();

        void setOperatingMode(int mode);
        void setOperatingMode(const char *mode);
        int getOperatingMode();
        
    private:
        bool _installed;
        bool _autoOperation;
        int _mode;
};


class SpaNetController {
    public:
        class Light{
            public:
                byte getBrightness();
                void setBrightness(byte value);

                static const char *light_modes[5]; // "White", "Color", "Fade", "Step", "Party"
                const char *getMode();
                void setMode(const char *mode);
                void setMode(byte mode);

                static const byte colour_map[25];
                void setColour(int colour);
                int getColour();

                bool isOn();
                void setIsOn(bool state);
        
                friend class SpaNetController;

                Light(SpaNetController* p);
                ~Light();

            private:
                SpaNetController* _parent;
                bool _isOn = false;
                byte _mode = 0; // min = 0, max = 4
                byte _brightness = 5; // min = 1, max = 5
                int _colour = 0; // 0 to 30
        };

        Light lights;

        enum heat_pump_modes {automatic=0, heat=1, cool=2, off=3};
        
        float   getAmps();
        int     getVolts();
        float   getHpumpAmbTemp();
        float   getHpumpConTemp();
        float   getWaterTemp();
        bool    isHeatingOn();
        bool    isUVOn();
        bool    isSanatiseRunning();
        String  getSerialNo();
        char    *getStatus();
        float   getHeaterTemp();

        /// @brief getPower()
        /// @return Returns the current power draw (W)
        float   getPower();
        /// @brief getKWH()
        /// @return Retuns KWH consumption since comisssioning (kWH)
        float   getTotalEnergy();
        /// @brief getPowerToday()
        /// @return Returns power consumed today (kWH)
        float   getEnergyToday();

        
        bool    pumpInstalled(int pump);
        Pump    *getPump(int pump);

        float   getWaterTempSetPoint();
        bool    setWaterTempSetPoint(float temp);

        heat_pump_modes getHeatPumpMode();
        bool    setHeatPumpMode(heat_pump_modes mode);

        bool    isAuxHeatingEnabled();
        bool    setAuxHeatingEnabled(bool enabled);

        bool    setPumpOperating(int pump, int mode);
        void    setPumpOperating(int pump, const char *mode);
        bool    setPump1Operating(int mode);
        bool    setPump2Operating(int mode);
        bool    setPump3Operating(int mode);
        bool    setPump4Operating(int mode);
        bool    setPump5Operating(int mode);

        void    queueCommand(String command);

        SpaNetController();
        ~SpaNetController();

        void tick();

        void subscribeUpdate(void (*u)(SpaNetController *));

        void forceUpdate();

        bool initialised();

        Pump pumps[5];

    private:
        bool _firstrun = false;

        float amps; 
        int volts;
        float hpump_amb_temperature;
        float hpump_con_temperature;
        float heater_temperature;

        float waterTemperature;
        float waterTemperatureSetPoint;
        heat_pump_modes heatPumpMode;
        bool auxHeatElement;
        bool heatingActive;
        bool uvActive;
        bool sanatiseActive;
        char* status;

        float instEnergy, totalPower, powerToday;

        String serialNo;

        Register registers[13]={1,33,30,31,29,31,34,15,15,15,17,33,18};

        /**
         * @brief time (ms) of next read of spa registers
         * 
         */
        ulong _nextUpdate=millis();

        /**
         * @brief time (ms) of last command queue check/publish
         * 
         */
        ulong lastCommand = millis();

        void (*update)(SpaNetController *) = NULL;

        bool parseStatus(String str);

        /**
         * @brief Sends command to spa controller. Use with care! Preferable use queueCommand to que commands for batch processing.
         * 
         * @param cmd 
         * @return String 
         * 
         */
        String sendCommand(String cmd);

        bool pollStatus();

        std::list<String> commands;
        
        void processCommands();
        void getRegisters();
};


#endif