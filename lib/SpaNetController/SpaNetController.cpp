#include "SpaNetController.h"

#define NUM(a) (sizeof(a) / sizeof(*a))
#define BAUD_RATE 38400

#if defined(ESP8266)
  #define RX_PIN 14 //goes to rx on spanet pin5
  #define TX_PIN 12 //goes to tx on spanet pin6

  #include <SoftwareSerial.h>

  SoftwareSerial spaSerial;
#elif defined(ESP32)
  #define RX_PIN 16 //goes to rx on spanet pin5
  #define TX_PIN 17 //goes to tx on spanet pin6
  HardwareSerial spaSerial = Serial2;
#endif


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
const char *Pump::pump_modes[5] = {"Off", "On", "", "", "Auto"};

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

SpaNetController::SpaNetController()
  : lights(this) {
    
    #if defined(ESP8266)
        // Start Software Serial
        spaSerial.begin(BAUD_RATE, SWSERIAL_8N1, RX_PIN, TX_PIN, false, 95, 11);  
    #elif defined(ESP32)
        spaSerial.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
    #endif
    
    spaSerial.setTimeout(250);
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


void SpaNetController::queueCommand(String command) {
  commands.emplace_back(command);
}

bool SpaNetController::setWaterTempSetPoint(float temp){
  if (temp < 10 || temp > 40) { return false;}  //TODO - Confirm that 40 is the actual top.
  
  String cmd = "W40:" + String(int(temp * 10));
  queueCommand(cmd);
  return true;
}

bool SpaNetController::setPumpOperating(int pump,int mode){
  debugD("pump=%d,mode=%d", pump, mode);
  String cmd = "S"+String(pump+21)+":" + String(mode);
  queueCommand(cmd);
  return true;
}

void SpaNetController::setPumpOperating(int pump, const char *mode){
  debugD("pump=%d,mode=%s", pump, mode);
  for (int i = 0; i < PUMP_MODES_COUNT; i++ ) {
    if (strcmp(mode,Pump::pump_modes[i])==0) {
      setPumpOperating(pump,i);
    }
  }
}

bool SpaNetController::setPump1Operating(int operatingMode){
  String cmd = "S22:" + String(operatingMode);
  queueCommand(cmd);
  return true;
}

bool SpaNetController::setPump2Operating(int operatingMode){
  String cmd = "S23:" + String(operatingMode);
  queueCommand(cmd);
  return true;
}

bool SpaNetController::setPump3Operating(int operatingMode){
  String cmd = "S24:" + String(operatingMode);
  queueCommand(cmd);
  return true;
}

bool SpaNetController::setPump4Operating(int operatingMode){
  String cmd = "S25:" + String(operatingMode);
  queueCommand(cmd);
  return true;
}

bool SpaNetController::setPump5Operating(int operatingMode){
  String cmd = "S26:" + operatingMode;
  queueCommand(cmd);
  return true;
}

SpaNetController::heat_pump_modes SpaNetController::getHeatPumpMode(){
  return heatPumpMode;
}

bool SpaNetController::setHeatPumpMode(SpaNetController::heat_pump_modes mode){
  String cmd = "W99:" + String(mode);
  queueCommand(cmd);
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
  queueCommand(cmd);
  return true;
}


float SpaNetController::getPower() { return instEnergy; }
float SpaNetController::getTotalEnergy() { return totalPower; }
float SpaNetController::getEnergyToday() { return powerToday; }


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
    registers[3].isValid() && \
    registers[4].isValid() && \
    registers[5].isValid() && \
    registers[6].isValid() && \
    registers[7].isValid() && \
    registers[9].isValid() && \
    registers[11].isValid() &&\
    registers[12].isValid();

  // First datapoint in registers is at array position 2
  if (regValid)
  {
    
    instEnergy = float(String(registers[3].getField(11)).toInt())/10;
    totalPower = float(String(registers[3].getField(12)).toInt())/100;
    powerToday = float(String(registers[3].getField(13)).toInt())/100;

    amps = float(String(registers[1].getField(2)).toInt())/10;
    
    volts = String(registers[1].getField(3)).toInt();

    lights._isOn = (strcmp(registers[4].getField(15),"1") == 0);
    lights._mode = byte(String(registers[5].getField(5)).toInt());
    lights._brightness = byte(String(registers[5].getField(3)).toInt());

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
  
  spaSerial.printf("\n");
  delay(100);
  spaSerial.print(cmd+"\n");
  String resp = spaSerial.readString();

  debugD("Received %s",resp.c_str());
  
  return resp;

}

bool SpaNetController::pollStatus(){

  debugD("Polling Status with RF command");
  
  if (parseStatus(sendCommand("RF"))) {
    debugD("Successful register poll, notify subscribers.");
    if (update) { //If there is a function pointer assiged then call function
      update(this);
      }
    return true;
  } else {
    return false;
  }
}

void SpaNetController::getRegisters() {
  debugD("Getting registers");
  if (pollStatus()) {
    _nextUpdate = millis() + UPDATEFREQUENCY;
  } else {
    _nextUpdate = millis() + 1000;
  }
}

void SpaNetController::processCommands() {

  debugD("Processsing commands");
  debugD("%d commands in command queue",commands.size());

  while(commands.size()>0){
    String command = commands.front();
    debugD("Processing command %s",command.c_str());
    sendCommand(command);
    commands.pop_front();
    delay(100);
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


/**
 * @brief Construct a new Spa Net Controller:: Light:: Light object
 * 
 * @param p pointer to parent SpaNetController object
 */
SpaNetController::Light::Light(SpaNetController* p){
  _parent = p;
}

/**
 * @brief Operating modes for spa lights {"White", "Color", "Fade", "Step", "Party"} 
 * 
 */
const char *SpaNetController::Light::light_modes[] = {"White", "Color", "Fade", "Step", "Party"};

/**
 * @brief Colour hue to controller colour integer identifier mapping
 * 
 */
const byte SpaNetController::Light::colour_map[] = {0, 4, 4, 19, 13, 25, 25, 16, 10, 7, 2, 8, 5, 3, 6, 6, 21, 21, 21, 18, 18, 9, 9, 1, 1};


/**
 * @brief Empty function
 * 
 */
SpaNetController::Light::~Light(){}


/**
 * @brief Is the spa light on?
 * 
 * @return true Light is on
 * @return false Light is off
 */
bool SpaNetController::Light::isOn() {
  return _isOn;
}

/**
 * @brief Set the light state
 * 
 * @param state True - lights on, False - lights off
 */
void SpaNetController::Light::setIsOn(bool state) {
  if (state != _isOn) {
    _parent->queueCommand("W14");
  } 
}

/**
 * @brief Get current light mode
 * 
 * @return const char* - one of "White", "Color", "Fade", "Step", "Party"
 */
const char *SpaNetController::Light::getMode(){
  return light_modes[_mode];
}

/**
 * @brief Set light mode
 * 
 * @param mode - one of "White", "Color", "Fade", "Step", "Party"
 */
void SpaNetController::Light::setMode(const char *mode){
  for (byte count = 0; count < NUM(light_modes); count++) {
    if (strcmp(mode,light_modes[count])==0) {
      setMode(count);
      return;
    }
  }
}

void SpaNetController::Light::setMode(byte mode){
  if (mode != _mode) {
      _parent->queueCommand("S07:" + String(mode));
  }
}

/**
 * @brief Gets the current brightness setting
 * 
 * @return byte (min 1, max 5)
 */
byte SpaNetController::Light::getBrightness() {
  return _brightness;
}


/**
 * @brief Sets the lights brightness
 * 
 * @param value integer between 1 and 5
 */
void SpaNetController::Light::setBrightness(byte value) {
  if (value != _brightness) {
    _parent->queueCommand("S08:" + String(value));
  }
}

/**
 * @brief Returns the colour of the spa lights 
 * 
 * @return int 
 */
int SpaNetController::Light::getColour() {
  return _colour;
}

/**
 * @brief Set the lights colour
 * 
 * @param colour 
 */
void SpaNetController::Light::setColour(int colour){
  if (colour != _colour) {
    _parent->queueCommand("S10:" + String(colour));
    _colour = colour;
  }
}