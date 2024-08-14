#include "SpaNetInterface.h"

#define BAUD_RATE 38400

#if defined(ESP8266)
    #define RX_PIN 13 //goes to rx on spanet pin5
    #define TX_PIN 15 //goes to tx on spanet pin6
    HardwareSerial spaSerial = Serial;
#elif defined(ESP32)
    #define RX_PIN 16 //goes to rx on spanet pin5
    #define TX_PIN 17 //goes to tx on spanet pin6
    HardwareSerial spaSerial = Serial2;
#endif

SpaNetInterface::SpaNetInterface(Stream &p) : port(p) {
}

SpaNetInterface::SpaNetInterface() : port(spaSerial) {
    #if defined(ESP8266)
        spaSerial.setRxBufferSize(1024);  //required for unit testing
        spaSerial.begin(BAUD_RATE);
        spaSerial.pins(TX_PIN, RX_PIN);
    #elif defined(ESP32)
        spaSerial.setRxBufferSize(1024);  //required for unit testing
        spaSerial.setTxBufferSize(1024);  //required for unit testing
        spaSerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
    #endif
    spaSerial.setTimeout(250);
}

SpaNetInterface::~SpaNetInterface() {}




void SpaNetInterface::flushSerialReadBuffer() {
    int x = 0;

    debugD("Flushing serial stream - %i bytes in the buffer", port.available());
    while (port.available() > 0 && x++<5120) { 
        debugV("%i,",port.read());
    }
    debugD("Flushed serial stream - %i bytes in the buffer", port.available());
}


void SpaNetInterface::sendCommand(String cmd) {

    flushSerialReadBuffer();

    debugD("Sending - %s",cmd.c_str());
    port.print('\n');
    port.flush();
    delay(50); // **TODO** is this needed?
    port.printf("%s\n", cmd);
    port.flush();

    long timeout = millis() + 1000; // wait up to 1 sec for a response

    debugD("Start waiting for a response");
    while (port.available()==0 and millis()<timeout) {}
    debugD("Finish waiting");

    _resultRegistersDirty = true; // we're trying to write to the registers so we can assume that they will now be dirty
}

String SpaNetInterface::sendCommandReturnResult(String cmd) {
    sendCommand(cmd);
    String result = port.readStringUntil('\n');
    port.read(); // get rid of the trailing LF char
    debugV("Read - %s",result.c_str());
    return result;
}

bool SpaNetInterface::setRB_TP_Pump1(int mode){
    debugD("setRB_TP_Pump1 - %i",mode);
    String result = sendCommandReturnResult("S22:"+String(mode));
    if (result == "S22-OK") {
        update_RB_TP_Pump1(String(mode));
        return true;
    }
    return false;
}

bool SpaNetInterface::setRB_TP_Pump2(int mode){
    debugD("setRB_TP_Pump2 - %i",mode);
    String result = sendCommandReturnResult("S23:"+String(mode));
    if (result == "S23-OK") {
        update_RB_TP_Pump2(String(mode));
        return true;
    }
    return false;
}

bool SpaNetInterface::setRB_TP_Pump3(int mode){
    debugD("setRB_TP_Pump3 - %i",mode);
    String result = sendCommandReturnResult("S24:"+String(mode));
    if (result == "S24-OK") {
        update_RB_TP_Pump3(String(mode));
        return true;
    }
    return false;
}

bool SpaNetInterface::setRB_TP_Pump4(int mode){
    debugD("setRB_TP_Pump4 - %i",mode);
    String result = sendCommandReturnResult("S25:"+String(mode));
    if (result == "S25-OK") {
        update_RB_TP_Pump4(String(mode));
        return true;
    }
    return false;
}

bool SpaNetInterface::setRB_TP_Pump5(int mode){
    debugD("setRB_TP_Pump5 - %i",mode);
    String result = sendCommandReturnResult("S26:"+String(mode));
    if (result == "S26-OK") {
        update_RB_TP_Pump5(String(mode));
        return true;
    }
    return false;
}

bool SpaNetInterface::setHELE(int mode){
    debugD("setHELE - %i", mode);
    String result = sendCommandReturnResult("W98:"+String(mode));
    if (result == String(mode)) {
        update_HELE(String(mode));
        return true;
    }
    return false;
}


/// @brief Set the water temperature set point * 10 (380 = 38.0)
/// @param temp 
/// @return 
bool SpaNetInterface::setSTMP(int temp){
    debugD("setSTMP - %s", String(temp));
    String result = sendCommandReturnResult("W40:" + String(temp));
    if (String(temp).compareTo(result)) {
        update_STMP(result);
        return true;
    }
    return false;
}

bool SpaNetInterface::setHPMP(int mode){
    debugD("setHPMP - %i", mode);
    String result = sendCommandReturnResult(String("W99:")+mode);
    if (result.toInt() == mode) {
        update_HPMP(result);
        return true;
    }
    return false;
}

bool SpaNetInterface::setHPMP(String mode){
    debugD("setHPMP - %s", mode.c_str());
    for (int x=0; x<HPMPStrings.size(); x++) {
        if (HPMPStrings[x] == mode) {
            return setHPMP(x);
        }
    }
    return false;
}

bool SpaNetInterface::readStatus() {

    // We could just do a port.readString but this will always impose a
    // 250ms (or whatever the timeout is) delay penality.  This in turn,
    // along with the other unavoidable delays can cause the status of
    // properties to bounce in certain UI's (apple devices, home assistant, etc)

    debugD("Reading registers -");

    int field = 0;
    validStatusResponse = false;
    String statusResponseTmp = "";

    while (field < 289)
    {
        statusResponseRaw[field] = port.readStringUntil(',');
        debugV("(%i,%s)",field,statusResponseRaw[field]);

        statusResponseTmp = statusResponseTmp + statusResponseRaw[field]+",";

        if (statusResponseRaw[field].isEmpty()) { // If we get a empty field then we've had a bad read.
            debugE("Throwing exception - null string");
            return false;
        }

        field++;
    }

    //Flush the remaining data from the buffer as the last field is meaningless
    flushSerialReadBuffer();

    statusResponse.update_Value(statusResponseTmp);

    if (field != 289) {
        debugE("Throwing exception - %i fields read expecting 289",field);
        return false;
    }

    updateMeasures();
    _resultRegistersDirty = false;
    validStatusResponse = true;

    debugD("Reading registers - finish");
    return true;
}

bool SpaNetInterface::isInitialised() { 
    return _initialised; 
}


void SpaNetInterface::updateStatus() {

    flushSerialReadBuffer();

    debugD("Update status called");
    sendCommand("RF");

    _nextUpdateDue = millis() + FAILEDREADFREQUENCY;    
    if (readStatus()) {
        debugD("readStatus returned true");
        _nextUpdateDue = millis() + UPDATEFREQUENCY;
        _initialised = true;
        if (updateCallback != nullptr) { updateCallback(); }
    }
}


void SpaNetInterface::loop(){
    if ( _lastWaitMessage + 1000 < millis()) {
        debugD("Waiting...");
        _lastWaitMessage = millis();
    }

    if (_resultRegistersDirty) {
        _nextUpdateDue = millis() + 200;  // if we need to read the registers, pause a bit to see if there are more commands coming.
        _resultRegistersDirty = false;
    }

    if (millis()>_nextUpdateDue) {
        updateStatus();    
    }
}


void SpaNetInterface::setUpdateCallback(void (*f)()) {
    updateCallback = f;
}


void SpaNetInterface::clearUpdateCallback() {
    updateCallback = nullptr;
}


void SpaNetInterface::updateMeasures() {
    #pragma region R2
    update_MainsCurrent(statusResponseRaw[2]);
    update_MainsVoltage(statusResponseRaw[3]);
    update_CaseTemperature(statusResponseRaw[4]);
    update_PortCurrent(statusResponseRaw[5]);
    // Not implemented - update_SpaTime(statusResponseRaw[12]+"-"+statusResponseRaw[11]+"-"+statusResponseRaw[10]+" "+statusResponseRaw[9]+":"+statusResponseRaw[8]+":"+statusResponseRaw[7]);
    update_HeaterTemperature(statusResponseRaw[13]);
    update_PoolTemperature(statusResponseRaw[14]);
    update_WaterPresent(statusResponseRaw[15]);
    update_AwakeMinutesRemaining(statusResponseRaw[17]);
    update_FiltPumpRunTimeTotal(statusResponseRaw[18]);
    update_FiltPumpReqMins(statusResponseRaw[19]);
    update_LoadTimeOut(statusResponseRaw[20]);
    update_HourMeter(statusResponseRaw[21]);
    update_Relay1(statusResponseRaw[22]);
    update_Relay2(statusResponseRaw[23]);
    update_Relay3(statusResponseRaw[24]);
    update_Relay4(statusResponseRaw[25]);
    update_Relay5(statusResponseRaw[26]);
    update_Relay6(statusResponseRaw[27]);
    update_Relay7(statusResponseRaw[28]);
    update_Relay8(statusResponseRaw[29]);
    update_Relay9(statusResponseRaw[30]); 
    #pragma endregion
    #pragma region R3
    update_CLMT(statusResponseRaw[34]);
    update_PHSE(statusResponseRaw[35]);
    update_LLM1(statusResponseRaw[36]);
    update_LLM2(statusResponseRaw[37]);
    update_LLM3(statusResponseRaw[38]);
    update_SVER(statusResponseRaw[39]);
    update_Model(statusResponseRaw[40]); 
    update_SerialNo1(statusResponseRaw[41]);
    update_SerialNo2(statusResponseRaw[42]); 
    update_D1(statusResponseRaw[43]);
    update_D2(statusResponseRaw[44]);
    update_D3(statusResponseRaw[45]);
    update_D4(statusResponseRaw[46]);
    update_D5(statusResponseRaw[47]);
    update_D6(statusResponseRaw[48]);
    update_Pump(statusResponseRaw[49]);
    update_LS(statusResponseRaw[50]);
    update_HV(statusResponseRaw[51]);
    update_SnpMR(statusResponseRaw[52]);
    update_Status(statusResponseRaw[53]);
    update_PrimeCount(statusResponseRaw[54]);
    update_EC(statusResponseRaw[55]);
    update_HAMB(statusResponseRaw[56]);
    update_HCON(statusResponseRaw[57]);
    // update_HV_2(statusResponseRaw[58]);
    #pragma endregion
    #pragma region R4
    update_Mode(statusResponseRaw[63]);
    update_Ser1_Timer(statusResponseRaw[64]);
    update_Ser2_Timer(statusResponseRaw[65]);
    update_Ser3_Timer(statusResponseRaw[66]);
    update_HeatMode(statusResponseRaw[67]);
    update_PumpIdleTimer(statusResponseRaw[68]);
    update_PumpRunTimer(statusResponseRaw[69]);
    update_AdtPoolHys(statusResponseRaw[70]);
    update_AdtHeaterHys(statusResponseRaw[71]);
    update_Power(statusResponseRaw[72]);
    update_Power_kWh(statusResponseRaw[73]);
    update_Power_Today(statusResponseRaw[74]);
    update_Power_Yesterday(statusResponseRaw[75]);
    update_ThermalCutOut(statusResponseRaw[76]);
    update_Test_D1(statusResponseRaw[77]);
    update_Test_D2(statusResponseRaw[78]);
    update_Test_D3(statusResponseRaw[79]);
    update_ElementHeatSourceOffset(statusResponseRaw[80]);
    update_Frequency(statusResponseRaw[81]);
    update_HPHeatSourceOffset_Heat(statusResponseRaw[82]);
    update_HPHeatSourceOffset_Cool(statusResponseRaw[83]);
    update_HeatSourceOffTime(statusResponseRaw[84]);
    update_Vari_Speed(statusResponseRaw[86]);
    update_Vari_Percent(statusResponseRaw[87]);
    update_Vari_Mode(statusResponseRaw[85]);
    #pragma endregion
    #pragma region R5
    //R5
    // Unknown encoding - TouchPad2.updateValue();
    // Unknown encoding - TouchPad1.updateValue();
    update_RB_TP_Pump1(statusResponseRaw[110]);
    update_RB_TP_Pump2(statusResponseRaw[111]);
    update_RB_TP_Pump3(statusResponseRaw[112]);
    update_RB_TP_Pump4(statusResponseRaw[113]);
    update_RB_TP_Pump5(statusResponseRaw[114]);
    //RB_TP_Blower.updateValue(statusResponseRaw[101]);
    //RB_TP_Light.updateValue(statusResponseRaw[102]);
    update_RB_TP_Auto(statusResponseRaw[105]);
    update_RB_TP_Heater(statusResponseRaw[104]);
    update_RB_TP_Ozone(statusResponseRaw[103]);
    update_RB_TP_Sleep(statusResponseRaw[102]);
    update_WTMP(statusResponseRaw[107]);
    update_CleanCycle(statusResponseRaw[108]);
    #pragma endregion
    #pragma region R6
    update_VARIValue(statusResponseRaw[121]);
    update_LBRTValue(statusResponseRaw[122]);
    update_CurrClr(statusResponseRaw[123]);
    update_ColorMode(statusResponseRaw[124]);
    update_LSPDValue(statusResponseRaw[125]);
    update_FiltSetHrs(statusResponseRaw[126]);
    update_FiltBlockHrs(statusResponseRaw[127]);
    update_STMP(statusResponseRaw[128]);
    update_L_24HOURS(statusResponseRaw[129]);
    update_PSAV_LVL(statusResponseRaw[130]);
    update_PSAV_BGN(statusResponseRaw[131]);
    update_PSAV_END(statusResponseRaw[132]);
    update_L_1SNZ_DAY(statusResponseRaw[133]);
    update_L_2SNZ_DAY(statusResponseRaw[134]);
    update_L_1SNZ_BGN(statusResponseRaw[135]);
    update_L_2SNZ_BGN(statusResponseRaw[136]);
    update_L_1SNZ_END(statusResponseRaw[137]);
    update_L_2SNZ_END(statusResponseRaw[138]);
    update_DefaultScrn(statusResponseRaw[139]);
    update_TOUT(statusResponseRaw[140]);
    update_VPMP(statusResponseRaw[141]);
    update_HIFI(statusResponseRaw[142]);
    update_BRND(statusResponseRaw[143]);
    update_PRME(statusResponseRaw[144]);
    update_ELMT(statusResponseRaw[145]);
    update_TYPE(statusResponseRaw[146]);
    update_GAS(statusResponseRaw[147]);
    #pragma endregion
    #pragma region R7
    update_WCLNTime(statusResponseRaw[151]);
    // The following 2 may be reversed
    update_TemperatureUnits(statusResponseRaw[153]);
    update_OzoneOff(statusResponseRaw[152]);
    update_Ozone24(statusResponseRaw[154]);
    update_Circ24(statusResponseRaw[156]);
    update_CJET(statusResponseRaw[155]);
    // 0 = off, 1 = step, 2 = variable
    update_VELE(statusResponseRaw[157]);
    //update_StartDD(statusResponseRaw[158]);
    //update_StartMM(statusResponseRaw[159]);
    //update_StartYY(statusResponseRaw[160]);
    update_V_Max(statusResponseRaw[161]);
    update_V_Min(statusResponseRaw[162]);
    update_V_Max_24(statusResponseRaw[163]);
    update_V_Min_24(statusResponseRaw[164]);
    update_CurrentZero(statusResponseRaw[165]);
    update_CurrentAdjust(statusResponseRaw[166]);
    update_VoltageAdjust(statusResponseRaw[167]);
    // 168 is unknown
    update_Ser1(statusResponseRaw[169]);
    update_Ser2(statusResponseRaw[170]);
    update_Ser3(statusResponseRaw[171]);
    update_VMAX(statusResponseRaw[172]);
    update_AHYS(statusResponseRaw[173]);
    update_HUSE(statusResponseRaw[174]);
    update_HELE(statusResponseRaw[175]);
    update_HPMP(statusResponseRaw[176]);
    update_PMIN(statusResponseRaw[177]);
    update_PFLT(statusResponseRaw[178]);
    update_PHTR(statusResponseRaw[179]);
    update_PMAX(statusResponseRaw[180]);
    #pragma endregion
    #pragma region R9
    update_F1_HR(statusResponseRaw[185]);
    update_F1_Time(statusResponseRaw[186]);
    update_F1_ER(statusResponseRaw[187]);
    update_F1_I(statusResponseRaw[188]);
    update_F1_V(statusResponseRaw[189]);
    update_F1_PT(statusResponseRaw[190]);
    update_F1_HT(statusResponseRaw[191]);
    update_F1_CT(statusResponseRaw[192]);
    update_F1_PU(statusResponseRaw[193]);
    update_F1_VE(statusResponseRaw[194]);
    update_F1_ST(statusResponseRaw[195]);
    #pragma endregion
    #pragma region RA
    update_F2_HR(statusResponseRaw[199]);
    update_F2_Time(statusResponseRaw[200]);
    update_F2_ER(statusResponseRaw[201]);
    update_F2_I(statusResponseRaw[202]);
    update_F2_V(statusResponseRaw[203]);
    update_F2_PT(statusResponseRaw[204]);
    update_F2_HT(statusResponseRaw[205]);
    update_F2_CT(statusResponseRaw[206]);
    update_F2_PU(statusResponseRaw[207]);
    update_F2_VE(statusResponseRaw[208]);
    update_F2_ST(statusResponseRaw[209]);
    #pragma endregion
    #pragma region RB
    update_F3_HR(statusResponseRaw[213]);
    update_F3_Time(statusResponseRaw[214]);
    update_F3_ER(statusResponseRaw[215]);
    update_F3_I(statusResponseRaw[216]);
    update_F3_V(statusResponseRaw[217]);
    update_F3_PT(statusResponseRaw[218]);
    update_F3_HT(statusResponseRaw[219]);
    update_F3_CT(statusResponseRaw[220]);
    update_F3_PU(statusResponseRaw[221]);
    update_F3_VE(statusResponseRaw[222]);
    update_F3_ST(statusResponseRaw[223]);
    #pragma endregion
    #pragma region RC
    //Outlet_Heater.updateValue(statusResponseRaw[]);
    //Outlet_Circ.updateValue(statusResponseRaw[]);
    //Outlet_Sanitise.updateValue(statusResponseRaw[]);
    //Outlet_Pump1.updateValue(statusResponseRaw[]);
    //Outlet_Pump2.updateValue(statusResponseRaw[]);
    //Outlet_Pump4.updateValue(statusResponseRaw[]);
    //Outlet_Pump5.updateValue(statusResponseRaw[]);
    update_Outlet_Blower(statusResponseRaw[235]);
    #pragma endregion
    #pragma region RE
    update_HP_Present(statusResponseRaw[242]);
    //HP_FlowSwitch.updateValue(statusResponseRaw[]);
    //HP_HighSwitch.updateValue(statusResponseRaw[]);
    //HP_LowSwitch.updateValue(statusResponseRaw[]);
    //HP_CompCutOut.updateValue(statusResponseRaw[]);
    //HP_ExCutOut.updateValue(statusResponseRaw[]);
    //HP_D1.updateValue(statusResponseRaw[]);
    //HP_D2.updateValue(statusResponseRaw[]);
    //HP_D3.updateValue(statusResponseRaw[]);
    update_HP_Ambient(statusResponseRaw[251]);
    update_HP_Condensor(statusResponseRaw[252]);
    update_HP_Compressor_State(statusResponseRaw[253]);
    update_HP_Fan_State(statusResponseRaw[254]);
    update_HP_4W_Valve(statusResponseRaw[255]);
    update_HP_Heater_State(statusResponseRaw[256]);
    update_HP_State(statusResponseRaw[257]);
    update_HP_Mode(statusResponseRaw[258]);
    update_HP_Defrost_Timer(statusResponseRaw[259]);
    update_HP_Comp_Run_Timer(statusResponseRaw[260]);
    update_HP_Low_Temp_Timer(statusResponseRaw[261]);
    update_HP_Heat_Accum_Timer(statusResponseRaw[262]);
    update_HP_Sequence_Timer(statusResponseRaw[263]);
    update_HP_Warning(statusResponseRaw[264]);
    update_FrezTmr(statusResponseRaw[265]);
    update_DBGN(statusResponseRaw[266]);
    update_DEND(statusResponseRaw[267]);
    update_DCMP(statusResponseRaw[268]);
    update_DMAX(statusResponseRaw[269]);
    update_DELE(statusResponseRaw[270]);
    update_DPMP(statusResponseRaw[271]);
    //CMAX.updateValue(statusResponseRaw[]);
    //HP_Compressor.updateValue(statusResponseRaw[]);
    //HP_Pump_State.updateValue(statusResponseRaw[]);
    //HP_Status.updateValue(statusResponseRaw[]);
    #pragma endregion
    #pragma region RG
    update_Pump1InstallState(statusResponseRaw[280]);
    update_Pump2InstallState(statusResponseRaw[281]);
    update_Pump3InstallState(statusResponseRaw[282]);
    update_Pump4InstallState(statusResponseRaw[283]);
    update_Pump5InstallState(statusResponseRaw[284]);
    update_Pump1OkToRun(statusResponseRaw[274]);
    update_Pump2OkToRun(statusResponseRaw[275]);
    update_Pump3OkToRun(statusResponseRaw[276]);
    update_Pump4OkToRun(statusResponseRaw[277]);
    update_Pump5OkToRun(statusResponseRaw[278]);
    update_LockMode(statusResponseRaw[285]);
    #pragma endregion

};