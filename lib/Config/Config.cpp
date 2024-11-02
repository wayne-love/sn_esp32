#include "Config.h"

String mqttServer = "mqtt";
String mqttPort = "1883";
String mqttUserName = "";
String mqttPassword = "";
String spaName = "MySpa";
int updateFrequency = 60;

bool readConfigFile() {
  debugI("Reading config file");
  File configFile = LittleFS.open("/config.json","r");
  if (!configFile) {
    return false;
  } else {
    size_t size = configFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);

    JsonDocument json;
    auto deserializeError = deserializeJson(json, buf.get());
    serializeJson(json, Serial);
    if ( ! deserializeError ) {
      debugI("Parsed JSON");
      if (json["mqtt_server"].is<String>()) {
        mqttServer = json["mqtt_server"].as<String>();
      }
      if (json["mqtt_port"].is<int>()) {
        mqttPort = json["mqtt_port"].as<String>();
      }
      mqttUserName = json["mqtt_username"].as<String>();
      mqttPassword = json["mqtt_password"].as<String>();
      if (json["spa_name"].is<String>()) {
        spaName = json["spa_name"].as<String>();
      }
      if (json["update_frequency"].is<int>()) {
        updateFrequency = json["update_frequency"].as<int>();
      }
    } else {
      debugW("Failed to parse config file");
    }
    configFile.close();
  }

  if (updateFrequency < 10) updateFrequency = 10;
  return true;
}

void writeConfigFile() {
  debugI("Updating config file");
  JsonDocument json;
  json["mqtt_server"] = mqttServer;
  json["mqtt_port"] = mqttPort;
  json["mqtt_password"] = mqttPassword;
  json["mqtt_username"] = mqttUserName;
  json["spa_name"] = spaName;
  json["update_frequency"] = updateFrequency;

  File configFile = LittleFS.open("/config.json","w");
  if (!configFile) {
    debugE("Failed to open config file for writing");
  } else {
    serializeJson(json, configFile);
    configFile.close();
    debugI("Config file updated");
    }
}
