#ifndef CONFIG_H
#define CONFIG_H


#include <Arduino.h>
#include <ArduinoJson.h>
#include <RemoteDebug.h>
#include <LittleFS.h>

extern String mqttServer;
extern String mqttPort;
extern String mqttUserName;
extern String mqttPassword;
extern bool rebootFlag;
extern String spaName;
extern int updateFrequency;
extern bool triggerWiFiManager;

#ifndef DEBUG_ENABLED
    extern RemoteDebug Debug;
#endif

#if defined(ESP32)
#include <Preferences.h>
#endif

#if defined(ESP8266)
  struct RTCData {
    int16_t magicNumber;  // Use a unique number to identify valid data
    int16_t rebootFlag;   // 1 start wifi manager on next boot
  };
  extern RTCData rtcData;
  extern const int16_t MAGIC_NUMBER;
#endif

#if defined(ESP32)
  extern Preferences preferences;  // For ESP32 reboot storage
#endif

bool readRebootFlag();
void writeRebootFlag(bool flagValue);
void readConfigFile();
void writeConfigFile();

#endif // CONFIG_H
