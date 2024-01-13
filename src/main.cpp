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
//#include <FS.h>
#include <ArduinoJson.h>
#include <array>

#include "Blinker.h"

#include "WebUI.h"

#include "SpaNetInterface.h"

class MQTT {
  public:
    String server;
    String port;
    String baseTopic;
};

MQTT mqtt;

#define NUM(a) (sizeof(a) / sizeof(*a)) //number of elements in an array

#if defined(ESP8266)
  #define PIN_D0 D0
#elif defined(ESP32)
  #define PIN_D0 0
  const int LED_BUILTIN = 2;
#endif

const int TRIGGER_PIN = PIN_D0;


#ifndef DEBUG_ENABLED
    #define DEBUG_ENABLED
    RemoteDebug Debug;
#endif

SpaNetInterface sni;

Blinker led(LED_BUILTIN);
WiFiClient wifi;
PubSubClient mqttClient(wifi);

WebUI ui(&sni);



bool saveConfig = false;
long mqttLastConnect = 0;
long wifiLastConnect = millis();
long bootTime = millis();
long statusLastPublish = millis();
bool autoDiscoveryPublished = false;
String mqttBase = "";
String spaSerialNumber = "";

void saveConfigCallback(){
  saveConfig = true;
}

// We check the button on D0 every loop, to allow people to restart the system 
void checkButton(){
  if(digitalRead(TRIGGER_PIN) == LOW) {
    debugI("Initial buttong press detected");
    delay(100); // wait and then test again to ensure that it is a held button not a press
    if(digitalRead(TRIGGER_PIN) == LOW) {
      debugI("Button press detected. Starting Portal");

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

/*
bool parseBool(String val){
  val.toUpperCase();
  if (val=="ON" || val=="TRUE" || val=="1") {
    return true;
  } else {
    return false;
  }
}

bool strEquals(const char* str, const char* expected) {
  return strcmp(str, expected) == 0;
}

void parseLightsJSON(String jString){
  DynamicJsonDocument json(1024);

  deserializeJson(json, jString);

  snc.lights.setIsOn(strEquals(json["state"], "ON"));
  if (json.containsKey("effect")) {
    const char *effect = json["effect"];
    snc.lights.setMode(effect);
  }

  if (json.containsKey("brightness")) {
    byte value = json["brightness"];
    if (value == 255) {
      value = 254;
    }
    value = (value / 51) + 1; // Map from 0 to 254 to 1 to 5
    snc.lights.setBrightness(value);
  }

  if (json.containsKey("color")) {
    int value = json["color"]["h"];
    snc.lights.setColour(SpaNetController::Light::colour_map[value/15]);
  }
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
    sni.setSTMP(int(p.toFloat()*10));
  } else if (property == "mode") {
    sni.setHPMP(p);
  } else {
    debugE("Unhandeled property - %s",property.c_str());
  }


/*
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
  } */ 
}


/*
void mqttPublishStatus(SpaNetController *s) {
  char ON[] = "ON";
  char OFF[] = "OFF";
  char *resp;
  
  String tmpString;

  // This gets called each time the spa does a successful poll
  // it takes as a pearemeter a pointer to the calling instance.

  mqttClient.publish((mqttBase + "voltage/value").c_str(), String(s->getVolts()).c_str());
  mqttClient.publish((mqttBase + "current/value").c_str(), String(s->getAmps()).c_str());
  mqttClient.publish((mqttBase + "hpump_amb_temp/value").c_str(), String(s->getHpumpAmbTemp()).c_str());
  mqttClient.publish((mqttBase + "hpump_con_temp/value").c_str(), String(s->getHpumpConTemp()).c_str());
  mqttClient.publish((mqttBase + "heater_temp/value").c_str(), String(s->getHeaterTemp()).c_str());
  mqttClient.publish((mqttBase + "lights/value").c_str(), buildLightsJSON().c_str());
  mqttClient.publish((mqttBase + "heat_pump_mode/value").c_str(), String(snc.getHeatPumpMode()).c_str());

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
  mqttClient.publish((mqttBase + "heat_pump_mode_txt/value").c_str(), hpModeStr.c_str());
  mqttClient.publish((mqttBase + "water_temp_set_point/value").c_str(), String(snc.getWaterTempSetPoint()).c_str());
  
  if (snc.isAuxHeatingEnabled()) {
    resp = ON;
  } else {
    resp = OFF;
  }

  mqttClient.publish((mqttBase + "resitive_heating/value").c_str(), String(snc.isAuxHeatingEnabled()).c_str());
  mqttClient.publish((mqttBase + "water_temp/value").c_str(), String(s->getWaterTemp()).c_str());
  mqttClient.publish((mqttBase + "heating_active/value").c_str(), String(snc.isHeatingOn()).c_str());
  mqttClient.publish((mqttBase + "uv_ozone_active/value").c_str(), String(snc.isUVOn()).c_str());
  mqttClient.publish((mqttBase + "sanatise_running/value").c_str(), String(snc.isSanatiseRunning()).c_str());
  mqttClient.publish((mqttBase + "status/value").c_str(), snc.getStatus());

  mqttClient.publish((mqttBase + "total_energy/value").c_str(), String(s->getTotalEnergy()).c_str());
  mqttClient.publish((mqttBase + "energy_today/value").c_str(), String(s->getEnergyToday()).c_str());
  mqttClient.publish((mqttBase + "power_consumption/value").c_str(), String(s->getPower()).c_str());
  

  for (int x = 0; x < 5;x++) {
    String pump = "pump" + String(x+1) + "_operating_mode";
    int mode = snc.getPump(x)->getOperatingMode();
    mqttClient.publish((mqttBase + pump + "/value").c_str(), String(mode).c_str());
    mqttClient.publish((mqttBase + pump + "_text/value").c_str(), Pump::pump_modes[mode]);
  }
}
*/

#pragma region Auto Discovery

/// @brief Publish a Sensor via MQTT auto discovery
/// @param deviceClass Sensor, etc
/// @param stateTopic Mqtt topic to read state information from.
/// @param unitOfMeasurement V, W, A, mV, etc
/// @param valueTemplate HA value template to parse topic payload to derive value
/// @param uniqueID Spa serial number + sensor id eg 123456-789012_temperature
/// @param deviceName Spa name eg MySpa
/// @param deviceIdentifier Spa serial number eg 123456-789012
void sensorADPublish(String name, String deviceClass, String stateTopic, String unitOfMeasurement, String valueTemplate, String uniqueID, String deviceName, String deviceIdentifier ) {

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

  json["name"]=name;
  json["device_class"] = deviceClass;
  json["state_topic"] = stateTopic;
  json["unit_of_measurement"] = unitOfMeasurement;
  json["value_template"] = valueTemplate;
  json["unique_id"] = uniqueID;
  JsonObject device = json.createNestedObject("device");
  device["name"] = deviceName;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(deviceIdentifier);

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/sensor/"+uniqueID+"/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}

/// @brief Publish a Binary Sensor via MQTT auto discovery
/// @param deviceClass Sensor, etc
/// @param stateTopic Mqtt topic to read state information from.
/// @param valueTemplate HA value template to parse topic payload to derive value
/// @param uniqueID Spa serial number + sensor id eg 123456-789012_temperature
/// @param deviceName Spa name eg MySpa
/// @param deviceIdentifier Spa serial number eg 123456-789012
void binarySensorADPublish (String name, String deviceClass, String stateTopic, String valueTemplate, String uniqueID, String deviceName, String deviceIdentifier) {
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

  if (deviceClass != "") { json["device_class"] = deviceClass; }
  json["name"]=name;
  json["state_topic"] = stateTopic;
  json["value_template"] = valueTemplate;
  json["unique_id"] = uniqueID;
  JsonObject device = json.createNestedObject("device");
  device["name"] = deviceName;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(deviceIdentifier);

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/binary_sensor/"+spaSerialNumber+"/"+uniqueID+"/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}


void climateADPublish(String name, String uniqueID, String deviceName, String deviceIdentifier ) {

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

  json["name"]=name;

  json["current_temperature_topic"]="sn_esp32/"+spaSerialNumber+"/state";;
  json["current_temperature_template"]="{{ value_json.watertemperature }}";

  JsonObject device = json.createNestedObject("device");
  device["name"] = deviceName;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(deviceIdentifier);
  
  String commandBase = "sn_esp32/"+spaSerialNumber+"/set/";

  json["initial"]=36;
  json["max_temp"]=41;
  json["min_temp"]=10;
  JsonArray modes = json.createNestedArray("modes");
  modes.add("auto");
  modes.add("heat");
  modes.add("cool");
  modes.add("off");
  json["mode_command_topic"]=commandBase+"mode";
  json["mode_state_template"]="{{ value_json.heatermode }}";
  json["mode_state_topic"]="sn_esp32/"+spaSerialNumber+"/state";
  json["temperature_command_topic"]=commandBase+"temperature";
  json["temperature_state_template"]="{{ value_json.temperaturesetpoint }}";
  json["temperature_state_topic"]="sn_esp32/"+spaSerialNumber+"/state";
  json["temperature_unit"]="C";
  json["temp_step"]=0.2;

  json["unique_id"] = uniqueID;
  
  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/climate/"+uniqueID+"/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}


void mqttHaAutoDiscovery() {
  String spaName = "MySpa"; //TODO - This needs to be a settable parameter.


  debugI("Publishing Home Assistant auto discovery");

  sensorADPublish("Water Temperature", "temperature","sn_esp32/"+spaSerialNumber+"/state","°C","{{ value_json.watertemperature }}",spaSerialNumber+"-Temperature", spaName, spaSerialNumber);
  sensorADPublish("Mains Voltage", "voltage","sn_esp32/"+spaSerialNumber+"/state","V","{{ value_json.voltage }}",spaSerialNumber+"-MainsVoltage", spaName, spaSerialNumber);
  sensorADPublish("Mains Current", "current","sn_esp32/"+spaSerialNumber+"/state","A","{{ value_json.current }}",spaSerialNumber+"-MainsCurrent", spaName, spaSerialNumber);
  sensorADPublish("Total Energy","energy","sn_esp32/"+spaSerialNumber+"/state","kWh","{{ value_json.totalenergy }}",spaSerialNumber+"-TotalEnergy", spaName, spaSerialNumber);
  sensorADPublish("Heatpump Ambient Temperature","temperature","sn_esp32/"+spaSerialNumber+"/state","°C","{{ value_json.hpambtemp }}",spaSerialNumber+"-HPAmbTemp", spaName, spaSerialNumber);
  sensorADPublish("Heatpump Condensor Temperature","temperature","sn_esp32/"+spaSerialNumber+"/state","°C","{{ value_json.hpcondtemp }}",spaSerialNumber+"-HPCondTemp", spaName, spaSerialNumber);

  binarySensorADPublish("Heating Active","","sn_esp32/"+spaSerialNumber+"/state","{{ value_json.heatingactive }}",spaSerialNumber+"-HeatingActive", spaName, spaSerialNumber);
  binarySensorADPublish("Ozone Active","","sn_esp32/"+spaSerialNumber+"/state","{{ value_json.ozoneactive }}",spaSerialNumber+"-OzoneActive", spaName, spaSerialNumber);
  
  climateADPublish("Heating",spaSerialNumber+"-Heating", spaName, spaSerialNumber);

  
 /*                             
  String output,topic;

  DynamicJsonDocument haTemplate(1024);

  debugI("Publishing Home Assistant auto discovery");

  JsonObject device = haTemplate.createNestedObject("device");

  device["identifiers"]=spaSerialNumber;
  device["name"]=spaName;

  haTemplate["availability_topic"]=mqttBase+"available";

  mqttSensorADPublish(haTemplate, "voltage", "Supply Voltage", "voltage", "v");
  mqttSensorADPublish(haTemplate, "current", "Supply Current", "current", "A");
  mqttBinarySensorADPublish(haTemplate, "heating_active", "Heating Active", "");
  mqttBinarySensorADPublish(haTemplate, "uv_ozone_active", "UV/Ozone Active", "");
  mqttBinarySensorADPublish(haTemplate, "sanatise_running", "Sanatise Cycle Running", "");
  mqttSensorADPublish(haTemplate, "hpump_amb_temp", "Heatpump Ambient Temperature", "temperature","°C");
  mqttSensorADPublish(haTemplate, "hpump_con_temp", "Heatpump Condensor Temperature", "temperature", "°C");
  mqttSensorADPublish(haTemplate, "water_temp", "Water Temperature", "temperature", "°C");  //Publish this as a sensor as well as HVAC so as to allow eaiser trending
  mqttLightsADPublish(haTemplate, "lights", "Lights");
  mqttClimateADPublish(haTemplate);

  mqttSensorADPublish(haTemplate, "total_energy", "Total Energy", "energy", "total_increasing", "kWh");
  mqttSensorADPublish(haTemplate, "energy_today", "Energy Today", "energy", "total_increasing", "kWh");
  mqttSensorADPublish(haTemplate, "power_consumption", "Power Consumption", "power", "W");
  
  
  mqttSwitchADPublish(haTemplate,"resitive_heating","Aux Resitive Heating");



  for (int x = 0; x < 5; x++) {
    Pump *pump = snc.getPump(x);
    if (pump->isInstalled()) {
      String id = "pump" + String(x+1) + "_operating_mode";
      String name = "Pump " + String(x+1);
      if (!pump->isAutoModeSupported()){
        mqttSwitchADPublish(haTemplate, id, name);  // Pumps should not be published as switches, rather fans, so to support mutispeed pumps.
      } else {
        mqttPumpSelectADPublish(haTemplate, id, name);
      }
    }
  }
*/
}

#pragma endregion


void mqttPublishStatus() {

  StaticJsonDocument<1024> json;

  json["watertemperature"] = String(sni.getWTMP() / 10) + "." + String(sni.getWTMP() % 10); //avoids stupid rounding errors
  json["voltage"]=String(sni.getMainsVoltage());
  json["current"]=String(sni.getMainsCurrent() / 10) + "." + String(sni.getMainsCurrent() % 10);
  json["heatingactive"]=sni.getRB_TP_Heater()? "ON": "OFF";
  json["ozoneactive"]=sni.getRB_TP_Ozone()? "ON": "OFF";
  json["totalenergy"]=String(sni.getPower_kWh() / 100) + "." + String(sni.getPower_kWh() % 100);
  json["hpambtemp"]=String(sni.getHP_Ambient());
  json["hpcondtemp"]=String(sni.getHP_Condensor());
  json["temperaturesetpoint"]=String(sni.getSTMP() / 10) + "." + String(sni.getSTMP() % 10);
  json["heatermode"]=sni.HPMPStrings[sni.getHPMP()];

  String output = "";
  serializeJson(json,output);

  mqttClient.publish(("sn_esp32/"+spaSerialNumber+"/state").c_str(),output.c_str());

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


  if (mqtt.server == "") { 
    mqtt.server = "mqtt"; 
  }
  if (mqtt.port == "") { 
    mqtt.port = "1883"; 
  }

  mqttClient.setServer(mqtt.server.c_str(),mqtt.port.toInt());
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(2048);

  ui.begin();

}



void loop() {  


  sni.loop();
  checkButton();
  led.tick();
  mqttClient.loop();
  
  Debug.handle();

  if (ui.initialised) { 
    ui.server->handleClient(); 
  }

  if (millis()-bootTime<5000) {debugI("waiting...");}

  if (sni.isInitialised() && millis()-bootTime>5000 ) {
    if ( spaSerialNumber=="" ) {
      debugI("Initialising...");
  
      spaSerialNumber = sni.getSerialNo1()+"-"+sni.getSerialNo2();
      debugI("Spa serial number is %s",spaSerialNumber.c_str());

      mqttBase = String("sn_esp32/") + spaSerialNumber + String("/");
      debugI("MQTT base topic is %s",mqttBase.c_str());

    }

    if (WiFi.status() == WL_CONNECTED) {
      if (mqttBase!=""){
        if (!mqttClient.connected()) {  // MQTT broker reconnect if not connected
          long now=millis();
          if (now - mqttLastConnect > 5000) {
            led.setInterval(500);
            debugW("MQTT not connected, attempting connection to %s:%s",mqtt.server.c_str(),mqtt.port.c_str());
            mqttLastConnect = now;

            if (mqttClient.connect("sn_esp32", (mqttBase+"available").c_str(),2,true,"offline")) {
              debugI("MQTT connected");
    
              String subTopic = mqttBase+"set/#";
              debugI("Subscribing to topic %s", subTopic.c_str());
              mqttClient.subscribe(subTopic.c_str());

              mqttClient.publish((mqttBase+"available").c_str(),"online",true);
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
            sni.setUpdateCallback(mqttPublishStatus);
            mqttPublishStatus();
          }
          led.setInterval(2000);

        }
      }
    } else { //wifi not connected
      led.setInterval(100);
      long now = millis();
      if (now-wifiLastConnect > 10000) {
        debugI("Wifi reconnecting...");
        wifiLastConnect = now;
        WiFi.reconnect();
      }
    }
  }

  


}