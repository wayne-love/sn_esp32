#include "SpaNetController.h"


SpaNetController::SpaNetController() {
    Serial2.begin(38400,SERIAL_8N1, 16, 17);
    Serial2.setTimeout(1000); 
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
  
  debugD("Read...");
  debugD("%s",str.c_str());

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
    return true;
  }
}

String SpaNetController::sendCommand(String cmd) {
  Serial2.printf("\n");
  delay(100);
  Serial2.print(cmd+"\n");
  delay(100);
  return Serial2.readString();
}

bool SpaNetController::pollStatus(){
  if (parseStatus(sendCommand("RF"))) {
    if (update) {update(this);}
    return true;
  } else {
    return false;
  }
}

void SpaNetController::tick(){
  if (millis()>_nextUpdate) {
    if (pollStatus()) {
      _nextUpdate = millis() + UPDATEFREQUENCY;
    } else {
      _nextUpdate = millis() + 1000;
    }
  }
}

void SpaNetController::subscribeUpdate(void (*u)(SpaNetController *)){
  this->update=u;
}
