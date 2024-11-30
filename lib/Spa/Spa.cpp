#include "Spa.h"

Spa::Spa() : port(SPA_SERIAL) {
    SPA_SERIAL.setRxBufferSize(1024);  //required for unit testing
    SPA_SERIAL.setTxBufferSize(1024);  //required for unit testing
    SPA_SERIAL.begin(38400, SERIAL_8N1, RX_PIN, TX_PIN);
    SPA_SERIAL.setTimeout(250);

    STMP.setBeforeUpdate(this, &Spa::setSTMP);

    RB_TP_Pump1.setBeforeUpdate(this, &Spa::setRB_TP_Pump1);
    RB_TP_Pump2.setBeforeUpdate(this, &Spa::setRB_TP_Pump2);
    RB_TP_Pump3.setBeforeUpdate(this, &Spa::setRB_TP_Pump3);
    RB_TP_Pump4.setBeforeUpdate(this, &Spa::setRB_TP_Pump4);
    RB_TP_Pump5.setBeforeUpdate(this, &Spa::setRB_TP_Pump5);
    HPMP.setBeforeUpdate(this, &Spa::setHPMP);
    L_1SNZ_DAY.setBeforeUpdate(this, &Spa::setL_1SNZ_DAY);
    L_2SNZ_DAY.setBeforeUpdate(this, &Spa::setL_2SNZ_DAY);
    L_1SNZ_BGN.setBeforeUpdate(this, &Spa::setL_1SNZ_BGN);
    L_1SNZ_END.setBeforeUpdate(this, &Spa::setL_1SNZ_END);
    L_2SNZ_BGN.setBeforeUpdate(this, &Spa::setL_2SNZ_BGN);
    L_2SNZ_END.setBeforeUpdate(this, &Spa::setL_2SNZ_END);

    L_1SNZ_BGN.setStringConverter(this, &Spa::timeStringToUint16);
    L_1SNZ_END.setStringConverter(this, &Spa::timeStringToUint16);
    L_2SNZ_BGN.setStringConverter(this, &Spa::timeStringToUint16);
    L_2SNZ_END.setStringConverter(this, &Spa::timeStringToUint16);

    ColorMode.setBeforeUpdate(this, &Spa::setColorMode);
    LBRTValue.setBeforeUpdate(this, &Spa::setLBRTValue);
    LSPDValue.setBeforeUpdate(this, &Spa::setLSPDValue);
    CurrClr.setBeforeUpdate(this, &Spa::setCurrClr);


}

void Spa::sendCommand(String cmd) {

    clearSerialReadBuffer();

    debugD("Sending - %s",cmd.c_str());
    port.print('\n');
    port.flush();
    delay(50); // **TODO** is this needed?
    port.printf("%s\n", cmd.c_str());
    port.flush();

    ulong timeout = millis() + 1000; // wait up to 1 sec for a response

    debugD("Start waiting for a response");
    while (port.available()==0 and millis()<timeout) {}
    debugD("Finish waiting");

    _resultRegistersDirty = true; // we're trying to write to the registers so we can assume that they will now be dirty
}

String Spa::sendCommandReturnResult(String cmd) {
    sendCommand(cmd);
    String result = port.readStringUntil('\r');
    port.read(); // get rid of the trailing LF char
    debugV("Read - %s",result.c_str());
    return result;
}

bool Spa::sendCommandCheckResult(String cmd, String expected){
    String result = sendCommandReturnResult(cmd);
    bool outcome = result == expected;
    if (!outcome) debugW("Sent comment %s, expected %s, got %s",cmd.c_str(),expected.c_str(),result.c_str());
    return outcome;
}

void Spa::clearSerialReadBuffer() {
    int bytesFlushed = 0;

    debugD("Flushing serial stream - %i bytes in the buffer", port.available());
    while (port.available() > 0 && bytesFlushed++<5120) { 
        int byteRead = port.read();
        debugV("%i,",byteRead);
    }
    debugD("Flushed serial stream - %i bytes in the buffer", port.available());
}

uint16_t Spa::timeStringToUint16(const String& time) {
    int colonIndex = time.indexOf(':');
    if (colonIndex == -1) {
        debugW("Invalid time string - %s", time.c_str());
        return 0;
    }
    int hours = time.substring(0, colonIndex).toInt();
    int minutes = time.substring(colonIndex + 1).toInt();
    return hours * 256 + minutes;
}

bool Spa::setSTMP(uint16_t temp) {
    debugD("setSTMP - %i", temp);
    temp = temp - temp % 2; // round to nearest 0.2 degrees
    if ((temp >= 160) && (temp <= 410)) {
        String stemp = String(temp);
        return sendCommandCheckResult("W40:" + stemp, stemp);
    }
    debugW("setSTMP - %i is out of range", temp);
    return false;
}

bool Spa::setColorMode(byte mode){
    debugD("setColorMode - %i", mode);
    if ((mode >= 0) && (mode <= 4)) {
        String smode = String(mode);
        return sendCommandCheckResult("S07:"+smode,smode);
    }
    debugW("setColorMode - %i is out of range", mode);
    return false;
}

bool Spa::setRB_TP_Pump1(byte mode){
    debugD("setRB_TP_Pump1 - %i",mode);
    if ((mode >= 0) && (mode <= 4)) {
        String smode = String(mode);
        return sendCommandCheckResult("S21:"+smode,smode);
    }
    debugW("setRB_TP_Pump1 - %i is out of range", mode);
    return false;
}

bool Spa::setRB_TP_Pump2(byte mode){
    debugD("setRB_TP_Pump2 - %i",mode);
    if ((mode >= 0) && (mode <= 4)) {
        String smode = String(mode);
        return sendCommandCheckResult("S22:"+smode,smode);
    }
    debugW("setRB_TP_Pump2 - %i is out of range", mode);
    return false;
}

bool Spa::setRB_TP_Pump3(byte mode){
    debugD("setRB_TP_Pump3 - %i",mode);
    if ((mode >= 0) && (mode <= 4)) {
        String smode = String(mode);
        return sendCommandCheckResult("S23:"+smode,smode);
    }
    debugW("setRB_TP_Pump3 - %i is out of range", mode);
    return false;
}

bool Spa::setRB_TP_Pump4(byte mode){
    debugD("setRB_TP_Pump4 - %i",mode);
    if ((mode >= 0) && (mode <= 4)) {
        String smode = String(mode);
        return sendCommandCheckResult("S24:"+smode,smode);
    }
    debugW("setRB_TP_Pump4 - %i is out of range", mode);
    return false;
}

bool Spa::setRB_TP_Pump5(byte mode){
    debugD("setRB_TP_Pump5 - %i",mode);
    if ((mode >= 0) && (mode <= 4)) {
        String smode = String(mode);
        return sendCommandCheckResult("S26:"+smode,smode);
    }
    debugW("setRB_TP_Pump5 - %i is out of range", mode);
    return false;
}

bool Spa::setHPMP(byte mode){
    debugD("setHPMP - %i", mode);
    if ((mode >= 0) && (mode <= 4)) {
        String smode = String(mode);
        return sendCommandCheckResult("W99:"+smode,smode);
    };
    debugW("setHPMP - %i is out of range", mode);
    return false;
}

bool Spa::setL_1SNZ_DAY(byte mode){
    debugD("setL_1SNZ_DAY - %i",mode);
    if ((mode >= 0) && (mode <= 128)) {
        String smode = String(mode);
        return sendCommandCheckResult("W67:"+smode,smode);
    }
    debugW("setL_1SNZ_DAY - %i is out of range", mode);
    return false;
}

bool Spa::setL_2SNZ_DAY(byte mode){
    debugD("setL_2SNZ_DAY - %i",mode);
    if ((mode >= 0) && (mode <= 128)) {
        String smode = String(mode);
        return sendCommandCheckResult("W70:"+smode,smode);
    }
    debugW("setL_2SNZ_DAY - %i is out of range", mode);
    return false;
}

bool Spa::setL_1SNZ_BGN(uint16_t time){
    debugD("setL_1SNZ_BGN - %i",time);
    if ((time >= 0) && (time <= 5947)) {
        String smode = String(time);
        return sendCommandCheckResult("W68:"+smode,smode);
    }
    debugW("setL_1SNZ_BGN - %i is out of range", time);
    return false;
}

bool Spa::setL_1SNZ_END(uint16_t time){
    debugD("setL_1SNZ_END - %i",time);
    if ((time >= 0) && (time <= 5947)) {
        String smode = String(time);
        return sendCommandCheckResult("W69:"+smode,smode);
    }
    debugW("setL_1SNZ_END - %i is out of range", time);
    return false;
}

bool Spa::setL_2SNZ_BGN(uint16_t time){
    debugD("setL_1SNZ_BGN - %i",time);
    if ((time >= 0) && (time <= 5947)) {
        String smode = String(time);
        return sendCommandCheckResult("W71:"+smode,smode);
    }
    debugW("setL_1SNZ_BGN - %i is out of range", time);
    return false;
}

bool Spa::setL_2SNZ_END(uint16_t time){
    debugD("setL_1SNZ_END - %i",time);
    if ((time >= 0) && (time <= 5947)) {
        String smode = String(time);
        return sendCommandCheckResult("W72:"+smode,smode);
    }
    debugW("setL_1SNZ_END - %i is out of range", time);
    return false;
}

bool Spa::setLBRTValue(byte brightness){
    debugD("setLBRTValue - %i", brightness);
    if ((brightness >= 1) && (brightness <= 5)) {
        String smode = String(brightness);
        return sendCommandCheckResult("S08:"+smode,smode);
    }
    debugW("setLBRTValue - %i is out of range", brightness);
    return false;
}

bool Spa::setLSPDValue(byte speed){
    debugD("setLSPDValue - %i", speed);
    if ((speed >= 1) && (speed <= 5)) {
        String smode = String(speed);
        return sendCommandCheckResult("S08:"+smode,smode);
    }
    debugW("setLSPDValue - %i is out of range", speed);
    return false; 
}

bool Spa::setCurrClr(byte colour){
    debugD("setCurrClr - %i", colour);
    if ((colour >= 0) && (colour <= 31)) {
        String smode = String(colour);
        return sendCommandCheckResult("S10:"+smode,smode);
    }
    debugW("setCurrClr - %i is out of range", colour);
    return false;
}