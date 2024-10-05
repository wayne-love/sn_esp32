#ifndef SPAINTERFACE_H
#define SPAINTERFACE_H

#include <Arduino.h>
#include <RemoteDebug.h>
#include <functional>
#include <stdexcept>
#include "SpaProperties.h"


#define UPDATEFREQUENCY 60000 //(ms) Frequency to poll controller when idle.
#define FAILEDREADFREQUENCY 1000 //(ms) Frequency to retry on a failed read of the status registers.

extern RemoteDebug Debug;

class SpaInterface : public SpaProperties {
    private:

        /// @brief Number of fields that we can expect to read.
        int statusResponseMinFields = 289;
        static const int statusResponseMaxFields = 295;
        
        /// @brief Each field of the RF cmd response as seperate elements.
        String statusResponseRaw[statusResponseMaxFields];

        int R2=-1;
        int R3=-1;
        int R4=-1;
        int R5=-1;
        int R6=-1;
        int R7=-1;
        int R9=-1;
        int RA=-1;
        int RB=-1;
        int RC=-1;
        int RE=-1;
        int RG=-1;

        /// @brief Does the status response array contain valid information?
        bool validStatusResponse = false;

        /// @brief Serial stream to interface to SpanNet hardware.
        Stream &port;

        /// @brief Read from serial interface, expect it to contain return from RF command
        /// @return true if successful read, false if there was a corrupted read
        bool readStatus();

        void updateMeasures();



        /// @brief Sends command to SpaNet controller.  Result must be read by some other method.
        /// Used for the 'RF' command so that we can do a optomised read of the return array.
        /// @param cmd - cmd to be executed.
        void sendCommand(String cmd);

        
        /// @brief Sends a command to the SpanNet controller and returns the result string
        /// @param cmd - cmd to be executed
        /// @return String - result string
        String sendCommandReturnResult(String cmd);

        /// @brief Sends the command and checks the result against the expected outcome
        /// @param cmd command to send
        /// @param expected expected string response
        /// @return result
        bool sendCommandCheckResult(String cmd, String expected);

        /// @brief Updates the attributes by sending the RF command and parsing the result.
        void updateStatus();

        void flushSerialReadBuffer();


        /// @brief Stores millis time at which next update should occur
        unsigned long _nextUpdateDue = 0;

        /// @brief False until first successful read of the registers.
        bool _initialised = false;

        /// @brief If the result registers have been modified locally, need to do a fress pull from the controller
        bool _resultRegistersDirty = true;

   
        void (*updateCallback)() = nullptr;

        u_long _lastWaitMessage = millis();



    public:
        /// @brief Init SNI to read from stream p. This stream needs to be started and configured baud 38400/8N1.
        /// useful if you want to link this to a software COM port on a chip similar to a ESP8266.
        /// Needs to be set to 38400/8/N/1
        /// @param p 
        SpaInterface(Stream &p);

        /// @brief Init SNI and use Serial2 for comms.  Serial 2 DOES NOT need to be configured.
        SpaInterface();

        ~SpaInterface();

        /// @brief Complete RF command response in a single string
        Property<String> statusResponse;

        /// @brief To be called by loop function of main sketch.  Does regular updates, etc.
        void loop();

        /// @brief Have we sucessfuly read the registers from the SpaNet controller.
        /// @return 
        bool isInitialised();

        /// @brief Set the function to be called when properties have been updated.
        /// @param f 
        void setUpdateCallback(void (*f)());

        /// @brief Clear the call back function.
        void clearUpdateCallback();

        /// @brief Set the desired water temperature
        /// @param temp Between 5 and 40 in 0.5 increments
        /// @return Returns True if succesful
        bool setSTMP(int temp);

        /// @brief Set snooze day ({128,127,96,31} -> {"Off","Everyday","Weekends","Weekdays"};)
        /// @param mode
        /// @return Returns True if succesful
        bool setL_1SNZ_DAY(int mode);

        /// @brief Set snooze time (provide an integer that uses this calculation HH:mm > HH*265+mm. e.g. 13:47 = 13*256+47 = 3375)
        /// @param mode
        /// @return Returns True if succesful
        bool setL_1SNZ_BGN(int mode);
        bool setL_1SNZ_END(int mode);

        /// @brief Set snooze day ({128,127,96,31} -> {"Off","Everyday","Weekends","Weekdays"};)
        /// @param mode
        /// @return Returns True if succesful
        bool setL_2SNZ_DAY(int mode);

        /// @brief Set snooze time (provide an integer that uses this calculation HH:mm > HH*265+mm. e.g. 13:47 = 13*256+47 = 3375)
        /// @param mode
        /// @return Returns True if succesful
        bool setL_2SNZ_BGN(int mode);
        bool setL_2SNZ_END(int mode);

        /// @brief Set Heat pump operating mode (0 --> 3, {auto, heat, cool, off})
        /// @param mode 
        /// @return Returns True if succesful
        bool setHPMP(int mode);
        bool setHPMP(String mode);

        /// @brief Set light mode (0 = white, 1 = colour, 2 = step, 3 = fade, 4 = party)
        /// @param mode
        /// @return Returns True if succesful
        bool setColorMode(int mode);
        bool setColorMode(String mode);

        /// @brief Set light brightness (min 1, max 5)
        /// @param mode
        /// @return Returns True if succesful
        bool setLBRTValue(int mode);

        /// @brief Set light effect speed (min 1, max 5)
        /// @param mode
        /// @return Returns True if succesful
        bool setLSPDValue(int mode);
        bool setLSPDValue(String mode);

        /// @brief Set light colour (min 0, max 31)
        /// @param mode
        /// @return Returns True if succesful
        bool setCurrClr(int mode);

        /// @brief Set the operating mode for pump 1
        /// @param mode 0 = off, 1 = on, 4 = auto (if supported)
        /// @return True if successful
        bool setRB_TP_Pump1(int mode);

        /// @brief Set the operating mode for pump 2
        /// @param mode 0 = off, 1 = on, 4 = auto (if supported)
        /// @return True if successful
        bool setRB_TP_Pump2(int mode);

        /// @brief Set the operating mode for pump 3
        /// @param mode 0 = off, 1 = on, 4 = auto (if supported)
        /// @return True if successful
        bool setRB_TP_Pump3(int mode);

        /// @brief Set the operating mode for pump 4
        /// @param mode 0 = off, 1 = on, 4 = auto (if supported)
        /// @return True if successful
        bool setRB_TP_Pump4(int mode);

        /// @brief Set the operating mode for pump 5
        /// @param mode 0 = off, 1 = on, 4 = auto (if supported)
        /// @return True if successful
        bool setRB_TP_Pump5(int mode);

        bool setRB_TP_Light(int mode);

        /// @brief Set aux element operating mode
        /// @param mode 0 = off, 1 = on
        /// @return True if successful
        bool setHELE(int mode);

        /// @brief Sets the clock on the spa
        /// @param t Time
        /// @return True if successful
        bool setSpaTime(time_t t);

        /// @brief Controls the air blower
        /// @param mode 0 = Varible, 1 = Ramp, 2 = Off
        /// @return True if successful
        bool setOutlet_Blower(int mode);

        /// @brief Set the speed of the air blower
        /// @param mode 1 = low, 5 = high
        /// @return True if successful
        bool setVARIValue(int mode);

        /// @brief Set Spa mode (0 --> 4, {"NORM","ECON","AWAY","WEEK"};)
        /// @param mode
        /// @return Returns True if succesful
        bool setMode(int mode);
        bool setMode(String mode);
};

#endif