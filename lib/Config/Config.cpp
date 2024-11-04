#include "Config.h"

template<typename T>
void (*Setting<T>::_settingCallback)(const char*, T) = nullptr;
void (*Setting<int>::_settingCallback)(const char*, int) = nullptr;

// Constructor
Config::Config() { }

// Read config from file
bool Config::readConfigFile() {
  debugI("Reading config file");
  File configFile = LittleFS.open("/config.json","r");
  if (!configFile) {
    return false;
  } else {
    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);

    JsonDocument json;
    auto deserializeError = deserializeJson(json, buf.get());
    serializeJson(json, Serial);

    if (!deserializeError) {
      debugI("Parsed JSON");

      if (json["mqtt_server"].is<String>()) MqttServer.setValue(json["mqtt_server"].as<String>());
      if (json["mqtt_port"].is<String>()) MqttPort.setValue(json["mqtt_port"].as<String>());
      if (json["mqtt_username"].is<String>()) MqttUsername.setValue(json["mqtt_username"].as<String>());
      if (json["mqtt_password"].is<String>()) MqttPassword.setValue(json["mqtt_password"].as<String>());
      if (json["spa_name"].is<String>()) SpaName.setValue(json["spa_name"].as<String>());
      if (json["update_frequency"].is<int>()) UpdateFrequency.setValue(json["update_frequency"].as<int>());
    } else {
      debugW("Failed to parse config file");
    }
    configFile.close();
  }

  return true;
}

// Write config to file
void Config::writeConfigFile() {
  debugI("Updating config file");
  JsonDocument json;

  json["mqtt_server"] = MqttServer.getValue();
  json["mqtt_port"] = MqttPort.getValue();
  json["mqtt_username"] = MqttUsername.getValue();
  json["mqtt_password"] = MqttPassword.getValue();
  json["spa_name"] = SpaName.getValue();
  json["update_frequency"] = UpdateFrequency.getValue();

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    debugE("Failed to open config file for writing");
  } else {
    serializeJson(json, configFile);
    configFile.close();
    debugI("Config file updated");
  }
}
