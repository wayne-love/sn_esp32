#ifndef SPANETINTERFACE_H
#define SPANETINTERFACE_H

#include <Arduino.h>
#include <RemoteDebug.h>
#include <functional>
#include <stdexcept>
#include "SpaNetProperties.h"
#include <RemoteDebug.h>


#define UPDATEFREQUENCY 60000 //(ms) Frequency to poll controller when idle.
#define FAILEDREADFREQUENCY 1000 //(ms) Frequency to retry on a failed read of the status registers.

extern RemoteDebug Debug;

class SpaNetInterface : public SpaNetProperties {
    private:

        /// @brief Minimum number of fields in status response for it to be considered valid.
        int statusResponseRawMin = 288;

        /// @brief Maximum number of fields in status response.
        int statusResponseRawMax = 350;

        /// @brief Each field of the RF cmd response as seperate elements.
        String statusResponseRaw[350];

        int R2;
        int R3;
        int R4;
        int R5;
        int R6;
        int R7;
        int R9;
        int RA;
        int RB;
        int RC;
        int RE;
        int RG;

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

        /// @brief Updates the attributes by sending the RF command and parsing the result.
        void updateStatus();

        void flushSerialReadBuffer();


        /// @brief Stores millis time at which next update should occur
        unsigned long _nextUpdateDue = 0;

        bool _initialised = false;

        /// @brief If the result registers have been modified locally, need to do a fress pull from the controller
        bool _resultRegistersDirty = true;

   
        void (*updateCallback)() = nullptr;

        long _lastWaitMessage = millis();



    public:
        /// @brief Init SNI to read from stream p. This stream needs to be started and configured baud 38400/8N1.
        /// useful if you want to link this to a software COM port on a chip similar to a ESP8266.
        /// Needs to be set to 38400/8/N/1
        /// @param p 
        SpaNetInterface(Stream &p);

        /// @brief Init SNI and use Serial2 for comms.  Serial 2 DOES NOT need to be configured.
        SpaNetInterface();

        ~SpaNetInterface();

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
        
        /// @brief Set Heat pump operating mode (0 --> 3, {auto, heat, cool, off})
        /// @param mode 
        /// @return Returns True if succesful
        bool setHPMP(int mode);
        bool setHPMP(String mode);

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

        /// @brief Set aux element operating mode
        /// @param mode 0 = off, 1 = on
        /// @return True if successful
        bool setHELE(int mode);
};


#endif