#include "SpaNetController.h"




Register::Register(int req) {
  requiredFields = req;
}


bool Register::updateRegister(const char update[]) {

  strcpy(reg, update);
  int len=strlen(reg);
  int y=1;
  element[0]=&reg[0];
  for(int x=0; x<len; x++) {        //split the string into a number of smaller strings
    if (reg[x]==',') {              //each terminated by 0x0. create a pointer to the 
      reg[x]=0;                     //start of each smaller string.
      element[y]=&reg[x+1];
      y++;
      }
  }

  if (y==requiredFields) {
    valid = true;
  } else {
    debugW("Error parsing register, expected %d fields, got %d",requiredFields,y);
    debugD("Update register request - %s",update);
    valid = false;
  }

  return valid;
}

char* Register::getField(int field) {
  return element[field];
}

bool Register::isValid() {
  return valid;
}


// *** Pump

void Pump::initialise(bool installed, bool autoOperation) {
  _installed = installed;
  _autoOperation = autoOperation;
}

bool Pump::isInstalled() {
  return _installed;
}

void Pump::setOperatingMode(int mode){
  _mode = mode;
}

int Pump::getOperatingMode() {
  return _mode;
}

bool Pump::isAutoModeSupported(){
  return _autoOperation;
}



// *** Start SpaNetController

SpaNetController::SpaNetController() {
    Serial2.begin(38400,SERIAL_8N1, 16, 17);
    Serial2.setTimeout(500);
}

SpaNetController::~SpaNetController() {}


float SpaNetController::getAmps(){
  return amps;
}

int SpaNetController::getVolts(){
  return volts;
}

float SpaNetController::getHpumpAmbTemp(){
  return hpump_amb_temperature;
}

float SpaNetController::getHpumpConTemp(){
  return hpump_con_temperature;
}

float SpaNetController::getWaterTemp(){
  return waterTemperature;
}

float SpaNetController::getWaterTempSetPoint(){
  return waterTemperatureSetPoint;
}


bool SpaNetController::setWaterTempSetPoint(float temp){
  // Should do some error checking here to ensure that we arnt tryting to freeze the spa
  String cmd = "W40:" + String(int(temp * 10));
  commands.emplace_back(cmd);
  return true;
}

bool SpaNetController::setPump1Operating(int operatingMode){
  String cmd = "S22:" + String(operatingMode);
  commands.emplace_back(cmd);
  return true;
}

bool SpaNetController::setPump2Operating(int operatingMode){
  String cmd = "S23:" + String(operatingMode);
  commands.emplace_back(cmd);
  return true;
}

bool SpaNetController::setPump3Operating(int operatingMode){
  String cmd = "S24:" + String(operatingMode);
  commands.emplace_back(cmd);
  return true;
}

bool SpaNetController::setPump4Operating(int operatingMode){
  String cmd = "S25:" + String(operatingMode);
  commands.emplace_back(cmd);
  return true;
}

bool SpaNetController::setPump5Operating(int operatingMode){
  String cmd = "S26:" + operatingMode;
  commands.emplace_back(cmd);
  return true;
}

SpaNetController::heat_pump_modes SpaNetController::getHeatPumpMode(){
  return heatPumpMode;
}

bool SpaNetController::setHeatPumpMode(SpaNetController::heat_pump_modes mode){
  String cmd = "W99:" + String(mode);
  commands.emplace_back(cmd);
  return true;
}

bool SpaNetController::isAuxHeatingEnabled(){
  return auxHeatElement;
}

bool SpaNetController::setAuxHeatingEnabled(bool enabled){
  String cmd;
  if (enabled) {
    cmd="W98:1";
  } else {
    cmd="W98:0";
  }
  commands.emplace_back(cmd);
  return true;
}

bool SpaNetController::isLightsOn() {
  return lightsOn;
}

bool SpaNetController::toggleLights(){
  String cmd = "W14";
  commands.emplace_back(cmd);
  return true;
}

bool SpaNetController::isHeatingOn() {
  return heatingActive;
}

bool SpaNetController::isUVOn() {
  return uvActive;
}

bool SpaNetController::isSanatiseRunning() {
  return sanatiseActive;
}

String SpaNetController::getSerialNo() {
  return serialNo;
}

char* SpaNetController::getStatus() {
  return registers[2].getField(21);
}

float SpaNetController::getHeaterTemp() {
  return heater_temperature;
}

bool SpaNetController::initialised() {
  return _firstrun;
}

bool SpaNetController::pumpInstalled(int pump){
  return pumps[pump].isInstalled();
}

Pump* SpaNetController::getPump(int pump){
  return &pumps[pump];
}

bool SpaNetController::parseStatus(String str) {

//R4 is hit and miss as to if it returns all its data.
//So need to work on a variable number of rows 
//returned.

  debugD("Parsing status string");

  int currentPos = 0;
  int colonIndex = str.indexOf(':');

  while (colonIndex>-1) {
    int r = str.indexOf('R',currentPos);
    if (r>-1){
      int currentReg = -1;
      char rChar = str.charAt(r+1); //should really check if r is the last character of string
      if (rChar=='F') { currentReg = 0; }
      else if (rChar=='2') { currentReg = 1; }
      else if (rChar=='3') { currentReg = 2; }
      else if (rChar=='4') { currentReg = 3; }
      else if (rChar=='5') { currentReg = 4; }
      else if (rChar=='6') { currentReg = 5; }
      else if (rChar=='7') { currentReg = 6; }
      else if (rChar=='9') { currentReg = 7; }
      else if (rChar=='A') { currentReg = 8; }
      else if (rChar=='B') { currentReg = 9; }
      else if (rChar=='C') { currentReg = 10; }
      else if (rChar=='E') { currentReg = 11; }
      else if (rChar=='G') { currentReg = 12; }
      if (currentReg>-1) {
        !registers[currentReg].updateRegister(str.substring(currentPos,colonIndex).c_str());
      }
    }
    currentPos=colonIndex+1;
    colonIndex=str.indexOf(':',currentPos);

  }

  bool regValid = registers[1].isValid() && \
    registers[2].isValid() && \
    registers[4].isValid() && \
    registers[5].isValid() && \
    registers[6].isValid() && \
    registers[9].isValid() && \
    registers[11].isValid() &&\
    registers[12].isValid();

  // First datapoint in registers is at array position 2
  if (regValid)
  {
    
    amps = float(String(registers[1].getField(2)).toInt())/10;
    
    volts = String(registers[1].getField(3)).toInt();

    lightsOn = String(registers[4].getField(15)).toInt();
    waterTemperature = float(String(registers[4].getField(16)).toInt())/10;

    waterTemperatureSetPoint = float(String(registers[5].getField(9)).toInt())/10;

    heatPumpMode=heat_pump_modes(String(registers[6].getField(27)).toInt());

    hpump_amb_temperature = String(registers[11].getField(11)).toInt();
    hpump_con_temperature = String(registers[11].getField(12)).toInt();

    auxHeatElement = bool(String(registers[6].getField(26)).toInt());

    heatingActive = bool(String(registers[4].getField(13)).toInt());

    uvActive = bool(String(registers[4].getField(12)).toInt());

    sanatiseActive = bool(String(registers[4].getField(17)).toInt());

    heater_temperature = float(String(registers[1].getField(13)).toInt()) / 10;

    for (int x = 0; x < 5; x++) {
      pumps[x].setOperatingMode(String(registers[4].getField(19 + x)).toInt()); 
    }

    if (!_firstrun) { // On first read, set static variables & set initialised flag

      debugD("First time, setting static elements");
      debugD("Serial number set to '%s'", registers[2].getField(9));

      serialNo = String(registers[2].getField(9));

      for (int x = 0; x < 5;x++) {

        // Itterate through string until you get to the 3rd field
        // ('-' delimiter field boundaries).  If 3rd field contains '4'
        // then the pump supports auto operation.
        bool ao = false;
        char *s = registers[12].getField(8 + x);
        int len = strlen(s);
        int d_count = 0;
        for (int c = 0; c < len;c++){
          if (s[c]=='-') {
            d_count++;
          }
          if (d_count==2) {
            if (s[c]=='4') {
              ao = true;
            }
          }
        }

          pumps[x].initialise(registers[12].getField(8 + x)[0] == '1', ao);
      }

      _firstrun = true;
    }


  }

  return regValid;
}

String SpaNetController::sendCommand(String cmd) {
  debugD("Sending %s",cmd.c_str());
  Serial2.printf("\n");
  delay(100);
  Serial2.print(cmd+"\n");
  delay(100);
  String resp = Serial2.readString();
  debugD("Received %s",resp.c_str());
  return resp;
}

bool SpaNetController::pollStatus(){
  if (parseStatus(sendCommand("RF"))) {
    debugD("Successful register poll, notify subscribers.");
    if (update) {
      debugD("Subscriber update called");
      update(this);
      }
    return true;
  } else {
    return false;
  }
}

void SpaNetController::getRegisters() {
  if (pollStatus()) {
    _nextUpdate = millis() + UPDATEFREQUENCY;
  } else {
    _nextUpdate = millis() + 1000;
  }
}

void SpaNetController::processCommands() {

  debugD("Processing %d commands in command queue",commands.size());

  while(commands.size()>0){
    String command = commands.front();
    debugD("Processing command %s",command.c_str());
    sendCommand(command);
    commands.pop_front();
  }

  getRegisters();

}

void SpaNetController::forceUpdate(){
  _nextUpdate = 0;
}

void SpaNetController::tick(){

  if (millis()>_nextUpdate) {
    getRegisters();
  }

  if ((commands.size()>0) && (millis()-lastCommand) > 500) {
    lastCommand = millis();
    processCommands();
  }

}

void SpaNetController::subscribeUpdate(void (*u)(SpaNetController *)){
  this->update=u;
}