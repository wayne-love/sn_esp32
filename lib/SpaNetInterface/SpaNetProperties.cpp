#include "SpaNetProperties.h"


inline boolean isNumber(String s) {
    if (s.isEmpty()) {
        return false;
    }
    for (int i = 0; i < s.length(); i++) {
        if ((!isDigit(s[i])) && (!s[i] == '-') && (!s[i]=='.')) {
            return false;
        }
    }
    return true;
}

boolean SpaNetProperties::update_MainsCurrent(String s){
    if (!isNumber(s)) {
        return false;
    }

    MainsCurrent.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_MainsVoltage(String s){
    if (!isNumber(s)) {
        return false;
    }

    MainsVoltage.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_CaseTemperature(String s){
    if (!isNumber(s)) {
        return false;
    }

    CaseTemperature.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PortCurrent(String s){
    if (!isNumber(s)) {
        return false;
    }

    PortCurrent.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HeaterTemperature(String s){
    if (!isNumber(s)) {
        return false;
    }

    HeaterTemperature.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PoolTemperature(String s){
    if (!isNumber(s)) {
        return false;
    }

    PoolTemperature.update_Value(s.toFloat() / 10);
    return true;
}

boolean SpaNetProperties::update_WaterPresent(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    WaterPresent.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_AwakeMinutesRemaining(String s){
    if (!isNumber(s)) {
        return false;
    }

    AwakeMinutesRemaining.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_FiltPumpRunTimeTotal(String s){
    if (!isNumber(s)) {
        return false;
    }

    FiltPumpRunTimeTotal.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_FiltPumpReqMins(String s){
    if (!isNumber(s)) {
        return false;
    }

    FiltPumpReqMins.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_LoadTimeOut(String s){
    if (!isNumber(s)) {
        return false;
    }

    LoadTimeOut.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HourMeter(String s){
    if (!isNumber(s)) {
        return false;
    }

    HourMeter.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Relay1(String s){
    if (!isNumber(s)) {
        return false;
    }

    Relay1.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Relay2(String s){
    if (!isNumber(s)) {
        return false;
    }

    Relay2.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Relay3(String s){
    if (!isNumber(s)) {
        return false;
    }

    Relay3.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Relay4(String s){
    if (!isNumber(s)) {
        return false;
    }

    Relay4.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Relay5(String s){
    if (!isNumber(s)) {
        return false;
    }

    Relay5.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Relay6(String s){
    if (!isNumber(s)) {
        return false;
    }

    Relay6.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Relay7(String s){
    if (!isNumber(s)) {
        return false;
    }

    Relay7.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Relay8(String s){
    if (!isNumber(s)) {
        return false;
    }

    Relay8.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Relay9(String s){
    if (!isNumber(s)) {
        return false;
    }

    Relay9.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_CLMT(String s){
    if (!isNumber(s)) {
        return false;
    }

    CLMT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PHSE(String s){
    if (!isNumber(s)) {
        return false;
    }

    PHSE.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_LLM1(String s){
    if (!isNumber(s)) {
        return false;
    }

    LLM1.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_LLM2(String s){
    if (!isNumber(s)) {
        return false;
    }

    LLM2.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_LLM3(String s){
    if (!isNumber(s)) {
        return false;
    }

    LLM3.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_SVER(String s){
    SVER.update_Value(s);
    return true;
}

boolean SpaNetProperties::update_Model(String s){
    Model.update_Value(s);
    return true;
}

boolean SpaNetProperties::update_SerialNo1(String s){
    SerialNo1.update_Value(s);
    return true;
}

boolean SpaNetProperties::update_SerialNo2(String s){
    SerialNo2.update_Value(s);
    return true;
}

boolean SpaNetProperties::update_D1(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    D1.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_D2(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    D2.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_D3(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    D3.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_D4(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    D4.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_D5(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    D5.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_D6(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    D6.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_Pump(String s){
    Pump.update_Value(s);
    return true;
}

boolean SpaNetProperties::update_LS(String s){
    if (!isNumber(s)) {
        return false;
    }

    LS.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HV(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    HV.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_SnpMR(String s){
    if (!isNumber(s)) {
        return false;
    }

    SnpMR.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Status(String s){
    Status.update_Value(s);
    return true;
}

boolean SpaNetProperties::update_PrimeCount(String s){
    if (!isNumber(s)) {
        return false;
    }

    PrimeCount.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_EC(String s){
    if (!isNumber(s)) {
        return false;
    }

    EC.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HAMB(String s){
    if (!isNumber(s)) {
        return false;
    }

    HAMB.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HCON(String s){
    if (!isNumber(s)) {
        return false;
    }

    HCON.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Mode(String s){
    Mode.update_Value(s);
    return true;
}

boolean SpaNetProperties::update_Ser1_Timer(String s){
    if (!isNumber(s)) {
        return false;
    }

    Ser1_Timer.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Ser2_Timer(String s){
    if (!isNumber(s)) {
        return false;
    }

    Ser2_Timer.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Ser3_Timer(String s){
    if (!isNumber(s)) {
        return false;
    }

    Ser3_Timer.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HeatMode(String s){
    if (!isNumber(s)) {
        return false;
    }

    HeatMode.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PumpIdleTimer(String s){
    if (!isNumber(s)) {
        return false;
    }

    PumpIdleTimer.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PumpRunTimer(String s){
    if (!isNumber(s)) {
        return false;
    }

    PumpRunTimer.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_AdtPoolHys(String s){
    if (!isNumber(s)) {
        return false;
    }

    AdtPoolHys.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_AdtHeaterHys(String s){
    if (!isNumber(s)) {
        return false;
    }

    AdtHeaterHys.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Power(String s){
    if (!isNumber(s)) {
        return false;
    }

    Power.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Power_kWh(String s){
    if (!isNumber(s)) {
        return false;
    }

    Power_kWh.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Power_Today(String s){
    if (!isNumber(s)) {
        return false;
    }

    Power_Today.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Power_Yesterday(String s){
    if (!isNumber(s)) {
        return false;
    }

    Power_Yesterday.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_ThermalCutOut(String s){
    if (!isNumber(s)) {
        return false;
    }

    ThermalCutOut.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Test_D1(String s){
    if (!isNumber(s)) {
        return false;
    }

    Test_D1.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Test_D2(String s){
    if (!isNumber(s)) {
        return false;
    }

    Test_D2.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Test_D3(String s){
    if (!isNumber(s)) {
        return false;
    }

    Test_D3.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_ElementHeatSourceOffset(String s){
    if (!isNumber(s)) {
        return false;
    }

    ElementHeatSourceOffset.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Frequency(String s){
    if (!isNumber(s)) {
        return false;
    }

    Frequency.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HPHeatSourceOffset_Heat(String s){
    if (!isNumber(s)) {
        return false;
    }

    HPHeatSourceOffset_Heat.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HPHeatSourceOffset_Cool(String s){
    if (!isNumber(s)) {
        return false;
    }

    HPHeatSourceOffset_Cool.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HeatSourceOffTime(String s){
    if (!isNumber(s)) {
        return false;
    }

    HeatSourceOffTime.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Vari_Speed(String s){
    if (!isNumber(s)) {
        return false;
    }

    Vari_Speed.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Vari_Percent(String s){
    if (!isNumber(s)) {
        return false;
    }

    Vari_Percent.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Vari_Mode(String s){
    if (!isNumber(s)) {
        return false;
    }

    Vari_Mode.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_RB_TP_Pump1(String s){
    if (!isNumber(s)) {
        return false;
    }

    RB_TP_Pump1.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_RB_TP_Pump2(String s){
    if (!isNumber(s)) {
        return false;
    }

    RB_TP_Pump2.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_RB_TP_Pump3(String s){
    if (!isNumber(s)) {
        return false;
    }

    RB_TP_Pump3.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_RB_TP_Pump4(String s){
    if (!isNumber(s)) {
        return false;
    }

    RB_TP_Pump4.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_RB_TP_Pump5(String s){
    if (!isNumber(s)) {
        return false;
    }

    RB_TP_Pump5.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_RB_TP_Blower(String s){
    if (!isNumber(s)) {
        return false;
    }

    RB_TP_Blower.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_RB_TP_Light(String s){
    if (!isNumber(s)) {
        return false;
    }

    RB_TP_Light.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_RB_TP_Auto(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    RB_TP_Auto.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_RB_TP_Heater(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    RB_TP_Heater.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_RB_TP_Ozone(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    RB_TP_Ozone.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_RB_TP_Sleep(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    RB_TP_Sleep.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_WTMP(String s){
    if (!isNumber(s)) {
        return false;
    }

    WTMP.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_CleanCycle(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    CleanCycle.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_VARIValue(String s){
    if (!isNumber(s)) {
        return false;
    }

    VARIValue.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_LBRTValue(String s){
    if (!isNumber(s)) {
        return false;
    }

    LBRTValue.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_CurrClr(String s){
    if (!isNumber(s)) {
        return false;
    }

    CurrClr.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_ColorMode(String s){
    if (!isNumber(s)) {
        return false;
    }

    ColorMode.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_LSPDValue(String s){
    if (!isNumber(s)) {
        return false;
    }

    LSPDValue.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_FiltSetHrs(String s){
    if (!isNumber(s)) {
        return false;
    }

    FiltSetHrs.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_FiltBlockHrs(String s){
    if (!isNumber(s)) {
        return false;
    }

    FiltBlockHrs.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_STMP(String s){
    if (!isNumber(s)) {
        return false;
    }

    STMP.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_L_24HOURS(String s){
    if (!isNumber(s)) {
        return false;
    }

    L_24HOURS.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PSAV_LVL(String s){
    if (!isNumber(s)) {
        return false;
    }

    PSAV_LVL.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PSAV_BGN(String s){
    if (!isNumber(s)) {
        return false;
    }

    PSAV_BGN.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PSAV_END(String s){
    if (!isNumber(s)) {
        return false;
    }

    PSAV_END.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_L_1SNZ_DAY(String s){
    if (!isNumber(s)) {
        return false;
    }

    L_1SNZ_DAY.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_L_2SNZ_DAY(String s){
    if (!isNumber(s)) {
        return false;
    }

    L_2SNZ_DAY.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_L_1SNZ_BGN(String s){
    if (!isNumber(s)) {
        return false;
    }

    L_1SNZ_BGN.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_L_2SNZ_BGN(String s){
    if (!isNumber(s)) {
        return false;
    }

    L_2SNZ_BGN.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_L_1SNZ_END(String s){
    if (!isNumber(s)) {
        return false;
    }

    L_1SNZ_END.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_L_2SNZ_END(String s){
    if (!isNumber(s)) {
        return false;
    }

    L_2SNZ_END.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_DefaultScrn(String s){
    if (!isNumber(s)) {
        return false;
    }

    DefaultScrn.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_TOUT(String s){
    if (!isNumber(s)) {
        return false;
    }

    TOUT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_VPMP(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    VPMP.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_HIFI(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    HIFI.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_BRND(String s){
    if (!isNumber(s)) {
        return false;
    }

    BRND.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PRME(String s){
    if (!isNumber(s)) {
        return false;
    }

    PRME.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_ELMT(String s){
    if (!isNumber(s)) {
        return false;
    }

    ELMT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_TYPE(String s){
    if (!isNumber(s)) {
        return false;
    }

    TYPE.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_GAS(String s){
    if (!isNumber(s)) {
        return false;
    }

    GAS.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_WCLNTime(String s){
    if (!isNumber(s)) {
        return false;
    }

    WCLNTime.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_TemperatureUnits(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    TemperatureUnits.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_OzoneOff(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    OzoneOff.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_Ozone24(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    Ozone24.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_Circ24(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    Circ24.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_CJET(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    CJET.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_VELE(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    VELE.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_V_Max(String s){
    if (!isNumber(s)) {
        return false;
    }

    V_Max.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_V_Min(String s){
    if (!isNumber(s)) {
        return false;
    }

    V_Min.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_V_Max_24(String s){
    if (!isNumber(s)) {
        return false;
    }

    V_Max_24.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_V_Min_24(String s){
    if (!isNumber(s)) {
        return false;
    }

    V_Min_24.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_CurrentZero(String s){
    if (!isNumber(s)) {
        return false;
    }

    CurrentZero.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_CurrentAdjust(String s){
    if (!isNumber(s)) {
        return false;
    }

    CurrentAdjust.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_VoltageAdjust(String s){
    if (!isNumber(s)) {
        return false;
    }

    VoltageAdjust.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Ser1(String s){
    if (!isNumber(s)) {
        return false;
    }

    Ser1.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Ser2(String s){
    if (!isNumber(s)) {
        return false;
    }

    Ser2.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Ser3(String s){
    if (!isNumber(s)) {
        return false;
    }

    Ser3.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_VMAX(String s){
    if (!isNumber(s)) {
        return false;
    }

    VMAX.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_AHYS(String s){
    if (!isNumber(s)) {
        return false;
    }

    AHYS.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HUSE(String s){
    if (!isNumber(s)) {
        return false;
    }

    HUSE.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HELE(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    HELE.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_HPMP(String s){
    if (!isNumber(s)) {
        return false;
    }

    HPMP.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PMIN(String s){
    if (!isNumber(s)) {
        return false;
    }

    PMIN.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PFLT(String s){
    if (!isNumber(s)) {
        return false;
    }

    PFLT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PHTR(String s){
    if (!isNumber(s)) {
        return false;
    }

    PHTR.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_PMAX(String s){
    if (!isNumber(s)) {
        return false;
    }

    PMAX.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F1_HR(String s){
    if (!isNumber(s)) {
        return false;
    }

    F1_HR.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F1_Time(String s){
    if (!isNumber(s)) {
        return false;
    }

    F1_Time.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F1_ER(String s){
    if (!isNumber(s)) {
        return false;
    }

    F1_ER.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F1_I(String s){
    if (!isNumber(s)) {
        return false;
    }

    F1_I.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F1_V(String s){
    if (!isNumber(s)) {
        return false;
    }

    F1_V.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F1_PT(String s){
    if (!isNumber(s)) {
        return false;
    }

    F1_PT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F1_HT(String s){
    if (!isNumber(s)) {
        return false;
    }

    F1_HT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F1_CT(String s){
    if (!isNumber(s)) {
        return false;
    }

    F1_CT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F1_ST(String s){
    if (!isNumber(s)) {
        return false;
    }

    F1_ST.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F1_PU(String s){
    if (!isNumber(s)) {
        return false;
    }

    F1_PU.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F1_VE(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    F1_VE.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_F2_HR(String s){
    if (!isNumber(s)) {
        return false;
    }

    F2_HR.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F2_Time(String s){
    if (!isNumber(s)) {
        return false;
    }

    F2_Time.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F2_ER(String s){
    if (!isNumber(s)) {
        return false;
    }

    F2_ER.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F2_I(String s){
    if (!isNumber(s)) {
        return false;
    }

    F2_I.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F2_V(String s){
    if (!isNumber(s)) {
        return false;
    }

    F2_V.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F2_PT(String s){
    if (!isNumber(s)) {
        return false;
    }

    F2_PT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F2_HT(String s){
    if (!isNumber(s)) {
        return false;
    }

    F2_HT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F2_CT(String s){
    if (!isNumber(s)) {
        return false;
    }

    F2_CT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F2_ST(String s){
    if (!isNumber(s)) {
        return false;
    }

    F2_ST.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F2_PU(String s){
    if (!isNumber(s)) {
        return false;
    }

    F2_PU.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F2_VE(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    F2_VE.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_F3_HR(String s){
    if (!isNumber(s)) {
        return false;
    }

    F3_HR.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F3_Time(String s){
    if (!isNumber(s)) {
        return false;
    }

    F3_Time.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F3_ER(String s){
    if (!isNumber(s)) {
        return false;
    }

    F3_ER.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F3_I(String s){
    if (!isNumber(s)) {
        return false;
    }

    F3_I.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F3_V(String s){
    if (!isNumber(s)) {
        return false;
    }

    F3_V.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F3_PT(String s){
    if (!isNumber(s)) {
        return false;
    }

    F3_PT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F3_HT(String s){
    if (!isNumber(s)) {
        return false;
    }

    F3_HT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F3_CT(String s){
    if (!isNumber(s)) {
        return false;
    }

    F3_CT.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F3_ST(String s){
    if (!isNumber(s)) {
        return false;
    }

    F3_ST.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F3_PU(String s){
    if (!isNumber(s)) {
        return false;
    }

    F3_PU.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_F3_VE(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    F3_VE.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_Outlet_Blower(String s){
    if (!isNumber(s)) {
        return false;
    }

    Outlet_Blower.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HP_Present(String s){
    if (!isNumber(s)) {
        return false;
    }

    HP_Present.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HP_Ambient(String s){
    if (!isNumber(s)) {
        return false;
    }

    HP_Ambient.update_Value(s.toInt());
    return true;
}


boolean SpaNetProperties::update_HP_Condensor(String s){
    if (!isNumber(s)) {
        return false;
    }

    HP_Condensor.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HP_Compressor_State(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    HP_Compressor_State.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_HP_Fan_State(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    HP_Fan_State.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_HP_4W_Valve(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    HP_4W_Valve.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_HP_Heater_State(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    HP_Heater_State.update_Value( s == "1" );
    return true;
}


boolean SpaNetProperties::update_HP_State(String s){
    if (!isNumber(s)) {
        return false;
    }

    HP_State.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HP_Mode(String s){
    if (!isNumber(s)) {
        return false;
    }

    HP_Mode.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HP_Defrost_Timer(String s){
    if (!isNumber(s)) {
        return false;
    }

    HP_Defrost_Timer.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HP_Comp_Run_Timer(String s){
    if (!isNumber(s)) {
        return false;
    }

    HP_Comp_Run_Timer.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HP_Low_Temp_Timer(String s){
    if (!isNumber(s)) {
        return false;
    }

    HP_Low_Temp_Timer.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HP_Heat_Accum_Timer(String s){
    if (!isNumber(s)) {
        return false;
    }

    HP_Heat_Accum_Timer.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HP_Sequence_Timer(String s){
    if (!isNumber(s)) {
        return false;
    }

    HP_Sequence_Timer.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_HP_Warning(String s){
    if (!isNumber(s)) {
        return false;
    }

    HP_Warning.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_FrezTmr(String s){
    if (!isNumber(s)) {
        return false;
    }

    FrezTmr.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_DBGN(String s){
    if (!isNumber(s)) {
        return false;
    }

    DBGN.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_DEND(String s){
    if (!isNumber(s)) {
        return false;
    }

    DEND.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_DCMP(String s){
    if (!isNumber(s)) {
        return false;
    }

    DCMP.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_DMAX(String s){
    if (!isNumber(s)) {
        return false;
    }

    DMAX.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_DELE(String s){
    if (!isNumber(s)) {
        return false;
    }

    DELE.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_DPMP(String s){
    if (!isNumber(s)) {
        return false;
    }

    DPMP.update_Value(s.toInt());
    return true;
}

boolean SpaNetProperties::update_Pump1InstallState(String s){
    Pump1InstallState.update_Value(s);
    return true;
}

boolean SpaNetProperties::update_Pump2InstallState(String s){
    Pump2InstallState.update_Value(s);
    return true;
}

boolean SpaNetProperties::update_Pump3InstallState(String s){
    Pump3InstallState.update_Value(s);
    return true;
}

boolean SpaNetProperties::update_Pump4InstallState(String s){
    Pump4InstallState.update_Value(s);
    return true;
}

boolean SpaNetProperties::update_Pump5InstallState(String s){
    Pump5InstallState.update_Value(s);
    return true;
}

boolean SpaNetProperties::update_Pump1OkToRun(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    Pump1OkToRun.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_Pump2OkToRun(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    Pump2OkToRun.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_Pump3OkToRun(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    Pump3OkToRun.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_Pump4OkToRun(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    Pump4OkToRun.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_Pump5OkToRun(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    Pump5OkToRun.update_Value( s == "1" );
    return true;
}

boolean SpaNetProperties::update_LockMode(String s) {
    if (s!="0" && s!="1") {
        return false;
    }

    LockMode.update_Value( s == "1" );
    return true;
}
