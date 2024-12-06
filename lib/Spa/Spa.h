// lib/Spa/Spa.h
#pragma once
#include <Arduino.h>
#include "Variable.h"
#include <RemoteDebug.h>

extern RemoteDebug Debug;

/**
 * @brief Main controller class for spa functionality
 */
class Spa {
private:

    /// @brief Serial stream to interface to SpanNet hardware.
    Stream &port;


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

    /**
     * @brief Flushes the serial read buffer.
     *
     * This function clears any data that is currently in the serial read buffer.
     * It is useful for ensuring that no old or unwanted data is left in the buffer
     * before starting a new read operation.
     */
    void clearSerialReadBuffer();

    /// @brief If the result registers have been modified locally, need to do a fress pull from the controller
    bool _resultRegistersDirty = true;

    /**
     * @brief Converts a time string to a uint16_t
     * 
     * @param time The time string in the format HH:MM
     * @return The time as a uint16_t
     */
    uint16_t timeStringToUint16(const String& time);

    bool setColorMode(byte mode);

    bool setRB_TP_Pump1(byte mode);
    bool setRB_TP_Pump2(byte mode);
    bool setRB_TP_Pump3(byte mode);
    bool setRB_TP_Pump4(byte mode);
    bool setRB_TP_Pump5(byte mode);
    
    bool setSTMP(uint16_t temp);
    bool setHPMP(byte mode);

    bool setL_1SNZ_DAY(byte mode);
    bool setL_1SNZ_BGN(uint16_t time);
    bool setL_1SNZ_END(uint16_t time);
    bool setL_2SNZ_DAY(byte mode);
    bool setL_2SNZ_BGN(uint16_t time);
    bool setL_2SNZ_END(uint16_t time);

    bool setLBRTValue(byte brightness);
    bool setLSPDValue(byte speed);
    bool setCurrClr(byte colour);
    bool setRB_TP_Light(bool mode);

public:
    /**
     * @brief Initialize spa controller
     */
    Spa();

    /**
     * @brief Spa target temperature setting
     * 
     * @details The value of this property is the target temperature in degrees Celsius * 10
     * eg 382 = 38.2°C. The temperature steps are in 0.2°C increments.  If a 0.1°C increment is
     * sent then it will be rounded down to the nearest 0.2°C increment.
     * 
     * @note Valid range: 10.0°C to 41.0°C
     */
    Variable<uint16_t> STMP;

    /**
     * @brief Heapump mode
     * 
     * @details Disabling the heatpump does not disable heating.  It just disables the heatpump.
     * 
     * @note Valid values: 0 = Auto, 1 = Heat, 2 = Cool, 3 = disabled
     */
    Variable<byte> HPMP = Variable<byte>(
        0, {
            {"Auto", 0},
            {"Heat", 1},
            {"Cool", 2},
            {"Disabled", 3}
        }
    );

    /**
     * @brief Represents the state of Pump 1 with various modes.
     * 
     * @note Can take the following values:
     * - "Off"  : 0
     * - "On"   : 1
     * - "Low"  : 2
     * - "High" : 3
     * - "Auto" : 4
     */
    Variable<byte> RB_TP_Pump1 = Variable<byte>(
        0, {
            {"Off", 0},
            {"On", 1},
            {"Low", 2},
            {"High", 3},
            {"Auto", 4}
        }
    );

    /**
     * @brief Represents the state of Pump 2 with various modes.
     * 
     * @note Can take the following values:
     * - "Off"  : 0
     * - "On"   : 1
     * - "Low"  : 2
     * - "High" : 3
     * - "Auto" : 4
     */
    Variable<byte> RB_TP_Pump2 = Variable<byte>(
        0, {
            {"Off", 0},
            {"On", 1},
            {"Low", 2},
            {"High", 3},
            {"Auto", 4}
        }
    );

    /**
     * @brief Represents the state of Pump 3 with various modes.
     * 
     * @note Can take the following values:
     * - "Off"  : 0
     * - "On"   : 1
     * - "Low"  : 2
     * - "High" : 3
     * - "Auto" : 4
     */
    Variable<byte> RB_TP_Pump3 = Variable<byte>(
        0, {
            {"Off", 0},
            {"On", 1},
            {"Low", 2},
            {"High", 3},
            {"Auto", 4}
        }
    );

    /**
     * @brief Represents the state of Pump 4 with various modes.
     * 
     * @note Can take the following values:
     * - "Off"  : 0
     * - "On"   : 1
     * - "Low"  : 2
     * - "High" : 3
     * - "Auto" : 4
     */
    Variable<byte> RB_TP_Pump4 = Variable<byte>(
        0, {
            {"Off", 0},
            {"On", 1},
            {"Low", 2},
            {"High", 3},
            {"Auto", 4}
        }
    );

    /**
     * @brief Represents the state of Pump 5 with various modes.
     * 
     * @note Can take the following values:
     * - "Off"  : 0
     * - "On"   : 1
     * - "Low"  : 2
     * - "High" : 3
     * - "Auto" : 4
     */
    Variable<byte> RB_TP_Pump5 = Variable<byte>(
        0, {
            {"Off", 0},
            {"On", 1},
            {"Low", 2},
            {"High", 3},
            {"Auto", 4}
        }
    );

    /// @brief Sleep timer 1 active days
    ///
    /// 128 = off, 127 = every day, 96 = weekends, 31 = weekdays
    Variable<byte> L_1SNZ_DAY = Variable<byte>(
        127, {
            {"Off", 128},
            {"Everyday", 127},
            {"Weekends", 96},
            {"Weekdays", 31}
        }
    );
    
    /// @brief Sleep timer 2 active days
    ///
    /// 128 = off, 127 = every day, 96 = weekends, 31 = weekdays
    Variable<byte> L_2SNZ_DAY = Variable<byte>(
        127, {
            {"Off", 128},
            {"Everyday", 127},
            {"Weekends", 96},
            {"Weekdays", 31}
        }
    );

    /**
     * @brief Sleep time 1 start time
     * @details Takes either uint16_t or string format HH:MM if uint16_t is used, the formula is h*256+m
     * eg 20:00 = 20*256+0 = 5120; 13:47 = 13*256+47 = 3375
     */
    Variable<uint16_t> L_1SNZ_BGN;

    /**
     * @brief Sleep time 2 start time
     * @details Takes either uint16_t or string format HH:MM if uint16_t is used, the formula is h*256+m
     * eg 20:00 = 20*256+0 = 5120; 13:47 = 13*256+47 = 3375
     */
    Variable<uint16_t> L_2SNZ_BGN;

    /**
     * @brief Sleep time 1 end time
     * @details Takes either uint16_t or string format HH:MM if uint16_t is used, the formula is h*256+m
     * eg 20:00 = 20*256+0 = 5120; 13:47 = 13*256+47 = 3375
     */
    Variable<uint16_t> L_1SNZ_END;

    /**
     * @brief Sleep time 2 end time
     * @details Takes either uint16_t or string format HH:MM if uint16_t is used, the formula is h*256+m
     * eg 20:00 = 20*256+0 = 5120; 13:47 = 13*256+47 = 3375
     */
    Variable<uint16_t> L_2SNZ_END;

    /**
     * @brief Light effect mode
     * 
     * @details Integer value representing the light effect mode
     * 
     * @note Valid values: 0 = White, 1 = Color, 3 = Fade, 2 = Step, 4 = Party
     */

    Variable<byte> ColorMode = Variable<byte>(
        0, {
            {"White", 0},
            {"Color", 1},
            {"Fade", 3},
            {"Step", 2},
            {"Party", 4}
        }
    );

    /**
     * @brief Light brightness
     * @details Range: 1 (dim) - 5 (bright)
     */
    Variable<byte> LBRTValue;

    /**
     * @brief Light effect speed
     * @details Range: 1 (slow) - 5 (fast)
     */
    Variable<byte> LSPDValue;

    /**
     * @brief Light colour
     * @details Range: 0 - 31
     */
    Variable<byte> CurrClr; 

    /**
     * @brief Light status (on/off)
     * @details true = on, false = off
     */
    Variable<bool> RB_TP_Light;

#pragma Properties
#pragma region R2
    /// @brief Mains current draw
    /// Monitors total system current draw for load shedding
    ReadOnlyVariable<int> MainsCurrent;

    /// @brief Mains voltage measurement
    /// @details Range: 200-260V AC
    /// Operating voltage with under/over voltage protection
    ReadOnlyVariable<int> MainsVoltage;

    /// @brief Internal controller case temperature
    /// @details Range: 0-70°C
    /// Critical for thermal protection of control system
    ReadOnlyVariable<int> CaseTemperature;

    /// @brief 12V DC auxiliary port current
    /// @details Range: 0-1000mA
    /// Monitors low voltage accessory load
    ReadOnlyVariable<int> PortCurrent;

    /// @brief Real-time clock for scheduling
    /// @details Used for filtration cycles and sleep mode timing
    /// Maintains time during power failure
    ReadOnlyVariable<time_t> SpaTime;

    /// @brief Heater element temperature sensor
    /// Primary temperature control and safety cutout sensor
    ReadOnlyVariable<int> HeaterTemperature;

    /// @brief Pool water temperature sensor
    /// @details Range: 5-41°C
    /// May read incorrectly during high flow conditions
    ReadOnlyVariable<int> PoolTemperature;

    /// @brief Water level detection
    /// @details true = adequate water level, false = low water condition
    /// Prevents dry running of pumps and heater
    ReadOnlyVariable<bool> WaterPresent;

    /// @brief Sleep mode countdown timer
    /// @details Range: 0-999 minutes
    /// Controls auto-sleep functionality
    ReadOnlyVariable<int> AwakeMinutesRemaining;

    /// @brief Total filtration pump runtime
    /// @details Accumulated runtime in minutes for maintenance tracking
    ReadOnlyVariable<int> FiltPumpRunTimeTotal;

    /// @brief Required daily filtration time
    /// @details Target runtime in minutes per 24h period
    /// Automatically adjusted based on usage and temperature
    ReadOnlyVariable<int> FiltPumpReqMins;

    /// @brief Equipment protection timer
    /// @details Safety timeout in seconds for pump/heater protection
    ReadOnlyVariable<int> LoadTimeOut;

    /// @brief Total system runtime
    /// @details Accumulated hours since installation
    /// Used for service interval tracking
    ReadOnlyVariable<int> HourMeter;

    /// @brief Relay1 count
    ReadOnlyVariable<int> Relay1;

    /// @brief Relay2 count
    ReadOnlyVariable<int> Relay2;

    /// @brief Relay3 count
    ReadOnlyVariable<int> Relay3;

    /// @brief Relay4 count
    ReadOnlyVariable<int> Relay4;

    /// @brief Relay5 count
    ReadOnlyVariable<int> Relay5;

    /// @brief Relay6 count
    ReadOnlyVariable<int> Relay6;

    /// @brief Relay7 count
    ReadOnlyVariable<int> Relay7;

    /// @brief Relay8 count
    ReadOnlyVariable<int> Relay8;

    /// @brief Relay9 count
    ReadOnlyVariable<int> Relay9;
#pragma endregion
#pragma region R3
    /// @brief Maximum current limit setting
    /// Should be set to the circuit breaker rating
    ReadOnlyVariable<int> CLMT;

    /// @brief Number of power phases in use
    /// @details Values: 1, 2, 3
    /// Defines electrical installation type
    ReadOnlyVariable<int> PHSE;

    /// @brief Phase 1 load limit
    /// @details Range: 1-5 devices
    /// Maximum concurrent devices on Phase 1 before load shedding
    ReadOnlyVariable<int> LLM1;

    /// @brief Phase 2 load limit
    /// @details Range: 1-5 devices
    /// Maximum concurrent devices on Phase 2 before load shedding
    ReadOnlyVariable<int> LLM2;

    /// @brief Phase 3 load limit
    /// @details Range: 1-5 devices
    /// Maximum concurrent devices on Phase 3 before load shedding
    ReadOnlyVariable<int> LLM3;

    /// @brief Controller firmware version
       ReadOnlyVariable<String> SVER;

    /// @brief Spa model identifier
    ReadOnlyVariable<String> Model;

    /// @brief Primary serial number
    /// @details Manufacturing batch identifier
    ReadOnlyVariable<String> SerialNo1;

    /// @brief Secondary serial number
    /// @details Individual unit identifier
    ReadOnlyVariable<String> SerialNo2;

    /// @brief DIP Switch 1 - Circulation Pump Fitted
    /// @details true=Yes, false=No
    /// Is there a circulation pump installed
    ReadOnlyVariable<bool> D1;

    /// @brief DIP Switch 2 - Pump 1 type
    /// @details true=Dual Speed, false=Single Speed
    /// If false then assume Pump 2 fitted
    ReadOnlyVariable<bool> D2;

    /// @brief DIP Switch 3 -Pump 3 Type / Fitted
    /// @details true=Dual Speed (SV2/4)/Fitted (SV3), false=Single Speed (SV2/4)/Not Fitted (SV3)
    ReadOnlyVariable<bool> D3;

    /// @brief DIP Switch 4 - Pump 4 Fitted
    /// @details true=2/3 Phase, false=Single Phase
    ReadOnlyVariable<bool> D4;

    /// @brief DIP Switch 5 - Phase Selection
    /// @details true=Enabled, false=Disabled
    /// Enables dedicated circulation pump
    ReadOnlyVariable<bool> D5;

    /// @brief DIP Switch 6 - Multi-Phase Power
    /// @details true=3 phase, false=2 phase    
    ReadOnlyVariable<bool> D6;

    /// @brief Active pump status
    /// @details Shows current pump state and speed
    ReadOnlyVariable<String> Pump;

    /// @brief Load shed threshold
    /// @details Range: 1-4 active devices
    /// Number of active devices that triggers heater shutdown
    ReadOnlyVariable<int> LS;

    /// @brief HV?
    /// @details true=Over voltage detected
    // ReadOnlyVariable<bool> HV;

    /// @brief MR?
    // ReadOnlyVariable<int> MR;

    /// @brief Current operating status
    ReadOnlyVariable<String> Status;

    /// @brief Pump prime counter
    /// @details Number of prime cycles completed
    ReadOnlyVariable<int> PrimeCount;

    /// @brief Element current draw
    /// Heater element current consumption
    ReadOnlyVariable<int> EC;

    /// @brief Heat pump ambient temperature
    /// External temperature for heat pump operation
    ReadOnlyVariable<int> HAMB;

    /// @brief Heat pump condensing temperature
    /// Internal heat pump temperature
    ReadOnlyVariable<int> HCON;

    /// @brief HV_2?
    // ReadOnlyVariable<bool> HV_2;
#pragma endregion
#pragma region R4
    // R4
    /// @brief Operation mode
    ///
    /// One of NORM, ECON, AWAY, WEEK
    ReadOnlyVariable<String> Mode;
    /// @brief Service Timer 1 (wks) 0 = off
    ReadOnlyVariable<int> Ser1_Timer;
    /// @brief Service Timer 2 (wks) 0 = off
    ReadOnlyVariable<int> Ser2_Timer;
    /// @brief Service Timer 3 (wks) 0 = off
    ReadOnlyVariable<int> Ser3_Timer;
    /// @brief Heat mode
    ///
    /// 1 = Standby
    /// 2 = HeatMix
    ReadOnlyVariable<int> HeatMode;
    /// @brief Pump idle time (sec)
    ReadOnlyVariable<int> PumpIdleTimer;
    /// @brief Pump run time (sec)
    ReadOnlyVariable<int> PumpRunTimer;
    /// @brief Pool temperature adaptive hysteresis
    ReadOnlyVariable<int> AdtPoolHys;
    /// @brief  Heater temperature adaptive hysteresis
    ReadOnlyVariable<int> AdtHeaterHys;
    /// @brief Power consumtion * 10
    ReadOnlyVariable<int> Power; 
    ReadOnlyVariable<int> Energy_kWh;
    // (kWh)
    ReadOnlyVariable<int> Energy_Today;
    // (kWh)
    ReadOnlyVariable<int> Energy_Yesterday;
    // 0 = ok
    ReadOnlyVariable<int> ThermalCutOut;
    ReadOnlyVariable<int> Test_D1;
    ReadOnlyVariable<int> Test_D2;
    ReadOnlyVariable<int> Test_D3;
    ReadOnlyVariable<int> ElementHeatSourceOffset;
    ReadOnlyVariable<int> Frequency;
    ReadOnlyVariable<int> HPHeatSourceOffset_Heat;
    // 100 = 0!?
    ReadOnlyVariable<int> HPHeatSourceOffset_Cool;
    ReadOnlyVariable<int> HeatSourceOffTime;
    ReadOnlyVariable<int> Vari_Speed;
    ReadOnlyVariable<int> Vari_Percent;
    // 5 = Filt
    // 4 = Off
    /// @brief Varible speed mode
    ///
    /// 5 = Filtering, 4 = Off
    ReadOnlyVariable<int> Vari_Mode;
#pragma endregion
#pragma region R5
    // R5
    //  Unknown encoding - Attribute<int> TouchPad2;
    //  Unknown encoding - Attribute<int> TouchPad1;

    ReadOnlyVariable<int> RB_TP_Blower;

    /// @brief Auto enabled
    ///
    /// True when auto enabled
    ReadOnlyVariable<bool> RB_TP_Auto;
    /// @brief Heating running
    ///
    /// True when heating/cooling active
    ReadOnlyVariable<bool> RB_TP_Heater;
    /// @brief Cleaning (UV/Ozone running)
    ///
    /// True when Ozone/UV is cleaning spa.
    ReadOnlyVariable<bool> RB_TP_Ozone;
    /// @brief Sleeping
    ///
    /// True when spa is sleeping due to sleep timer
    ReadOnlyVariable<bool> RB_TP_Sleep;
    /// @brief Water temperature ('C)
    ReadOnlyVariable<int> WTMP;
    /// @brief Clean cycle running
    ///
    /// True when a clean cycle is running
    ReadOnlyVariable<bool> CleanCycle;
#pragma endregion
#pragma region R6
    // R6
    /// @brief Blower variable speed
    ///
    /// min 1, max 5
    ReadOnlyVariable<int> VARIValue;







    /// @brief Filter run time (in hours) per block
    ReadOnlyVariable<int> FiltSetHrs;
    /// @brief Filter block duration (hours)
    ReadOnlyVariable<int> FiltBlockHrs;



    // 1 = 12 hrs
    ReadOnlyVariable<int> L_24HOURS;
    /// @brief Power save level
    ///
    /// 0 = off, 1 = low, 2 = high
    ReadOnlyVariable<int> PSAV_LVL;
    /// @brief Peak power start time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    ReadOnlyVariable<int> PSAV_BGN;
    /// @brief Peak power end time
    ///
    /// Formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    ReadOnlyVariable<int> PSAV_END;

    /// @brief Default screen for control panels
    ///
    /// 0 = WTPM
    ReadOnlyVariable<int> DefaultScrn;
    /// @brief Time out duration (min)
    ///
    /// Time in min before pump and blower time out (min 10, max 30)
    ReadOnlyVariable<int> TOUT;
    ReadOnlyVariable<bool> VPMP;
    ReadOnlyVariable<bool> HIFI;
    /// @brief BRND
    ///
    /// 2 = VORT
    ReadOnlyVariable<int> BRND;
    /// @brief PRME
    ///
    /// 0 = 10secF
    ReadOnlyVariable<int> PRME;
    ReadOnlyVariable<int> ELMT;
    /// @brief TYPE
    ///
    /// 3 = SV3
    ReadOnlyVariable<int> TYPE;
    ReadOnlyVariable<int> GAS;
#pragma endregion
#pragma region R7
    // R7
    /// @brief Daily clean cycle start time
    ///
    /// Time with the formula h*256+m (ie: for 20:00, integer will be 20*256+0 = 5120; for 13:47, integer will be 13*256+47 = 3375)
    ReadOnlyVariable<int> WCLNTime;
    /// @brief Use 'F instead of 'C as the temp. UMO
    ReadOnlyVariable<bool> TemperatureUnits;
    ReadOnlyVariable<bool> OzoneOff;
    /// @brief Sanitiser 24 hrs
    ///
    /// True if sanitiser (ozone) power outlet on permanently, false if automatically controlled.
    /// See SV-Series-OEM-Install-Manual.pdf page 20.
    ReadOnlyVariable<bool> Ozone24;
    /// @brief Circulation pump 24hrs
    ///
    /// True if circulation pump is always on, false if automatically controlled.
    /// See SV-Series-OEM-Install-Manual.pdf page 20.
    ReadOnlyVariable<bool> Circ24;
    ReadOnlyVariable<bool> CJET;
    /// @brief Variable heat element operation
    ///
    /// If true allows variable power to be fed to the heating element.
    /// See SV-Series-OEM-Install-Manual.pdf page 19.
    ReadOnlyVariable<bool> VELE;

    /// TODO #2 - Not Implemented
    /// @brief Date of comissioning
    /// ReadOnlyVariable<time_t> ComissionDate;

    /// @brief Highest voltage ever recorded (V)
    ReadOnlyVariable<int> V_Max;
    /// @brief Lowest voltage ever recorded (V)
    ReadOnlyVariable<int> V_Min;
    /// @brief Highest voltage in past 24 hrs (V)
    ReadOnlyVariable<int> V_Max_24;
    /// @brief Lowest voltage in past 24 hrs (V)
    ReadOnlyVariable<int> V_Min_24;
    ReadOnlyVariable<int> CurrentZero;
    ReadOnlyVariable<int> CurrentAdjust;
    ReadOnlyVariable<int> VoltageAdjust;
    ReadOnlyVariable<int> Ser1;
    ReadOnlyVariable<int> Ser2;
    ReadOnlyVariable<int> Ser3;
    /// @brief Variable heat element max power (A)
    ///
    /// Maximum current that the heat element is allowed to draw (between 3 and 25A)
    /// See SV-Series-OEM-Install-Manual.pdf page 19.
    ReadOnlyVariable<int> VMAX;
    /// @brief Adaptive Hysteresis
    ///
    /// Maximum adaptive hysteresis value (0=disabled).  See SV-Series-OEM-Install-Manual.pdf page 20.
    ReadOnlyVariable<int> AHYS;
    /// @brief HUSE
    ///
    /// 1 = Off
    ReadOnlyVariable<bool> HUSE;
    /// @brief Heat pump active whilst spa is in use
    ///
    /// If false then when spa is in use then heat pump will not run to reduce noise levels
    /// See SV-Series-OEM-Install-Manual.pdf page 19.
    ReadOnlyVariable<bool> HELE;

    /// @brief Varible pump minimum speed setting
    ///
    /// Min 20%, Max 100%
    /// See SV-Series-OEM-Install-Manual.pdf page 21.
    ReadOnlyVariable<int> PMIN;
    /// @brief Variable pump filtration speed setting
    ///
    /// Min 20%, Max 100%
    /// See SV-Series-OEM-Install-Manual.pdf page 21.
    ReadOnlyVariable<int> PFLT;
    /// @brief Varible pump heater speed setting
    ///
    /// Min 20%, Max 100%
    ReadOnlyVariable<int> PHTR;
    /// @brief Varible pump maximum speed setting
    ///
    /// Maximum speed the varible pump will run at.
    /// Min 20%, Max 100%
    ReadOnlyVariable<int> PMAX;
#pragma endregion
#pragma region R9
    // R9
    /// @brief Fault runtime occurance (hrs)
    ReadOnlyVariable<int> F1_HR;
    /// @brief Fault time of day occurance
    ReadOnlyVariable<int> F1_Time;
    /// @brief Fault error codes
    ///
    /// 6 = ER612VOverload - High current detected on 12v line
    ReadOnlyVariable<int> F1_ER;
    /// @brief Supply current draw at time of error (A)
    ReadOnlyVariable<int> F1_I;
    /// @brief Supply voltage at time of error (V)
    ReadOnlyVariable<int> F1_V;
    /// @brief Pool temperature at time of error ('C)
    ReadOnlyVariable<int> F1_PT;
    /// @brief Heater temperature at time of error ('C)
    ReadOnlyVariable<int> F1_HT;
    ReadOnlyVariable<int> F1_CT;
    ReadOnlyVariable<int> F1_PU;
    ReadOnlyVariable<bool> F1_VE;
    /// @brief Heater setpoint at time of error ('C)
    ReadOnlyVariable<int> F1_ST;
#pragma endregion
#pragma region RA
    // RA
    /// @brief Fault runtime occurance (hrs)
    ReadOnlyVariable<int> F2_HR;
    /// @brief Fault time of day occurance
    ReadOnlyVariable<int> F2_Time;
    /// @brief Fault error codes
    ///
    /// 6 = ER612VOverload - High current detected on 12v line
    ReadOnlyVariable<int> F2_ER;
    /// @brief Supply current draw at time of error (A)
    ReadOnlyVariable<int> F2_I;
    /// @brief Supply voltage at time of error (V)
    ReadOnlyVariable<int> F2_V;
    /// @brief Pool temperature at time of error ('C)
    ReadOnlyVariable<int> F2_PT;
    /// @brief Heater temperature at time of error ('C)
    ReadOnlyVariable<int> F2_HT;
    ReadOnlyVariable<int> F2_CT;
    ReadOnlyVariable<int> F2_PU;
    ReadOnlyVariable<bool> F2_VE;
    /// @brief Heater setpoint at time of error ('C)
    ReadOnlyVariable<int> F2_ST;
#pragma endregion
#pragma region RB
    // RB
    /// @brief Fault runtime occurance (hrs)
    ReadOnlyVariable<int> F3_HR;
    /// @brief Fault time of day occurance
    ReadOnlyVariable<int> F3_Time;
    /// @brief Fault error codes
    ///
    /// 6 = ER612VOverload - High current detected on 12v line
    ReadOnlyVariable<int> F3_ER;
    /// @brief Supply current draw at time of error (A)
    ReadOnlyVariable<int> F3_I;
    /// @brief Supply voltage at time of error (V)
    ReadOnlyVariable<int> F3_V;
    /// @brief Pool temperature at time of error ('C)
    ReadOnlyVariable<int> F3_PT;
    /// @brief Heater temperature at time of error ('C)
    ReadOnlyVariable<int> F3_HT;
    ReadOnlyVariable<int> F3_CT;
    ReadOnlyVariable<int> F3_PU;
    ReadOnlyVariable<bool> F3_VE;
    /// @brief Heater setpoint at time of error ('C)
    ReadOnlyVariable<int> F3_ST;
#pragma endregion
#pragma region RC
    // RC
    //  Encoding of the RC registers is not obvious
    // Attribute<bool> Outlet_Heater;
    // Attribute<bool> Outlet_Circ;
    // Attribute<bool> Outlet_Sanitise;
    // Attribute<bool> Outlet_Pump1;
    // Attribute<bool> Outlet_Pump2;
    // Attribute<bool> Outlet_Pump4;
    // Attribute<bool> Outlet_Pump5;
    /// @brief Blower status
    ///
    /// 0 = variable mode, 1 = ramp mode, 2 = off
    ReadOnlyVariable<int> Outlet_Blower;
#pragma endregion
#pragma region RE
    // RE
    /// @brief Heatpump installed / interface version
    ReadOnlyVariable<int> HP_Present;
    // Encoding of these registers is not clear
    // Attribute<bool> HP_FlowSwitch;
    // Attribute<bool> HP_HighSwitch;
    // Attribute<bool> HP_LowSwitch;
    // Attribute<bool> HP_CompCutOut;
    // Attribute<bool> HP_ExCutOut;
    // Attribute<bool> HP_D1;
    // Attribute<bool> HP_D2;
    // Attribute<bool> HP_D3;
    /// @brief Ambient air temperature ('C)
    ReadOnlyVariable<int> HP_Ambient;
    /// @brief Compressor temperature ('C)
    ReadOnlyVariable<int> HP_Condensor;
    /// @brief Compressor running
    ReadOnlyVariable<bool> HP_Compressor_State;
    /// @brief Fan running
    ReadOnlyVariable<bool> HP_Fan_State;
    ReadOnlyVariable<bool> HP_4W_Valve;
    ReadOnlyVariable<bool> HP_Heater_State;
    /// @brief Heatpump state
    ///
    /// 0 = Standby
    ReadOnlyVariable<int> HP_State;
    /// @brief Heatpump mode
    ///
    /// 1 = Heat
    ReadOnlyVariable<int> HP_Mode;
    ReadOnlyVariable<int> HP_Defrost_Timer;
    ReadOnlyVariable<int> HP_Comp_Run_Timer;
    ReadOnlyVariable<int> HP_Low_Temp_Timer;
    ReadOnlyVariable<int> HP_Heat_Accum_Timer;
    ReadOnlyVariable<int> HP_Sequence_Timer;
    ReadOnlyVariable<int> HP_Warning;
    ReadOnlyVariable<int> FrezTmr;
    ReadOnlyVariable<int> DBGN;
    ReadOnlyVariable<int> DEND;
    ReadOnlyVariable<int> DCMP;
    ReadOnlyVariable<int> DMAX;
    ReadOnlyVariable<int> DELE;
    ReadOnlyVariable<int> DPMP;
// Attribute<int> CMAX;
// Attribute<int> HP_Compressor;
// Attribute<int> HP_Pump_State;
// Attribute<int> HP_Status;
#pragma endregion
#pragma region RG
    /// @brief Pump 1 install state
    ///
    /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
    /// means it is), the second part (1- above) indicates it's speed type. The third
    /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
    ReadOnlyVariable<String> Pump1InstallState;
    /// @brief Pump 2 install state
    ///
    /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
    /// means it is), the second part (1- above) indicates it's speed type. The third
    /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
    ReadOnlyVariable<String> Pump2InstallState;
    /// @brief Pump 3 install state
    ///
    /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
    /// means it is), the second part (1- above) indicates it's speed type. The third
    /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
    ReadOnlyVariable<String> Pump3InstallState;
    /// @brief Pump 4 install state
    ///
    /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
    /// means it is), the second part (1- above) indicates it's speed type. The third
    /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
    ReadOnlyVariable<String> Pump4InstallState;
    /// @brief Pump 5 install state
    ///
    /// (eg 1-1-014) First part (1- or 0-) indicates whether the pump is installed/fitted. If so (1-
    /// means it is), the second part (1- above) indicates it's speed type. The third
    /// part (014 above) represents it's possible states (0 OFF, 1 ON, 4 AUTO)
    ReadOnlyVariable<String> Pump5InstallState;
    /// @brief Pump 1 is in safe state to start
    ReadOnlyVariable<bool> Pump1OkToRun;
    /// @brief Pump 2 is in safe state to start
    ReadOnlyVariable<bool> Pump2OkToRun;
    /// @brief Pump 3 is in safe state to start
    ReadOnlyVariable<bool> Pump3OkToRun;
    /// @brief Pump 4 is in safe state to start
    ReadOnlyVariable<bool> Pump4OkToRun;
    /// @brief Pump 5 is in safe state to start
    ReadOnlyVariable<bool> Pump5OkToRun;
    /// @brief Lock mode
    ///
    /// 0 = keypad unlocked, 1 = partial lock, 2 = full lock
    ReadOnlyVariable<int> LockMode;

#pragma endregion
#pragma endregion

};
