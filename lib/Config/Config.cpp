#include "Config.h"

template<typename T>
void (*Setting<T>::_settingCallback)(const char*, T) = nullptr;
void (*Setting<int>::_settingCallback)(const char*, int) = nullptr;

Preferences preferences;

// Constructor
Config::Config() { }

// Read configuration
bool Config::readConfig() {
  debugI("Reading config from Preferences or file");

  // Check if Preferences are available
  if (preferences.begin("eSpa-config", true)) {
    debugI("Using Preferences for configuration");

    MqttServer.setValue(preferences.getString("MqttServer", ""));
    MqttPort.setValue(preferences.getInt("MqttPort", 1883));
    MqttUsername.setValue(preferences.getString("MqttUsername", ""));
    MqttPassword.setValue(preferences.getString("MqttPassword", ""));
    SpaName.setValue(preferences.getString("SpaName", "eSpa"));
    UpdateFrequency.setValue(preferences.getInt("spaPollFreq", 60));

    preferences.end();
    return true;
  } else {
    debugI("Preferences not found, checking for config file");
    if (readConfigFile()) {
      writeConfig();
      return true;
    }
  }
  return false;
}

// Write configuration to Preferences
void Config::writeConfig() {
  debugI("Writing configuration to Preferences");
  if (preferences.begin("eSpa-config", false)) {
    preferences.putString("MqttServer", MqttServer.getValue());
    preferences.putInt("MqttPort", MqttPort.getValue());
    preferences.putString("MqttUsername", MqttUsername.getValue());
    preferences.putString("MqttPassword", MqttPassword.getValue());
    preferences.putString("SpaName", SpaName.getValue());
    preferences.putInt("spaPollFreq", UpdateFrequency.getValue());
    preferences.end();
  } else {
    debugE("Failed to open Preferences for writing");
  }
}

// Read config from file and populate settings
bool Config::readConfigFile() {
  debugI("Reading config file");
  if (!LittleFS.begin()) {
    debugW("Failed to mount file system, formatting");
    return false;
  }
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    debugW("Config file not found");
    LittleFS.end();
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
      if (json["mqtt_port"].is<int>()) MqttPort.setValue(json["mqtt_port"].as<int>());
      if (json["mqtt_username"].is<String>()) MqttUsername.setValue(json["mqtt_username"].as<String>());
      if (json["mqtt_password"].is<String>()) MqttPassword.setValue(json["mqtt_password"].as<String>());
      if (json["spa_name"].is<String>()) SpaName.setValue(json["spa_name"].as<String>());
      if (json["update_frequency"].is<int>()) UpdateFrequency.setValue(json["update_frequency"].as<int>());
    } else {
      debugW("Failed to parse config file");
      LittleFS.end();
      return false;
    }
    configFile.close();
  }

  LittleFS.end();
  return true;
}

// Write configuration to file (for backup purposes or debugging)
void Config::writeConfigFile() {
  debugI("Updating config file");
  JsonDocument json;

 if (!LittleFS.begin()) {
    debugW("Failed to mount file system, formatting");
    return;
  }

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
  LittleFS.end();
}
