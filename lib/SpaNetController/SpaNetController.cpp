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



bool SpaNetController::parseStatus(String str) {

//R4 is hit and miss as to if it returns all its data.
//So need to work on a variable number of rows 
//returned.

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

  // First datapoint in registers is at array position 2
  if (registers[1].isValid() && registers[5].isValid() && registers[11].isValid()) {
    amps = float(String(registers[1].getField(2)).toInt())/10;
    volts = String(registers[1].getField(3)).toInt();
    hpump_amb_temperature = String(registers[11].getField(11)).toInt();
    hpump_con_temperature = String(registers[11].getField(12)).toInt();
    lightsOn = String(registers[4].getField(15)).toInt();
    return true;
  } else { return false; }

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
    command c = commands.front();
    debugD("Processing command %s/%s",c.topic.c_str(),c.payload.c_str());
    commands.pop_front();
    if (c.topic=="lights" && (c.payload=="0" || c.payload=="1")) {
      sendCommand("W14");
    }
  }

  getRegisters();

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


void SpaNetController::pushCommand(String t, String p){
  debugD("Pushing %s/%s to command queue",t.c_str(),p.c_str());
  commands.push_back({t,p});
  debugD("%d commands in queue",commands.size());
}

bool SpaNetController::isLightsOn() {
  return lightsOn;
}