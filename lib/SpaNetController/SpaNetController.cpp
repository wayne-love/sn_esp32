#include "SpaNetController.h"


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




bool SpaNetController::parseStatus(String str) {

  int element=1;
  int commaIndex=str.indexOf(',');
  int elementBoundaries[290];

  elementBoundaries[0]=0;
  
  while (commaIndex>-1){
    elementBoundaries[element] = commaIndex;
    element++;
    commaIndex=str.indexOf(',',commaIndex+1);
  }

  elementBoundaries[element] = str.length();

  if (element != 290) { 
    debugW("Wrong number of parameters read, read %d, expecting 289", element-1);
    return false;
  }
  else {
    debugI("Successful read of SpaNet status");
    amps = float(str.substring(elementBoundaries[2]+1,elementBoundaries[3]).toInt())/10;
    volts = str.substring(elementBoundaries[3]+1,elementBoundaries[4]).toInt();
    hpump_amb_temperature = float(str.substring(elementBoundaries[251]+1,elementBoundaries[252]).toInt());
    lightsOn = bool(str.substring(elementBoundaries[106]+1,elementBoundaries[107]).toInt());
    return true;
  }
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