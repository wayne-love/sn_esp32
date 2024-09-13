#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
#endif

#include <WiFiClient.h>
#include <WiFiManager.h>
#include <RemoteDebug.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <FS.h>
#include <ArduinoJson.h>

#include "Blinker.h"
#include "SpaNetController.h"
#include "WebUI.h"

#define NUM(a) (sizeof(a) / sizeof(*a))

#if defined(ESP8266)
  #define PIN_D0 D0
#elif defined(ESP32)
  #define PIN_D0 0
  const int LED_BUILTIN = 2;
#endif

const int TRIGGER_PIN = EN_PIN;


#ifndef DEBUG_ENABLED
    #define DEBUG_ENABLED
    RemoteDebug Debug;
#endif

SpaInterface si;

Blinker led(LED_BUILTIN);
WiFiClient wifi;
PubSubClient mqttClient(wifi);
SpaNetController snc;
WebUI ui(&snc);


bool saveConfig = false;
long mqttLastConnect = 0;
long wifiLastConnect = millis();
long bootTime = millis();
long statusLastPublish = millis();
bool autoDiscoveryPublished = false;

String mqttServer = "";
String mqttPort = "";
String mqttUserName = "";
String mqttPassword = "";

String mqttBase = "";
String mqttStatusTopic = "";
String mqttSet = "";
String mqttAvailability = "";
String mqttLightsTopic = "";

String spaSerialNumber = "";

void saveConfigCallback(){
  saveConfig = true;
}

// We check the button on D0 every loop, to allow people to restart the system 
void checkButton(){
  if(digitalRead(TRIGGER_PIN) == LOW) {
    delay(100);
    if(digitalRead(TRIGGER_PIN) == LOW) {
      if (ui.initialised) {
        ui.server->stop();
      }
      
      WiFiManager wm;
      WiFiManagerParameter custom_mqtt_server("server", "MQTT server", mqtt.server.c_str(), 40);
      WiFiManagerParameter custom_mqtt_port("port", "MQTT port", mqtt.port.c_str(), 6);
      wm.addParameter(&custom_mqtt_server);
      wm.addParameter(&custom_mqtt_port);
      wm.setBreakAfterConfig(true);
      wm.setSaveConfigCallback(saveConfigCallback);
      
      debugI("Button Pressed, Starting Portal");
      wm.startConfigPortal();
      debugI("Exiting Portal");

      if (saveConfig) {
        mqtt.server= String(custom_mqtt_server.getValue());
        mqtt.port= String(custom_mqtt_port.getValue());

        debugI("Updating config file");
        DynamicJsonDocument json(1024);
        json["mqtt_server"] = mqtt.server;
        json["mqtt_port"] = mqtt.port;

        File configFile = LittleFS.open("/config.json","w");
        if (!configFile) {
          debugE("Failed to open config file for writing");
        } else {
          serializeJson(json, configFile);
          configFile.close();
          debugI("Config file updated");
        }
      }

      ESP.restart();  // restart, dirty but easier than trying to restart services one by one
    }
  }
}

bool parseBool(String val){
  val.toUpperCase();
  if (val=="ON" || val=="TRUE" || val=="1") {
    return true;
  } else {
    return false;
  }
}
*/
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);

  String p = "";
  for (int x = 0; x < length; x++) {
    p += char(*payload);
    payload++;
  }

  debugD("MQTT subscribe received '%s' with payload '%s'",topic,p.c_str());

  String property = t.substring(t.lastIndexOf("/")+1);

  debugI("Received update for %s to %s",property.c_str(),p.c_str());

  if (property == "temperature") {
    si.setSTMP(int(p.toFloat()*10));
  } else if (property == "heatpumpmode") {
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
  } else if (property == "auxheat") {
    si.setHELE(p=="OFF"?0:1);
  } else if (property == "datetime") {
    tmElements_t tm;
    tm.Year=CalendarYrToTm(p.substring(0,4).toInt());
    tm.Month=p.substring(5,7).toInt();
    tm.Day=p.substring(8,10).toInt();
    tm.Hour=p.substring(11,13).toInt();
    tm.Minute=p.substring(14,16).toInt();
    tm.Second=p.substring(17).toInt();
    si.setSpaTime(makeTime(tm));
  } else if (property == "lightsspeed") {
    si.setLSPDValue(p);
  } else if (property == "blower") {
    si.setOutlet_Blower(p=="OFF"?2:0);
  } else if (property == "blowerspeed") {
    if (p=="0") si.setOutlet_Blower(2);
    else si.setVARIValue(p.toInt());
  } else if (property == "blowermode") {
    si.setOutlet_Blower(p=="Variable"?0:1);
  } else if (property == "lights") {
    DynamicJsonDocument json(1024);
    deserializeJson(json, p);
    si.setRB_TP_Light(json["state"]=="ON"?1:0);
    if (json.containsKey("effect")) {
      const char *effect = json["effect"];
      si.setColorMode(String(effect));
    }
    if (json.containsKey("brightness")) {
      byte value = json["brightness"];
      if (value == 255) {
        value = 254;
      }
      value = (value / 51) + 1; // Map from 0 to 254 to 1 to 5
      si.setLBRTValue(value);
    }
    if (json.containsKey("color")) {
      int value = json["color"]["h"];
      si.setCurrClr(si.colorMap[value/15]);
    }

String buildLightsJSON() {
  DynamicJsonDocument json(1024);

  if (snc.lights.isOn()) {
    json["state"] = "ON";
  } else {
    json["state"] = "OFF";
  }

  json["effect"] = snc.lights.getMode();
  json["brightness"] = int(snc.lights.getBrightness() * 51);
  int hue;
  for (int count = 0; count < NUM(SpaNetController::Light::colour_map); count++){
    if (SpaNetController::Light::colour_map[count] == snc.lights.getColour()) {
      hue = count * 15;
    }
  }

  json["color"]["h"] = hue;
  json["color"]["s"] = 100;
  json["color_mode"] = "hs";

  String buffer;
  serializeJsonPretty(json, buffer);
  return buffer;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  
  int start = t.lastIndexOf("/", t.lastIndexOf("/") - 1) + 1; //get second last "/"
  String item = t.substring(start, t.lastIndexOf("/"));

  String p = "";
  for (int x = 0; x < length; x++) {
    p = p + char(*payload);
    payload++;
  }

  debugI("Got update for %s - %s", item.c_str(), p.c_str());

  String itemCopy = item;
  itemCopy.remove(4,1);

  if (itemCopy == "pump_operating_mode_text") {
    debugI("pump mode update");
    int pump = item.substring(4, 5).toInt();
    snc.setPumpOperating(pump, p.c_str());
   } else if (itemCopy == "pump_operating_mode") {
    int pump = item.substring(4, 5).toInt();
    snc.setPumpOperating(pump, p.toInt());
  } else if (item == "lights") {
    parseLightsJSON(p);
  } else if (item == "heat_pump_mode") {
    snc.setHeatPumpMode(SpaNetController::heat_pump_modes(p.toInt()));
  } else if (item == "water_temp_set_point") {
    snc.setWaterTempSetPoint(p.toFloat());
  } else if (item == "resitive_heating") {
    snc.setAuxHeatingEnabled(parseBool(p));
  } else if (item == "heat_pump_mode_txt") {
    if (p == "auto") {
      snc.setHeatPumpMode(SpaNetController::heat_pump_modes(SpaNetController::automatic));
    } else if (p == "heat") {
      snc.setHeatPumpMode(SpaNetController::heat_pump_modes(SpaNetController::heat));
    } else if (p == "cool") {
      snc.setHeatPumpMode(SpaNetController::heat_pump_modes(SpaNetController::cool));
    } else if (p == "off") {
      snc.setHeatPumpMode(SpaNetController::heat_pump_modes(SpaNetController::off));
    }
  } 
}

void mqttPublishStatus(SpaNetController *s) {
  char ON[] = "ON";
  char OFF[] = "OFF";
  char *resp;
  
  String tmpString;

  // This gets called each time the spa does a successful poll
  // it takes as a pearemeter a pointer to the calling instance.

  mqttClient.publish((mqtt.baseTopic + "voltage/value").c_str(), String(s->getVolts()).c_str());
  mqttClient.publish((mqtt.baseTopic + "current/value").c_str(), String(s->getAmps()).c_str());
  mqttClient.publish((mqtt.baseTopic + "hpump_amb_temp/value").c_str(), String(s->getHpumpAmbTemp()).c_str());
  mqttClient.publish((mqtt.baseTopic + "hpump_con_temp/value").c_str(), String(s->getHpumpConTemp()).c_str());
  mqttClient.publish((mqtt.baseTopic + "heater_temp/value").c_str(), String(s->getHeaterTemp()).c_str());
  mqttClient.publish((mqtt.baseTopic + "lights/value").c_str(), buildLightsJSON().c_str());
  mqttClient.publish((mqtt.baseTopic + "heat_pump_mode/value").c_str(), String(snc.getHeatPumpMode()).c_str());

  String hpModeStr="unknown";

  switch (snc.getHeatPumpMode()) {
    case SpaNetController::heat_pump_modes(SpaNetController::automatic) :
      hpModeStr = "auto";
      break;
    case SpaNetController::heat_pump_modes(SpaNetController::heat) :
      hpModeStr = "heat";
      break;
    case SpaNetController::heat_pump_modes(SpaNetController::cool) :
      hpModeStr = "cool";
      break;
    case SpaNetController::heat_pump_modes(SpaNetController::off) :
      hpModeStr = "off";
      break;
  }
  mqttClient.publish((mqtt.baseTopic + "heat_pump_mode_txt/value").c_str(), hpModeStr.c_str());
  mqttClient.publish((mqtt.baseTopic + "water_temp_set_point/value").c_str(), String(snc.getWaterTempSetPoint()).c_str());
  
  if (snc.isAuxHeatingEnabled()) {
    resp = ON;
  } else {
    resp = OFF;
  }

  mqttClient.publish((mqtt.baseTopic + "resitive_heating/value").c_str(), String(snc.isAuxHeatingEnabled()).c_str());
  mqttClient.publish((mqtt.baseTopic + "water_temp/value").c_str(), String(s->getWaterTemp()).c_str());
  mqttClient.publish((mqtt.baseTopic + "heating_active/value").c_str(), String(snc.isHeatingOn()).c_str());
  mqttClient.publish((mqtt.baseTopic + "uv_ozone_active/value").c_str(), String(snc.isUVOn()).c_str());
  mqttClient.publish((mqtt.baseTopic + "sanatise_running/value").c_str(), String(snc.isSanatiseRunning()).c_str());
  mqttClient.publish((mqtt.baseTopic + "status/value").c_str(), snc.getStatus());

  mqttClient.publish((mqtt.baseTopic + "total_energy/value").c_str(), String(s->getTotalEnergy()).c_str());
  mqttClient.publish((mqtt.baseTopic + "energy_today/value").c_str(), String(s->getEnergyToday()).c_str());
  mqttClient.publish((mqtt.baseTopic + "power_consumption/value").c_str(), String(s->getPower()).c_str());
  

  for (int x = 0; x < 5;x++) {
    String pump = "pump" + String(x+1) + "_operating_mode";
    int mode = snc.getPump(x)->getOperatingMode();
    mqttClient.publish((mqtt.baseTopic + pump + "/value").c_str(), String(mode).c_str());
    mqttClient.publish((mqtt.baseTopic + pump + "_text/value").c_str(), Pump::pump_modes[mode]);
  }
}
*/

#pragma region Auto Discovery

/// @brief Publish a Sensor via MQTT auto discovery
/// @param name Sensor name
/// @param entityCategory https://developers.home-assistant.io/blog/2021/10/26/config-entity?_highlight=diagnostic#entity-categories (empty string accepted)
/// @param deviceClass Sensor, etc (empty string accepted)
/// @param stateTopic Mqtt topic to read state information from.
/// @param unitOfMeasurement V, W, A, mV, etc (empty string accepted)
/// @param valueTemplate HA value template to parse topic payload to derive value
/// @param stateClass https://developers.home-assistant.io/docs/core/entity/sensor/#long-term-statistics (empty string accepted)
/// @param propertId Unique ID of the property
/// @param deviceName Spa name eg MySpa
/// @param deviceIdentifier Spa serial number eg 123456-789012
void sensorADPublish(String name, String entityCategory, String deviceClass, String stateTopic, String unitOfMeasurement, String valueTemplate, String stateClass, String propertyId, String deviceName, String deviceIdentifier ) {

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

  StaticJsonDocument<512> json;
  String uniqueID = spaSerialNumber + "-" + propertyId;

  json["name"]=name;
  if (entityCategory != "") {json["entity_category"] = entityCategory; }
  if (deviceClass != "") { json["device_class"] = deviceClass; }
  json["state_topic"] = stateTopic;
  if (unitOfMeasurement != "") { json["unit_of_measurement"] = unitOfMeasurement; }
  json["value_template"] = valueTemplate;
  if (stateClass != "") { json["state_class"] = stateClass; }
  json["unique_id"] = uniqueID;
  JsonObject device = json.createNestedObject("device");
  device["name"] = deviceName;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(deviceIdentifier);

  JsonObject availability = json.createNestedObject("availability");
  availability["topic"] =mqttAvailability;

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/sensor/"+ spaSerialNumber + "/" + uniqueID + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}


/// @brief Publish a Binary Sensor via MQTT auto discovery
/// @param name Sensor name
/// @param deviceClass Sensor, etc
/// @param stateTopic Mqtt topic to read state information from.
/// @param valueTemplate HA value template to parse topic payload to derive value
/// @param propertId Unique ID of the property
/// @param deviceName Spa name eg MySpa
/// @param deviceIdentifier Spa serial number eg 123456-789012
void binarySensorADPublish (String name, String deviceClass, String stateTopic, String valueTemplate, String propertyId, String deviceName, String deviceIdentifier) {
/*
{
  "name":null,
  "device_class":"motion",
  "state_topic":"homeassistant/binary_sensor/garden/state",
  "unique_id":"motion01ad",
  "device":{
    "identifiers":[
        "01ad"
    ],
    "name":"Garden"
  }
}*/

  StaticJsonDocument<512> json;
  String uniqueID = spaSerialNumber + "-" + propertyId;

  if (deviceClass != "") { json["device_class"] = deviceClass; }
  json["name"]=name;
  json["state_topic"] = stateTopic;
  json["value_template"] = valueTemplate;
  json["unique_id"] = uniqueID;
  JsonObject device = json.createNestedObject("device");
  device["name"] = deviceName;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(deviceIdentifier);

  JsonObject availability = json.createNestedObject("availability");
  availability["topic"] =mqttAvailability;

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/binary_sensor/"+ spaSerialNumber + "/" + uniqueID + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}

/// @brief Publish a Climate control via MQTT auto discovery
/// @param name Sensor name
/// @param propertId Unique ID of the property
/// @param deviceName Spa name eg MySpa
/// @param deviceIdentifier Spa serial number eg 123456-789012
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

  StaticJsonDocument<1024> json;
  String uniqueID = spaSerialNumber + "-" + propertyId;

  if (name != "") { json["name"]=name; }

  json["current_temperature_topic"]=mqttStatusTopic;;
  json["current_temperature_template"]="{{ value_json.watertemperature }}";

  JsonObject device = json.createNestedObject("device");
  device["name"] = deviceName;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(deviceIdentifier);

  json["initial"]=36;
  json["max_temp"]=41;
  json["min_temp"]=10;

  JsonArray modes = json.createNestedArray("modes");
  modes.add("auto");
//  modes.add("heat");
//  modes.add("cool");
//  modes.add("off");  // this just turns the HP off, not the heating
//  json["mode_command_topic"]=mqttSet+"/mode";
//  json["mode_state_template"]="{{ value_json.heatermode }}";
  json["mode_state_template"]="auto";
  json["mode_state_topic"]=mqttStatusTopic;
  
  json["temperature_command_topic"]=mqttSet+"/temperature";
  json["temperature_state_template"]="{{ value_json.temperaturesetpoint }}";
  json["temperature_state_topic"]=mqttStatusTopic;
  json["temperature_unit"]="C";
  json["temp_step"]=0.2;

  json["unique_id"] = uniqueID;

  JsonObject availability = json.createNestedObject("availability");
  availability["topic"] =mqttAvailability;

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/climate/"+ spaSerialNumber + "/" + uniqueID + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}

/// @brief Publish a fan control via MQTT auto discovery
/// @param name Sensor name
/// @param stateTopic Mqtt topic to read state information from.
/// @param propertId Unique ID of the property
/// @param deviceName Spa name eg MySpa
/// @param deviceIdentifier Spa serial number eg 123456-789012
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

  StaticJsonDocument<1024> json;
  String uniqueID = spaSerialNumber + "-" + propertyId;

  if (name != "") { json["name"]=name; }
  JsonObject device = json.createNestedObject("device");
  device["name"] = deviceName;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(deviceIdentifier);


  json["state_topic"] = mqttStatusTopic;
  json["state_value_template"] = "{{ value_json."+propertyId+" }}";

  json["command_topic"] = mqttSet + "/" + propertyId;
  
  json["percentage_state_topic"] = mqttStatusTopic;
  json["percentage_command_topic"] = mqttSet + "/" + propertyId + "speed";
  json["percentage_value_template"] = "{{ value_json."+ propertyId + "speed }}";
  
  json["preset_mode_state_topic"] = mqttStatusTopic;
  json["preset_mode_command_topic"] = mqttSet + "/" + propertyId + "mode";
  json["preset_mode_value_template"] = "{{ value_json."+ propertyId + "mode }}";
  
  JsonArray modes = json.createNestedArray("preset_modes");
  modes.add("Variable");
  modes.add("Ramp");
  json["speed_range_min"]=1;
  json["speed_range_max"]=5;

  json["unique_id"] = uniqueID;

  JsonObject availability = json.createNestedObject("availability");
  availability["topic"] =mqttAvailability;

  String discoveryTopic = "homeassistant/fan/"+ spaSerialNumber + "/" + uniqueID + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);
}



/// @brief Publish a swtich by MQTT auto discovery
/// @param name Name to display
/// @param deviceClass outlet = power outlet, switch (or "") = generic switch
/// @param stateTopic Mqtt topic to read state information from.
/// @param valueTemplate HA value template to parse topic payload to derive value
/// @param propertyId string appended to spa serial number to create a unique id eg 123456-789012_pump1
/// @param deviceName Name of the spa
/// @param deviceIdentifier identifier of the spa (serial number)
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

  StaticJsonDocument<512> json;

  if (deviceClass != "") { json["device_class"] = deviceClass; }
  json["name"]=name;
  json["state_topic"] = stateTopic;
  json["value_template"] = valueTemplate;
  json["command_topic"] = mqttSet + "/" + propertyId;
  json["unique_id"] = spaSerialNumber + "-" + propertyId;
  JsonObject device = json.createNestedObject("device");
  device["name"] = deviceName;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(deviceIdentifier);

  JsonObject availability = json.createNestedObject("availability");
  availability["topic"] =mqttAvailability;

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/switch/" + spaSerialNumber + "/" + spaSerialNumber + "-" + propertyId + "/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}

void mqttLightsADPublish(DynamicJsonDocument base, String dataPointId, String dataPointName) {
  String spaId = base["device"]["identifiers"];
  base = mqttSensorJson(base, dataPointId, dataPointName);
  base["command_topic"]=mqtt.baseTopic+dataPointId+"/set";
  base["schema"] = "json";
  base["brightness"] = true;
  base["color_mode"] = true;
  base["effect"] = true;
  JsonArray effect_list = base.createNestedArray("effect_list");
  effect_list.add("White");
  effect_list.add("Color");
  effect_list.add("Fade");
  effect_list.add("Step");
  effect_list.add("Party");
  JsonArray color_modes = base.createNestedArray("supported_color_modes");
  color_modes.add("hs");

  String topic = "homeassistant/light/spanet_" + spaId + "/" + dataPointId + "/config";
  String output;
  serializeJsonPretty(base,output);
  mqttClient.publish(topic.c_str(),output.c_str(),true);
}

void mqttPumpSelectADPublish(DynamicJsonDocument base, String dataPointId, String dataPointName) {
  String spaId = base["device"]["identifiers"];
  base["state_topic"] = mqtt.baseTopic + dataPointId + "_text/value";
  base["command_topic"] = mqtt.baseTopic + dataPointId + "_text/set";
  base["name"] = dataPointName;
  base["unique_id"] = "spanet_" + spaId + "_" + dataPointId;
  
  JsonArray options = base.createNestedArray("options");
  for (int i = 0; i < PUMP_MODES_COUNT; i++) {
    if (!strEquals(Pump::pump_modes[i],"")) {
      options.add(Pump::pump_modes[i]);
    }
  }

  String topic = "homeassistant/select/spanet_" + spaId + "/" + dataPointId + "/config";
  String output;
  serializeJsonPretty(base,output);
  mqttClient.publish(topic.c_str(),output.c_str(),true);
}

void mqttClimateADPublish(DynamicJsonDocument base) {
  String spaId = base["device"]["identifiers"];
  base["unique_id"] = "spanet_" + spaId + "_thermostat";
  base["temperature_state_topic"] = mqtt.baseTopic + "water_temp_set_point/value";
  base["temperature_command_topic"] = mqtt.baseTopic + "water_temp_set_point/set";
  base["current_temperature_topic"] = mqtt.baseTopic + "water_temp/value";

  JsonArray modes = base.createNestedArray("modes");
  modes.add("off");
  modes.add("cool");
  modes.add("heat");
  modes.add("auto");
  
  base["mode_state_topic"] = mqtt.baseTopic + "heat_pump_mode_txt/value";
  base["mode_command_topic"] = mqtt.baseTopic + "heat_pump_mode_txt/set";
  base["max_temp"] = 41;
  base["min_temp"] = 5;
  base["precision"] = 0.1;
  base["temp_step"] = 0.5;
  base["name"] = base["device"]["name"];

  String topic = "homeassistant/climate/spanet_" + spaId + "/config";
  String output;
  serializeJsonPretty(base,output);
  mqttClient.publish(topic.c_str(),output.c_str(),true);
}

void mqttHaAutoDiscovery() {
  String spaName = "MySpa";                             //TODO - This needs to be a settable parameter.
  String spaSerialNumber = String(snc.getSerialNo());
  String output,topic;

  DynamicJsonDocument haTemplate(1024);

  debugI("Publishing Home Assistant auto discovery");

  sensorADPublish("Water Temperature","","temperature",mqttStatusTopic,"°C","{{ value_json.watertemperature }}","measurement","Temperature", spaName, spaSerialNumber);
  sensorADPublish("Heater Temperature","diagnostic","temperature",mqttStatusTopic,"°C","{{ value_json.heatertemperature }}","measurement","HeaterTemperature", spaName, spaSerialNumber);
  sensorADPublish("Case Temperature","diagnostic","temperature",mqttStatusTopic,"°C","{{ value_json.casetemperature }}","measurement","WaterTemperature", spaName, spaSerialNumber);
  sensorADPublish("Mains Voltage","diagnostic","voltage",mqttStatusTopic,"V","{{ value_json.voltage }}","measurement","MainsVoltage", spaName, spaSerialNumber);
  sensorADPublish("Mains Current","diagnostic","current",mqttStatusTopic,"A","{{ value_json.current }}","measurement","MainsCurrent", spaName, spaSerialNumber);
  sensorADPublish("Power","","energy",mqttStatusTopic,"W","{{ value_json.power }}","measurement","Power", spaName, spaSerialNumber);
  sensorADPublish("Total Energy","","energy",mqttStatusTopic,"Wh","{{ value_json.totalenergy }}","total_increasing","TotalEnergy", spaName, spaSerialNumber);
  sensorADPublish("Heatpump Ambient Temperature","","temperature",mqttStatusTopic,"°C","{{ value_json.hpambtemp }}","measurement","HPAmbTemp", spaName, spaSerialNumber);
  sensorADPublish("Heatpump Condensor Temperature","","temperature",mqttStatusTopic,"°C","{{ value_json.hpcondtemp }}","measurement","HPCondTemp", spaName, spaSerialNumber);
  sensorADPublish("Status","","",mqttStatusTopic,"","{{ value_json.status }}","","Status", spaName, spaSerialNumber);
  
  binarySensorADPublish("Heating Active","",mqttStatusTopic,"{{ value_json.heatingactive }}","HeatingActive", spaName, spaSerialNumber);
  binarySensorADPublish("Ozone Active","",mqttStatusTopic,"{{ value_json.ozoneactive }}","OzoneActive", spaName, spaSerialNumber);
  
  climateADPublish(spaName,"Heating", spaName, spaSerialNumber);
  selectADPublish("Heatpump Mode",{"Auto","Heat","Cool","Off"}, mqttStatusTopic, "{{ value_json.heatermode }}","heatpumpmode", spaName, spaSerialNumber);

  if (si.getPump1InstallState().startsWith("1") && !(si.getPump1InstallState().endsWith("4"))) {
     switchADPublish("Pump 1","",mqttStatusTopic,"{{ value_json.pump1 }}","pump1",spaName,spaSerialNumber);
  }

  if (si.getPump2InstallState().startsWith("1") && !(si.getPump2InstallState().endsWith("4"))) {
    switchADPublish("Pump 2","",mqttStatusTopic,"{{ value_json.pump2 }}","pump2",spaName,spaSerialNumber);
  } 

  if (si.getPump3InstallState().startsWith("1") && !(si.getPump3InstallState().endsWith("4"))) {
    switchADPublish("Pump 3","",mqttStatusTopic,"{{ value_json.pump3 }}","pump3",spaName,spaSerialNumber);
  } 

  if (si.getPump4InstallState().startsWith("1") && !(si.getPump4InstallState().endsWith("4"))) {
    switchADPublish("Pump 4","",mqttStatusTopic,"{{ value_json.pump4 }}","pump4",spaName,spaSerialNumber);
  } 

  if (si.getPump5InstallState().startsWith("1") && !(si.getPump5InstallState().endsWith("4"))) {
    switchADPublish("Pump 5","",mqttStatusTopic,"{{ value_json.pump5 }}","pump5",spaName,spaSerialNumber);
  }   

  switchADPublish("Aux Heat Element","",mqttStatusTopic,"{{ value_json.auxheat }}","auxheat",spaName,spaSerialNumber);
  lightADPublish("Lights","",mqttLightsTopic,"","lights",spaName,spaSerialNumber);
  selectADPublish("Lights Speed",{"1","2","3","4","5"},mqttStatusTopic,"{{ value_json.lightsspeed }}","lightsspeed",spaName, spaSerialNumber);

  textADPublish("Date Time",mqttStatusTopic,"{{ value_json.datetime }}", "datetime", spaName, spaSerialNumber, "config", "[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}");

  fanADPublish("Blower","blower",spaName, spaSerialNumber);
  
  //switchADPublish("Blower","",mqttState,"{{ value_json.blower }}","blower",spaName, spaSerialNumber);
  //selectADPublish("Blower Mode", {"VariSpeed","Ramp"}, mqttState, "{{ value_json.blowermode }}", "blowermode", spaName, spaSerialNumber);

}

#pragma endregion

#pragma region MQTT Publish

void mqttPublishStatusString(String s){

  mqttClient.publish(String(mqttBase+"rfResponse").c_str(),s.c_str());

}

void mqttPublishStatus() {

  StaticJsonDocument<1024> json;

  json["watertemperature"] = String(si.getWTMP() / 10) + "." + String(si.getWTMP() % 10); //avoids stupid rounding errors
  json["heatertemperature"] = String(si.getHeaterTemperature() / 10) + "." + String(si.getHeaterTemperature() % 10); //avoids stupid rounding errors
  json["casetemperature"] = String(si.getCaseTemperature()); //avoids stupid rounding errors
  json["voltage"]=String(si.getMainsVoltage());
  json["current"]=String(si.getMainsCurrent() / 10) + "." + String(si.getMainsCurrent() % 10);
  json["power"]=String(si.getPower() / 10) + "." + String(si.getPower() % 10);
  json["heatingactive"]=si.getRB_TP_Heater()? "ON": "OFF";
  json["ozoneactive"]=si.getRB_TP_Ozone()? "ON": "OFF";
  json["totalenergy"]=String(si.getPower_kWh() * 10); // convert to kWh to Wh.
  json["hpambtemp"]=String(si.getHP_Ambient());
  json["hpcondtemp"]=String(si.getHP_Condensor());
  json["temperaturesetpoint"]=String(si.getSTMP() / 10) + "." + String(si.getSTMP() % 10);
  json["heatermode"]=si.HPMPStrings[si.getHPMP()];

  json["pump1"]=si.getRB_TP_Pump1()==0? "OFF" : "ON"; // we're ignoring auto here
  json["pump2"]=si.getRB_TP_Pump2()==0? "OFF" : "ON"; // we're ignoring auto here
  json["pump3"]=si.getRB_TP_Pump3()==0? "OFF" : "ON"; // we're ignoring auto here
  json["pump4"]=si.getRB_TP_Pump4()==0? "OFF" : "ON"; // we're ignoring auto here
  json["pump5"]=si.getRB_TP_Pump5()==0? "OFF" : "ON"; // we're ignoring auto here


  String y=String(year(si.getSpaTime()));
  String m=String(month(si.getSpaTime()));
  if (month(si.getSpaTime())<10) m = "0"+m;
  String d=String(day(si.getSpaTime()));
  if (day(si.getSpaTime())<10) d = "0"+d;
  String h=String(hour(si.getSpaTime()));
  if (hour(si.getSpaTime())<10) h = "0"+h;
  String min=String(minute(si.getSpaTime()));
  if (minute(si.getSpaTime())<10) min = "0"+min;
  String s=String(second(si.getSpaTime()));
  if (second(si.getSpaTime())<10) s = "0"+s;

  json["datetime"]=y+"-"+m+"-"+d+" "+h+":"+min+":"+s;

  json["auxheat"]=si.getHELE()==0? "OFF" : "ON";

  json["status"]=si.getStatus();
  json["lightsspeed"]=si.getLSPDValue();

  json["blower"] = si.getOutlet_Blower()==2? "OFF" : "ON";
  json["blowermode"] = si.getOutlet_Blower()==1? "Ramp" : "Variable";
  json["blowerspeed"] = si.getOutlet_Blower() ==2? "0" : String(si.getVARIValue());

  String output = "";
  serializeJson(json,output);

  mqttClient.publish(mqttStatusTopic.c_str(),output.c_str());

  json.clear();
  json["state"]=si.getRB_TP_Light()? "ON": "OFF";

  json["effect"] = si.colorModeStrings[si.getColorMode()];
  json["brightness"] = int(si.getLBRTValue() * 51);

  // 0 = white, if white, then set the hue and saturation to white so the light displays correctly in HA.
  if (si.getColorMode() == 0) {
    json["color"]["h"] = 0;
    json["color"]["s"] = 0;
  } else {
    int hue = 4;
    for (int count = 0; count < sizeof(si.colorMap); count++){
      if (si.colorMap[count] == si.getCurrClr()) {
        hue = count * 15;
      }
    }
  }
}

void setup() {
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  delay(200);

  WiFi.mode(WIFI_STA); 
  WiFi.begin();

  Debug.begin(WiFi.getHostname());
  Debug.setResetCmdEnabled(true);
  Debug.showProfiler(true);
  Debug.setSerialEnabled(true);

  debugA("Starting...");

  debugI("Mounting FS");
  if (LittleFS.begin()){
    debugI("Mounted FS");
    debugI("Reading config file");
    File configFile = LittleFS.open("/config.json","r");
    if (configFile) {
      debugI("Reading config file");
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);

      DynamicJsonDocument json(1024);
      auto deserializeError = deserializeJson(json, buf.get());
      serializeJson(json, Serial);
      if ( ! deserializeError ) {
        debugI("Parsed JSON");
        mqtt.server = json["mqtt_server"].as<String>();
        mqtt.port = json["mqtt_port"].as<String>();
      } else {
        debugW("Failed to parse config file");
      }
      configFile.close();
    }
  } else {
    debugW("Failed to mount file system");
  }
  
  if (mqtt.server == "") { 
    mqtt.server = "mqtt"; 
  }
  if (mqtt.port == "") { 
    mqtt.port = "1883"; 
  }

  String snum = String(snc.getSerialNo());

  mqtt.baseTopic = "sn_esp32/" + snum + "/"; 

  mqttClient.setServer(mqtt.server.c_str(),mqtt.port.toInt());
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(2048);
  snc.subscribeUpdate(mqttPublishStatus);
}

long mqttLastConnect = 0;
long wifiLastConnect = millis();
long bootTime = millis();
bool autoDiscoveryPublished = false;
bool sncFirstInit = false;

void loop() {  
  checkButton();
  led.tick();
  mqttClient.loop();
  Debug.handle();

  if (!sncFirstInit && snc.initialised()) {
    mqtt.baseTopic = "sn_esp32/" + snc.getSerialNo() + "/";
    sncFirstInit = true;
    ui.begin();
  }

  if (ui.initialised) { 
    ui.server->handleClient(); 
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (snc.initialised()){
      if (!mqttClient.connected()) {
        long now=millis();
        if (now - mqttLastConnect > 5000) {
          led.setInterval(500);
          debugW("MQTT not connected, attempting connection to %s:%s",mqtt.server.c_str(),mqtt.port.c_str());
          mqttLastConnect = now;
          if (mqttClient.connect("sn_esp32", (mqtt.baseTopic+"available").c_str(),2,true,"offline")) {
            debugI("MQTT connected");
            mqttClient.subscribe((mqtt.baseTopic+"+/set").c_str());
            mqttClient.publish((mqtt.baseTopic+"available").c_str(),"online",true);
            autoDiscoveryPublished = false;
          } else {
            debugW("MQTT connection failed");
          }
        }
      } else {
        if (!autoDiscoveryPublished) {
          mqttHaAutoDiscovery();
          autoDiscoveryPublished = true;
          snc.forceUpdate();
        }
        led.setInterval(2000);
      }
    }
  } else {
    led.setInterval(100);
    long now = millis();
    if (now-wifiLastConnect > 10000) {
      wifiLastConnect = now;
      WiFi.reconnect();
    }
  }

  Debug.handle();
  mqttClient.loop();

  delay(1);
}