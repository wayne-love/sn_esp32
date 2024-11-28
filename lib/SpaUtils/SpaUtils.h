#ifndef SPAUTILS_H
#define SPAUTILS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <RemoteDebug.h>
#include <time.h>
#include <TimeLib.h>
#include "SpaInterface.h"
#include "Config.h"
#include <PubSubClient.h>
#include "MQTTClientWrapper.h"
#include <vector>

extern RemoteDebug Debug;

String convertToTime(int data);
int convertToInteger(String &timeStr);
bool getPumpModesJson(SpaInterface &si, int pumpNumber, JsonObject pumps);

bool getPumpInstalledState(String pumpState);
int getPumpSpeeds(String pumpState);
const std::vector<String> pumpStates = {"OFF", "ON", "LOW", "HIGH", "AUTO"};
std::vector<int> getPumpPossibleStates(String pumpState);
std::vector<String> getPumpPossibleStateStrings(String pumpInstallState);
int getPumpSpeedFromStateString(String state);
int getPumpSpeedMax(String pumpState);
int getPumpSpeedMin(String pumpState);

bool generateStatusJson(SpaInterface &si, MQTTClientWrapper &mqttClient, String &output, bool prettyJson=false);

#endif // SPAUTILS_H
