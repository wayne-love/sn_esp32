#include "Common.h"

String mqttServer = "mqtt";
String mqttPort = "1883";
String mqttUserName = "";
String mqttPassword = "";
bool rebootFlag = false;
String spaName = "MySpa";
int updateFrequency = 60;
bool triggerWiFiManager = false;

#ifndef DEBUG_ENABLED
    #define DEBUG_ENABLED
    RemoteDebug Debug;
#endif

#if defined(ESP8266)
  RTCData rtcData;
  const int16_t MAGIC_NUMBER = 0xAAAA;
#endif

#if defined(ESP32)
  Preferences preferences;  // For ESP32 reboot storage
#endif

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

void readConfigFile() {
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
