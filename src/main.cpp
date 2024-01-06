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


#include "Blinker.h"

#include "WebUI.h"

#include "SpaNetInterface.h"
#include "autoDiscovery.h"

#define NUM(a) (sizeof(a) / sizeof(*a)) //number of elements in an array

#if defined(ESP8266)
  #define PIN_D0 D0
#elif defined(ESP32)
  #define PIN_D0 0
  const int LED_BUILTIN = 2;
#endif

const int TRIGGER_PIN = PIN_D0;

RemoteDebug Debug;
Blinker led(LED_BUILTIN);
WiFiClient wifi;
PubSubClient mqttClient(wifi);

// **TODO** WebUI ui(&snc);
SpaNetInterface sni;

// End MQTT



bool saveConfig = false;

void saveConfigCallback(){
  saveConfig = true;
}

// We check the button on D0 every loop, to allow people to restart the system 
void checkButton(){
  if(digitalRead(TRIGGER_PIN) == LOW) {
    delay(100);
    if(digitalRead(TRIGGER_PIN) == LOW) {

//      if (ui.initialised) {
//        ui.server->stop();
//      }
      
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

  // Get the serial numbers from the spare and use these as a unique identifier
  String snum = sni.getSerialNo1()+"-"+sni.getSerialNo2();

//  mqtt.baseTopic = "sn_esp32/" + snum + "/"; 

  mqttClient.setServer(mqtt.server.c_str(),mqtt.port.toInt());
//  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(2048);
//  snc.subscribeUpdate(mqttPublishStatus);
}

long mqttLastConnect = 0;
long wifiLastConnect = millis();
long bootTime = millis();
bool autoDiscoveryPublished = false;
bool sncFirstInit = false;

void loop() {  
  checkButton();
  led.tick();

  sni.loop();

/*
  if (!sncFirstInit && snc.initialised()) {
    mqtt.baseTopic = "sn_esp32/" + snc.getSerialNo() + "/";
    sncFirstInit = true;
    ui.begin();
  }

  if (ui.initialised) { 
    ui.server->handleClient(); 
  }
*/

  if (WiFi.status() == WL_CONNECTED) {
    if (sni.isInitialised()){
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
//          snc.forceUpdate();
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