#include "main.h"
#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>

#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>

#endif

#include <WiFiClient.h>
#include <WiFiManager.h>
#include <RemoteDebug.h>
#include <PubSubClient.h>
#include <LittleFS.h>
//#include <ArduinoJson.h>
#include <vector>
#include <time.h>
#include <TimeLib.h>
#if defined(LED_PIN)
#include "Blinker.h"
#endif
#include "WebUI.h"

#include "SpaInterface.h"
#include "SpaUtils.h"
#include "HAAutoDiscovery.h"

#if defined(ESP32)
#include <Preferences.h>
#endif

#ifndef DEBUG_ENABLED
    #define DEBUG_ENABLED
    RemoteDebug Debug;
#endif

// Define the threshold for detecting a fast reboot (within 20 seconds)
#define REBOOT_THRESHOLD 20

#if defined(ESP8266)
struct RTCData {
  int16_t magicNumber;  // Use a unique number to identify valid data
  int16_t rebootFlag;   // 1 start wifi manager on next boot
};
RTCData rtcData;
const int16_t MAGIC_NUMBER = 0xAAAA;
#endif

#if defined(ESP32)
Preferences preferences;  // For ESP32 reboot storage
#endif

unsigned long bootStartMillis;  // To track when the device started
bool rebootFlag = false;

SpaInterface si;

#if defined(LED_PIN)
Blinker led(LED_PIN);
#endif
WiFiClient wifi;
PubSubClient mqttClient(wifi);

WebUI ui(&si);



bool saveConfig = false;
ulong mqttLastConnect = 0;
ulong wifiLastConnect = millis();
ulong bootTime = millis();
ulong statusLastPublish = millis();
bool delayedStart = true; // Delay spa connection for 10sec after boot to allow for external debugging if required.
bool autoDiscoveryPublished = false;

String mqttServer = "";
String mqttPort = "";
String mqttUserName = "";
String mqttPassword = "";

String mqttBase = "";
String mqttStatusTopic = "";
String mqttSet = "";
String mqttAvailability = "";

String spaSerialNumber = "";

void saveConfigCallback(){
  saveConfig = true;
}

bool readRebootFlag() {
  bool retVal = false;

  #if defined(ESP8266)
    ESP.rtcUserMemoryRead(0, (uint32_t*)&rtcData, sizeof(rtcData));
    // Check if the magic number matches
    if (rtcData.magicNumber != MAGIC_NUMBER) {
      debugD("Invalid or uninitialized RTC memory.");
    } else {
      retVal = (rtcData.rebootFlag  == 1);
    }
  #elif defined(ESP32)
    preferences.begin("reboot_data", false);
    retVal = preferences.getBool("rebootFlag", false);
    preferences.end();
  #endif
  rebootFlag = retVal;
  return retVal;
}

void writeRebootFlag(bool flagValue) {
  rebootFlag = flagValue;
  #if defined(ESP8266)
    // Write MAGIC_NUMBER so we can validate the data when we read it later
    rtcData.magicNumber = MAGIC_NUMBER;
    rtcData.rebootFlag = flagValue?1:0;
    ESP.rtcUserMemoryWrite(0, (uint32_t*)&rtcData, sizeof(rtcData));
  #elif defined(ESP32)
    preferences.begin("reboot_data", false);
    preferences.putBool("rebootFlag", flagValue);
    preferences.end();
  #endif
}

void startWiFiManager(){
  writeRebootFlag(false);
  if (ui.initialised) {
    ui.server->stop();
  }

  WiFiManager wm;
  WiFiManagerParameter custom_mqtt_server("server", "MQTT server", mqttServer.c_str(), 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT port", mqttPort.c_str(), 6);
  WiFiManagerParameter custom_mqtt_username("username", "MQTT Username", mqttUserName.c_str(), 20 );
  WiFiManagerParameter custom_mqtt_password("password", "MQTT Password", mqttPassword.c_str(), 40 );
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_username);
  wm.addParameter(&custom_mqtt_password);
  wm.setBreakAfterConfig(true);
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setConnectTimeout(300); //close the WiFiManager after 300 seconds of inactivity


  wm.startConfigPortal();
  debugI("Exiting Portal");

  if (saveConfig) {
    mqttServer = String(custom_mqtt_server.getValue());
    mqttPort = String(custom_mqtt_port.getValue());
    mqttUserName = String(custom_mqtt_username.getValue());
    mqttPassword = String(custom_mqtt_password.getValue());

    debugI("Updating config file");
    JsonDocument json;
    json["mqtt_server"] = mqttServer;
    json["mqtt_port"] = mqttPort;
    json["mqtt_password"] = mqttPassword;
    json["mqtt_username"] = mqttUserName;

    File configFile = LittleFS.open("/config.json","w");
    if (!configFile) {
      debugE("Failed to open config file for writing");
    } else {
      serializeJson(json, configFile);
      configFile.close();
      debugI("Config file updated");
    }
  }
}

// We check the EN_PIN every loop, to allow people to configure the system
void checkButton(){
#if defined(EN_PIN)
  if(digitalRead(EN_PIN) == LOW) {
    debugI("Initial buttong press detected");
    delay(100); // wait and then test again to ensure that it is a held button not a press
    if(digitalRead(EN_PIN) == LOW) {
      debugI("Button press detected. Starting Portal");
      startWiFiManager();

      ESP.restart();  // restart, dirty but easier than trying to restart services one by one
    }
  }
#endif
}

void checkRebootThreshold(){
  // Check if REBOOT_THRESHOLD seconds have passed since the boot time, then clear the reboot flag
  if (rebootFlag && millis() - bootStartMillis > (REBOOT_THRESHOLD * 1000)) {
    debugI("Clear reboot flag after %i seconds.", REBOOT_THRESHOLD);
    writeRebootFlag(false);
  }
}

bool shouldStartWiFiManager() {
  bool isSelectedRebootReason = false;

  #if defined(ESP8266)
    uint32_t resetReason = ESP.getResetInfoPtr()->reason;
    debugI("ESP8266 Reset Reason: %d", resetReason);

    // Check selected reset reasons for ESP8266
    if (resetReason == REASON_SOFT_RESTART || resetReason == REASON_EXT_SYS_RST) {
      isSelectedRebootReason = true;
    }

  #elif defined(ESP32)
    esp_reset_reason_t resetReason = esp_reset_reason();
    debugI("ESP32 Reset Reason: %d", resetReason);

    // Check selected reset reasons for ESP32
    if (resetReason == ESP_RST_POWERON || resetReason == ESP_RST_EXT || resetReason == ESP_RST_SW) {
      isSelectedRebootReason = true;
    }
  #endif

  return (isSelectedRebootReason && rebootFlag);
}


void climateADPublish(String name, String propertyId, String deviceName, String deviceIdentifier ) {

/*
{ 
   "device_class":"temperature",
   "state_topic":"homeassistant/sensor/sensorBedroom/state",
   "unit_of_measurement":"°C",
   "value_template":"{{ value_json.temperature}}",
   "unique_id":"temp01ae",
   "device":{
      "identifiers":[
         "bedroom01ae"
      ],
      "name":"Bedroom"
   }
}"
*/

  JsonDocument json;
  String uniqueID = spaSerialNumber + "-" + propertyId;

  if (name != "") { json["name"]=name; }

  json["current_temperature_topic"]=mqttStatusTopic;;
  json["current_temperature_template"]="{{ value_json.temperatures.water }}";

  JsonObject device = json["device"].to<JsonObject>();
  device["name"] = deviceName;
  JsonArray identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceIdentifier);

  json["initial"]=36;
  json["max_temp"]=41;
  json["min_temp"]=10;
  JsonArray modes = json["modes"].to<JsonArray>();
  modes.add("auto"); // the actual modes of the heat pump are controlled through a select control to avoid accidently turning off the HP and using the resistance heater
  json["mode_state_template"]="auto";
  json["mode_state_topic"]=mqttStatusTopic;

  json["action_topic"]=mqttStatusTopic;
  json["action_template"]="{% if value_json.status.heatingActive == 'ON' %}heating{% else %}off{% endif %}";
  
  json["temperature_command_topic"]=mqttSet+"/status_temperatureSetPoint";
  json["temperature_state_template"]="{{ value_json.status.temperatureSetPoint }}";
  json["temperature_state_topic"]=mqttStatusTopic;
  json["temperature_unit"]="C";
  json["temp_step"]=0.2;

  json["unique_id"] = uniqueID;

  JsonObject availability = json["availability"].to<JsonObject>();
  availability["topic"] =mqttAvailability;

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/climate/"+ spaSerialNumber + "/" + uniqueID + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}

void fanADPublish(String name, String propertyId, String deviceName, String deviceIdentifier ) {

/*
# Example using percentage based speeds with preset modes configuration.yaml
mqtt:
  - fan:
      name: "Bedroom Fan"
      state_topic: "bedroom_fan/on/state"
      command_topic: "bedroom_fan/on/set"
      direction_state_topic: "bedroom_fan/direction/state"
      direction_command_topic: "bedroom_fan/direction/set"
      oscillation_state_topic: "bedroom_fan/oscillation/state"
      oscillation_command_topic: "bedroom_fan/oscillation/set"
      percentage_state_topic: "bedroom_fan/speed/percentage_state"
      percentage_command_topic: "bedroom_fan/speed/percentage"
      preset_mode_state_topic: "bedroom_fan/preset/preset_mode_state"
      preset_mode_command_topic: "bedroom_fan/preset/preset_mode"
      preset_modes:
        -  "auto"
        -  "smart"
        -  "whoosh"
        -  "eco"
        -  "breeze"
      qos: 0
      payload_on: "true"
      payload_off: "false"
      payload_oscillation_on: "true"
      payload_oscillation_off: "false"
      speed_range_min: 1
      speed_range_max: 10
*/

  JsonDocument json;
  String uniqueID = spaSerialNumber + "-" + propertyId;

  if (name != "") { json["name"]=name; }
  JsonObject device = json["device"].to<JsonObject>();
  device["name"] = deviceName;
  JsonArray identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceIdentifier);


  json["state_topic"] = mqttStatusTopic;
  json["state_value_template"] = "{{ value_json."+propertyId+".state }}";

  json["command_topic"] = mqttSet + "/" + propertyId + "_state";
  
  json["percentage_state_topic"] = mqttStatusTopic;
  json["percentage_command_topic"] = mqttSet + "/" + propertyId + "_speed";
  json["percentage_value_template"] = "{{ value_json."+ propertyId + ".speed }}";
  
  json["preset_mode_state_topic"] = mqttStatusTopic;
  json["preset_mode_command_topic"] = mqttSet + "/" + propertyId + "_mode";
  json["preset_mode_value_template"] = "{{ value_json."+ propertyId + ".mode }}";
  
  JsonArray modes = json["preset_modes"].to<JsonArray>();
  modes.add("Variable");
  modes.add("Ramp");
  json["speed_range_min"]=1;
  json["speed_range_max"]=5;

  json["unique_id"] = uniqueID;

  JsonObject availability = json["availability"].to<JsonObject>();
  availability["topic"] =mqttAvailability;

  String discoveryTopic = "homeassistant/fan/"+ spaSerialNumber + "/" + uniqueID + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);
}


void switchADPublish (String name, String deviceClass, String stateTopic, String valueTemplate, String propertyId, String deviceName, String deviceIdentifier) {
/*
{
   "name":"Irrigation",
   "command_topic":"homeassistant/switch/irrigation/set",
   "state_topic":"homeassistant/switch/irrigation/state",
   "unique_id":"irr01ad",
   "device":{
      "identifiers":[
         "garden01ad"
      ],
      "name":"Garden"
   }
}*/

  JsonDocument json;

  if (deviceClass != "") { json["device_class"] = deviceClass; }
  json["name"]=name;
  json["state_topic"] = stateTopic;
  json["value_template"] = valueTemplate;
  json["command_topic"] = mqttSet + "/" + propertyId;
  json["unique_id"] = spaSerialNumber + "-" + propertyId;
  JsonObject device = json["device"].to<JsonObject>();
  device["name"] = deviceName;
  JsonArray identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceIdentifier);

  JsonObject availability = json["availability"].to<JsonObject>();
  availability["topic"] =mqttAvailability;

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/switch/" + spaSerialNumber + "/" + spaSerialNumber + "-" + propertyId + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}


void selectADPublish (String name, std::vector<String> options, String stateTopic, String valueTemplate, String propertyId, String deviceName, String deviceIdentifier, String category="") {
/*
{
   "name":"Irrigation",
   "command_topic":"homeassistant/switch/irrigation/set",
   "state_topic":"homeassistant/switch/irrigation/state",
   "unique_id":"irr01ad",
   "device":{
      "identifiers":[
         "garden01ad"
      ],
      "name":"Garden"
   }
}*/

  JsonDocument json;

  if (category != "") json["entity_category"] = category;
  json["name"]=name;
  json["state_topic"] = stateTopic;
  json["value_template"] = valueTemplate;
  json["command_topic"] = mqttSet + "/" + propertyId;
  json["unique_id"] = spaSerialNumber + "-" + propertyId;
  JsonObject device = json["device"].to<JsonObject>();
  device["name"] = deviceName;
  JsonArray identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceIdentifier);

  JsonObject availability = json["availability"].to<JsonObject>();
  availability["topic"] =mqttAvailability;

  JsonArray opts = json["options"].to<JsonArray>();
  for (auto o : options) opts.add(o);
  
  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/select/" + spaSerialNumber + "/" + spaSerialNumber + "-" + propertyId + "/config";
  String output = "";
  serializeJson(json,output);
  debugV("json: %s", output.c_str());
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}


void textADPublish (String name, String stateTopic, String valueTemplate, String propertyId, String deviceName, String deviceIdentifier, String category="", String regex="") {
/*{
  "availability": {
      "topic": "sn_esp32/21110001-20000337/available"
  },
  "command_topic": "sn_esp32/21110001-20000337/set/datetime",
  "device": {
      "identifiers": [
          "21110001-20000337"
      ],
      "name": "MySpa"
  },
	"entity_category": "config",
	"pattern": "[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}",
  "name": "Date Time",
  "state_topic": "sn_esp32/21110001-20000337/status",
  "unique_id": "21110001-20000337-datetime",
  "value_template": "{{ value_json.datetime }}"
}*/
  JsonDocument json;

  JsonObject availability = json["availability"].to<JsonObject>();
  availability["topic"] =mqttAvailability;

  json["command_topic"] = mqttSet + "/" + propertyId;

  JsonObject device = json["device"].to<JsonObject>();
  device["name"] = deviceName;
  JsonArray identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceIdentifier);

  if (category != "") json["entity_category"] = category;
  
  if (regex != "") json["pattern"] = regex;

  json["name"]=name;

  json["state_topic"] = stateTopic;

  json["unique_id"] = spaSerialNumber + "-" + propertyId;

  json["value_template"] = valueTemplate;

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/text/" + spaSerialNumber + "/" + spaSerialNumber + "-" + propertyId + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}


void lightADPublish (String name, String deviceClass, String stateTopic, String valueTemplate, String propertyId, String deviceName, String deviceIdentifier) {
/*
{
   "name":"Irrigation",
   "command_topic":"homeassistant/switch/irrigation/set",
   "state_topic":"homeassistant/switch/irrigation/state",
   "unique_id":"irr01ad",
   "device":{
      "identifiers":[
         "garden01ad"
      ],
      "name":"Garden"
   }
}*/

  JsonDocument json;

  if (deviceClass != "") { json["device_class"] = deviceClass; }
  json["name"]=name;
  //json["schema"] = "json";
  json["state_topic"] = stateTopic;
  json["brightness_state_topic"] = stateTopic;
  json["color_mode_state_topic"] = stateTopic;
  json["effect_state_topic"] = stateTopic;
  json["hs_state_topic"] = stateTopic;

  json["command_topic"] = mqttSet + "/" + propertyId + "_state";
  json["brightness_command_topic"] = mqttSet + "/" + propertyId + "_brightness";
  json["effect_command_topic"] = mqttSet + "/" + propertyId + "_effect";
  json["hs_command_topic"] = mqttSet + "/" + propertyId + "_color";

  // Find the last character that is not a space or curly brace
  int lastIndex = valueTemplate.length() - 1;
  while (lastIndex >= 0 && (valueTemplate[lastIndex] == ' ' || valueTemplate[lastIndex] == '}')) {
    lastIndex--;
  }

  // Value templates to extract values from the same topic
  json["state_value_template"] = valueTemplate.substring(0, lastIndex + 1) + ".state" + valueTemplate.substring(lastIndex + 1);
  json["brightness_value_template"] = valueTemplate.substring(0, lastIndex + 1) + ".brightness" + valueTemplate.substring(lastIndex + 1);
  json["effect_value_template"] = valueTemplate.substring(0, lastIndex + 1) + ".effect" + valueTemplate.substring(lastIndex + 1);
  json["hs_value_template"] = valueTemplate.substring(0, lastIndex + 1) + ".color.h" + valueTemplate.substring(lastIndex + 1) + ","
                            + valueTemplate.substring(0, lastIndex + 1) + ".color.s" + valueTemplate.substring(lastIndex + 1);
  json["color_mode_value_template"] = valueTemplate.substring(0, lastIndex + 1) + ".color_mode" + valueTemplate.substring(lastIndex + 1);

  json["unique_id"] = spaSerialNumber + "-" + propertyId;
  JsonObject device = json["device"].to<JsonObject>();
  device["name"] = deviceName;
  JsonArray identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceIdentifier);

  JsonObject availability = json["availability"].to<JsonObject>();
  availability["topic"] =mqttAvailability;

  json["brightness"] = true;
  json["brightness_scale"]=5;
  json["effect"] = true;
  JsonArray effect_list = json["effect_list"].to<JsonArray>();
  for (auto effect: si.colorModeStrings) effect_list.add(effect);
  JsonArray color_modes = json["supported_color_modes"].to<JsonArray>();
  color_modes.add("hs");

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/light/" + spaSerialNumber + "/" + spaSerialNumber + "-" + propertyId + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}

void mqttHaAutoDiscovery() {
  String spaName = "MySpa"; //TODO - This needs to be a settable parameter.


  debugI("Publishing Home Assistant auto discovery");

  String output;
  String discoveryTopic;

  SpaAdConfig spa;
  spa.spaName = spaName;
  spa.spaSerialNumber = spaSerialNumber;
  spa.stateTopic = mqttStatusTopic;
  spa.availabilityTopic = mqttAvailability;

  //sensorADPublish("Water Temperature","","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.water }}","measurement","WaterTemperature", spaName, spaSerialNumber);
  //sensorADPublish("Heater Temperature","diagnostic","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.heater }}","measurement","HeaterTemperature", spaName, spaSerialNumber);
  //sensorADPublish("Case Temperature","diagnostic","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.case }}","measurement","CaseTemperature", spaName, spaSerialNumber);
  //sensorADPublish("Mains Voltage","diagnostic","voltage",mqttStatusTopic,"V","{{ value_json.power.voltage }}","measurement","MainsVoltage", spaName, spaSerialNumber);
  //sensorADPublish("Mains Current","diagnostic","current",mqttStatusTopic,"A","{{ value_json.power.current }}","measurement","MainsCurrent", spaName, spaSerialNumber);
  //sensorADPublish("Power","","energy",mqttStatusTopic,"W","{{ value_json.power.energy }}","measurement","Power", spaName, spaSerialNumber);
  //sensorADPublish("Total Energy","","energy",mqttStatusTopic,"Wh","{{ value_json.totalenergy }}","total_increasing","TotalEnergy", spaName, spaSerialNumber);
  //sensorADPublish("Heatpump Ambient Temperature","","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.heatpumpAmbient }}","measurement","HPAmbTemp", spaName, spaSerialNumber);
  //sensorADPublish("Heatpump Condensor Temperature","","temperature",mqttStatusTopic,"°C","{{ value_json.temperatures.heatpumpCondensor }}","measurement","HPCondTemp", spaName, spaSerialNumber);
  //sensorADPublish("State","","",mqttStatusTopic,"","{{ value_json.status.state }}","","State", spaName, spaSerialNumber);
  spa.commandTopic = mqttSet;
  
  AutoDiscoveryConfig config;

  config.displayName = "Water Temperature";
  config.valueTemplate = "{{ value_json.temperatures.water }}";
  config.propertyId = "WaterTemperature";
  config.deviceClass = "temperature";
  config.entityCategory = "";
  generateSensorAdJSON(output, config, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Case Temperature";
  config.valueTemplate = "{{ value_json.temperatures.case }}";
  config.propertyId = "CaseTemperature";
  config.deviceClass = "temperature";
  config.entityCategory = "diagnostic";
  generateSensorAdJSON(output, config, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Heater Temperature";
  config.valueTemplate = "{{ value_json.temperatures.heater }}";
  config.propertyId = "HeaterTemperature";
  config.deviceClass = "temperature";
  config.entityCategory = "diagnostic";
  generateSensorAdJSON(output, config, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Heatpump Ambient Temperature";
  config.valueTemplate = "{{ value_json.temperatures.heatpumpAmbient }}";
  config.propertyId = "HPAmbTemp";
  config.deviceClass = "temperature";
  config.entityCategory = "diagnostic";
  generateSensorAdJSON(output, config, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Heatpump Condensor Temperature";
  config.valueTemplate = "{{ value_json.temperatures.heatpumpCondensor }}";
  config.propertyId = "HPCondTemp";
  config.deviceClass = "temperature";
  config.entityCategory = "diagnostic";
  generateSensorAdJSON(output, config, spa, discoveryTopic, "measurement", "°C");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Mains Voltage";
  config.valueTemplate = "{{ value_json.power.voltage }}";
  config.propertyId = "MainsVoltage";
  config.deviceClass = "voltage";
  config.entityCategory = "diagnostic";
  generateSensorAdJSON(output, config, spa, discoveryTopic, "measurement", "V");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Mains Current";
  config.valueTemplate = "{{ value_json.power.current }}";
  config.propertyId = "MainsCurrent";
  config.deviceClass = "current";
  config.entityCategory = "diagnostic";
  generateSensorAdJSON(output, config, spa, discoveryTopic, "measurement", "A");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Power";
  config.valueTemplate = "{{ value_json.power.power }}";
  config.propertyId = "Power";
  config.deviceClass = "power";
  config.entityCategory = "diagnostic";
  generateSensorAdJSON(output, config, spa, discoveryTopic, "measurement", "W");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Total Energy";
  config.valueTemplate = "{{ value_json.power.totalEnergy }}";
  config.propertyId = "TotalEnergy";
  config.deviceClass = "energy";
  config.entityCategory = "diagnostic";
  generateSensorAdJSON(output, config, spa, discoveryTopic, "measurement", "kWh");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "State";
  config.valueTemplate = "{{ value_json.status.state }}";
  config.propertyId = "State";
  config.deviceClass = "";
  config.entityCategory = "";
  generateSensorAdJSON(output, config, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //binarySensorADPublish("Heating Active","",mqttStatusTopic,"{{ value_json.status.heatingActive }}","HeatingActive", spaName, spaSerialNumber);
  //binarySensorADPublish("Ozone Active","",mqttStatusTopic,"{{ value_json.status.ozoneActive }}","OzoneActive", spaName, spaSerialNumber);
  config.displayName = "Heating Active";
  config.valueTemplate = "{{ value_json.status.heatingActive }}";
  config.propertyId = "HeatingActive";
  config.deviceClass = "heat";
  config.entityCategory = "";
  generateBinarySensorAdJSON(output, config, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Ozone Active";
  config.valueTemplate = "{{ value_json.status.ozoneActive }}";
  config.propertyId = "OzoneActive";
  config.deviceClass = "running";
  config.entityCategory = "";
  generateBinarySensorAdJSON(output, config, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //climateADPublish(mqttClient, spa, spaName, "{{ value_json.temperatures }}", "Heating");
  config.displayName = spaName;
  config.valueTemplate = "{{ value_json.temperatures }}";
  config.propertyId = "Heating";
  config.deviceClass = "";
  config.entityCategory = "";
  generateClimateAdJSON(output, config, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //selectADPublish(mqttClient, spa, "Heatpump Mode", "{{ value_json.heatpump.mode }}", "heatpump_mode", "", "", {"Auto","Heat","Cool","Off"});
  config.displayName = "Heatpump Mode";
  config.valueTemplate = "{{ value_json.heatpump.mode }}";
  config.propertyId = "heatpump_mode";
  config.deviceClass = "";
  config.entityCategory = "";
  generateSelectAdJSON(output, config, spa, discoveryTopic, {"Auto","Heat","Cool","Off"});
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);


  config.deviceClass = "";
  config.entityCategory = "";
  if (si.getPump1InstallState().startsWith("1") && !(si.getPump1InstallState().endsWith("4"))) {
     //switchADPublish(mqttClient, spa, "Pump 1", "{{ value_json.pumps.pump1.state }}", "pump1");
    config.displayName = "Pump 1";
    config.valueTemplate = "{{ value_json.pumps.pump1.state }}";
    config.propertyId = "pump1";
    generateSwitchAdJSON(output, config, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  if (si.getPump2InstallState().startsWith("1") && !(si.getPump2InstallState().endsWith("4"))) {
    //switchADPublish(mqttClient, spa, "Pump 2","{{ value_json.pumps.pump2.state }}", "pump2");
    config.displayName = "Pump 2";
    config.valueTemplate = "{{ value_json.pumps.pump2.state }}";
    config.propertyId = "pump2";
    generateSwitchAdJSON(output, config, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  if (si.getPump3InstallState().startsWith("1") && !(si.getPump3InstallState().endsWith("4"))) {
    //switchADPublish(mqttClient, spa, "Pump 3", "{{ value_json.pumps.pump3.state }}", "pump3");
    config.displayName = "Pump 3";
    config.valueTemplate = "{{ value_json.pumps.pump3.state }}";
    config.propertyId = "pump3";
    generateSwitchAdJSON(output, config, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  if (si.getPump4InstallState().startsWith("1") && !(si.getPump4InstallState().endsWith("4"))) {
    //switchADPublish(mqttClient, spa, "Pump 4", "{{ value_json.pumps.pump4.state }}", "pump4");
    config.displayName = "Pump 4";
    config.valueTemplate = "{{ value_json.pumps.pump4.state }}";
    config.propertyId = "pump4";
    generateSwitchAdJSON(output, config, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
  }

  if (si.getPump5InstallState().startsWith("1") && !(si.getPump5InstallState().endsWith("4"))) {
    //switchADPublish(mqttClient, spa, "Pump 5", "{{ value_json.pumps.pump5.state }}", "pump5");
    config.displayName = "Pump 5";
    config.valueTemplate = "{{ value_json.pumps.pump5.state }}";
    config.propertyId = "pump5";
    generateSwitchAdJSON(output, config, spa, discoveryTopic);
    mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  }

  //switchADPublish(mqttClient, spa, "Aux Heat Element", "{{ value_json.heatpump.auxheat }}", "heatpump_auxheat");
  config.displayName = "Aux Heat Element";
  config.valueTemplate = "{{ value_json.heatpump.auxheat }}";
  config.propertyId = "heatpump_auxheat";
  config.deviceClass = "";
  config.entityCategory = "";
  generateSwitchAdJSON(output, config, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  std::vector<String> colorModeStrings(std::begin(si.colorModeStrings), std::end(si.colorModeStrings));
  //lightADPublish(mqttClient, spa, "Lights", "{{ value_json.lights }}", "lights", "", "", colorModeStrings);
  config.displayName = "Lights";
  config.valueTemplate = "{{ value_json.lights }}";
  config.propertyId = "lights";
  config.deviceClass = "";
  config.entityCategory = "";
  generateLightAdJSON(output, config, spa, discoveryTopic, colorModeStrings);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //selectADPublish(mqttClient, spa, "Lights Speed","{{ value_json.lights.speed }}","lights_speed", "", "", {"1","2","3","4","5"});
  config.displayName = "Lights Speed";
  config.valueTemplate = "{{ value_json.lights.speed }}";
  config.propertyId = "lights_speed";
  config.deviceClass = "";
  config.entityCategory = "";
  generateSelectAdJSON(output, config, spa, discoveryTopic, {"1","2","3","4","5"});
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  std::vector<String> sleepStrings;
  for (const auto& pair : si.sleepMap) {
      sleepStrings.push_back(pair.first);
  }
  //selectADPublish(mqttClient, spa, "Sleep Timer 1","{{ value_json.sleepTimers.timer1.state }}", "sleepTimers_1_state", "config", "", sleepStrings);
  //selectADPublish(mqttClient, spa, "Sleep Timer 2","{{ value_json.sleepTimers.timer2.state }}", "sleepTimers_2_state", "config", "", sleepStrings);
  config.displayName = "Sleep Timer 1";
  config.valueTemplate = "{{ value_json.sleepTimers.timer1.state }}";
  config.propertyId = "sleepTimers_1_state";
  config.deviceClass = "";
  config.entityCategory = "config";
  generateSelectAdJSON(output, config, spa, discoveryTopic, sleepStrings);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Sleep Timer 2";
  config.valueTemplate = "{{ value_json.sleepTimers.timer2.state }}";
  config.propertyId = "sleepTimers_2_state";
  config.deviceClass = "";
  config.entityCategory = "config";
  generateSelectAdJSON(output, config, spa, discoveryTopic, sleepStrings);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Date Time";
  config.valueTemplate = "{{ value_json.status.datetime }}";
  config.propertyId = "status_datetime";
  config.deviceClass = "";
  config.entityCategory = "config";
  generateTextAdJSON(output, config, spa, discoveryTopic, "[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Sleep Timer 1 Begin";
  config.valueTemplate = "{{ value_json.sleepTimers.timer1.begin }}";
  config.propertyId = "sleepTimers_1_begin";
  config.deviceClass = "";
  config.entityCategory = "config";
  generateTextAdJSON(output, config, spa, discoveryTopic, "[0-2][0-9]:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Sleep Timer 1 End";
  config.valueTemplate = "{{ value_json.sleepTimers.timer1.end }}";
  config.propertyId = "sleepTimers_1_end";
  config.deviceClass = "";
  config.entityCategory = "config";
  generateTextAdJSON(output, config, spa, discoveryTopic, "[0-2][0-9]:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Sleep Timer 2 Begin";
  config.valueTemplate = "{{ value_json.sleepTimers.timer2.begin }}";
  config.propertyId = "sleepTimers_2_begin";
  config.deviceClass = "";
  config.entityCategory = "config";
  generateTextAdJSON(output, config, spa, discoveryTopic, "[0-2][0-9]:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  config.displayName = "Sleep Timer 2 End";
  config.valueTemplate = "{{ value_json.sleepTimers.timer2.end }}";
  config.propertyId = "sleepTimers_2_end";
  config.deviceClass = "";
  config.entityCategory = "config";
  generateTextAdJSON(output, config, spa, discoveryTopic, "[0-2][0-9]:[0-9]{2}");
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  //fanADPublish(mqttClient, spa, "Blower", "{{ value_json.blower }}", "blower");
  config.displayName = "Blower";
  config.valueTemplate = "{{ value_json.blower }}";
  config.propertyId = "blower";
  config.deviceClass = "";
  config.entityCategory = "";
  generateFanAdJSON(output, config, spa, discoveryTopic);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

  std::vector<String> spaModeStrings(std::begin(si.spaModeStrings), std::end(si.spaModeStrings));
  //selectADPublish(mqttClient, spa, "Spa Mode", "{{ value_json.status.spaMode }}", "status_spaMode", "", "", spaModeStrings);
  config.displayName = "Spa Mode";
  config.valueTemplate = "{{ value_json.status.spaMode }}";
  config.propertyId = "status_spaMode";
  config.deviceClass = "";
  config.entityCategory = "";
  generateSelectAdJSON(output, config, spa, discoveryTopic, spaModeStrings);
  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);

}

#pragma region MQTT Publish / Subscribe

void mqttPublishStatusString(String s){

  mqttClient.publish(String(mqttBase+"rfResponse").c_str(),s.c_str());

}

void mqttPublishStatus() {
  String json;
  if (generateStatusJson(si, json)) {
    mqttClient.publish(mqttStatusTopic.c_str(),json.c_str());
  } else {
    debugD("Error generating json");
  }
}


void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);

  String p = "";
  for (uint x = 0; x < length; x++) {
    p += char(*payload);
    payload++;
  }

  debugD("MQTT subscribe received '%s' with payload '%s'",topic,p.c_str());

  String property = t.substring(t.lastIndexOf("/")+1);

  debugI("Received update for %s to %s",property.c_str(),p.c_str());

  if (property == "temperatures_setPoint") {
    si.setSTMP(int(p.toFloat()*10));
  } else if (property == "heatpump_mode") {
    si.setHPMP(p);
  } else if (property == "pump1") {
    si.setRB_TP_Pump1(p=="OFF"?0:1);
  } else if (property == "pump2") {
    si.setRB_TP_Pump2(p=="OFF"?0:1);
  } else if (property == "pump3") {
    si.setRB_TP_Pump3(p=="OFF"?0:1);
  } else if (property == "pump4") {
    si.setRB_TP_Pump4(p=="OFF"?0:1);
  } else if (property == "pump5") {
    si.setRB_TP_Pump5(p=="OFF"?0:1);
  } else if (property == "heatpump_auxheat") {
    si.setHELE(p=="OFF"?0:1);
  } else if (property == "status_datetime") {
    tmElements_t tm;
    tm.Year=CalendarYrToTm(p.substring(0,4).toInt());
    tm.Month=p.substring(5,7).toInt();
    tm.Day=p.substring(8,10).toInt();
    tm.Hour=p.substring(11,13).toInt();
    tm.Minute=p.substring(14,16).toInt();
    tm.Second=p.substring(17).toInt();
    si.setSpaTime(makeTime(tm));
  } else if (property == "lights_state") {
    si.setRB_TP_Light(p=="ON"?1:0);
  } else if (property == "lights_effect") {
    si.setColorMode(p);
  } else if (property == "lights_brightness") {
    si.setLBRTValue(p.toInt());
  } else if (property == "lights_color") {
    int pos = p.indexOf(',');
    if ( pos > 0) {
      int value = p.substring(0, pos).toInt();
      si.setCurrClr(si.colorMap[value/15]);
    }
  } else if (property == "lights_speed") {
    si.setLSPDValue(p);
  } else if (property == "blower_state") {
    si.setOutlet_Blower(p=="OFF"?2:0);
  } else if (property == "blower_speed") {
    if (p=="0") si.setOutlet_Blower(2);
    else si.setVARIValue(p.toInt());
  } else if (property == "blower_mode") {
    si.setOutlet_Blower(p=="Variable"?0:1);
  } else if (property == "sleepTimers_1_state" || property == "sleepTimers_2_state") {
    for (const auto& pair : si.sleepMap) {
      if (pair.first == p) {
        if (property == "sleepTimers_1_state")
          si.setL_1SNZ_DAY(pair.second);
        else if (property == "sleepTimers_2_state")
          si.setL_2SNZ_DAY(pair.second);
        break;
      }
    }
  } else if (property == "sleepTimers_1_begin") {
    si.setL_1SNZ_BGN(convertToInteger(p));
  } else if (property == "sleepTimers_1_end") {
    si.setL_1SNZ_END(convertToInteger(p));
  } else if (property == "sleepTimers_2_begin") {
    si.setL_2SNZ_BGN(convertToInteger(p));
  } else if (property == "sleepTimers_2_end") {
    si.setL_2SNZ_END(convertToInteger(p));
  } else if (property == "status_spaMode") {
    si.setMode(p);
  } else {
    debugE("Unhandled property - %s",property.c_str());
  }
}


#pragma endregion

void setup() {
  #if defined(EN_PIN)
    pinMode(EN_PIN, INPUT_PULLUP);
  #endif

  #if !defined(ESP8266) or defined(ENABLE_SERIAL)
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Debug.setSerialEnabled(true);
  #endif

  delay(200);
  debugA("Starting... %s", WiFi.getHostname());

  WiFi.mode(WIFI_STA); 
  WiFi.begin();

  Debug.begin(WiFi.getHostname());
  Debug.setResetCmdEnabled(true);
  Debug.showProfiler(true);

  debugI("Mounting FS");

  if (!LittleFS.begin()) {
    debugW("Failed to mount file system, formatting");
    LittleFS.format();
    LittleFS.begin();
  }

  debugI("Reading config file");
  File configFile = LittleFS.open("/config.json","r");
  if (configFile) {
    size_t size = configFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);

    JsonDocument json;
    auto deserializeError = deserializeJson(json, buf.get());
    serializeJson(json, Serial);
    if ( ! deserializeError ) {
      debugI("Parsed JSON");
      mqttServer = json["mqtt_server"].as<String>();
      mqttPort = json["mqtt_port"].as<String>();
      mqttUserName = json["mqtt_username"].as<String>();
      mqttPassword = json["mqtt_password"].as<String>();
    } else {
      debugW("Failed to parse config file");
    }
    configFile.close();
  }


  if (mqttServer == "") { 
    mqttServer = "mqtt"; 
  }
  if (mqttPort == "") { 
    mqttPort = "1883"; 
  }

  mqttClient.setServer(mqttServer.c_str(),mqttPort.toInt());
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(2048);

  int valueFlag = readRebootFlag();
  debugI("readRebootFlag: %i", valueFlag);
  bootStartMillis = millis();  // Record the current boot time in milliseconds

  // If rebootFlag is true, the device rebooted within the threshold
  if (shouldStartWiFiManager()) {
    debugI("Detected reboot within the last %i seconds. Starting WiFiManager...", REBOOT_THRESHOLD);
    startWiFiManager();
    ESP.restart();
  } else {
    debugI("Normal boot, no WiFiManager needed.");
    // Set the reboot flag to true
    writeRebootFlag(true);
  }

  ui.begin();

}



void loop() {  



  checkButton();
  checkRebootThreshold();
  #if defined(LED_PIN)
  led.tick();
  #endif
  mqttClient.loop();
  Debug.handle();

  if (ui.initialised) { 
    ui.server->handleClient(); 
  }

  if (WiFi.status() != WL_CONNECTED) {
    //wifi not connected
    #if defined(LED_PIN)
    led.setInterval(100);
    #endif

    if (millis()-wifiLastConnect > 10000) {
      debugI("Wifi reconnecting...");
      wifiLastConnect = millis();
      WiFi.reconnect();
    }
  } else {
    if (delayedStart) {
      delayedStart = !(bootTime + 10000 < millis());
    } else {

      si.loop();

      if (si.isInitialised()) {
        if ( spaSerialNumber=="" ) {
          debugI("Initialising...");
      
          spaSerialNumber = si.getSerialNo1()+"-"+si.getSerialNo2();
          debugI("Spa serial number is %s",spaSerialNumber.c_str());

          mqttBase = String("sn_esp32/") + spaSerialNumber + String("/");
          mqttStatusTopic = mqttBase + "status";
          mqttSet = mqttBase + "set";
          mqttAvailability = mqttBase+"available";
          debugI("MQTT base topic is %s",mqttBase.c_str());
        }
        if (!mqttClient.connected()) {  // MQTT broker reconnect if not connected
          long now=millis();
          if (now - mqttLastConnect > 1000) {
            #if defined(LED_PIN)
            led.setInterval(500);
            #endif
            debugW("MQTT not connected, attempting connection to %s:%s",mqttServer.c_str(),mqttPort.c_str());
            mqttLastConnect = now;


            if (mqttClient.connect("sn_esp32", mqttUserName.c_str(), mqttPassword.c_str(), mqttAvailability.c_str(),2,true,"offline")) {
              debugI("MQTT connected");
    
              String subTopic = mqttBase+"set/#";
              debugI("Subscribing to topic %s", subTopic.c_str());
              mqttClient.subscribe(subTopic.c_str());

              mqttClient.publish(mqttAvailability.c_str(),"online",true);
              autoDiscoveryPublished = false;
            } else {
              debugW("MQTT connection failed");
            }

          }
        } else {
          if (!autoDiscoveryPublished) {  // This is the setup area, gets called once when communication with Spa and MQTT broker have been established.
            debugI("Publish autodiscovery information");
            mqttHaAutoDiscovery();
            autoDiscoveryPublished = true;
            si.setUpdateCallback(mqttPublishStatus);
            mqttPublishStatus();

            si.statusResponse.setCallback(mqttPublishStatusString);

          }
          #if defined(LED_PIN)
          led.setInterval(2000);
          #endif
        }
      }
    }
  }
}