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

extern RemoteDebug Debug;

String convertToTime(int data);
int convertToInteger(String &timeStr);
bool getPumpModes(SpaInterface &si, int pumpNumber, JsonObject pumps);
bool generateStatusJson(SpaInterface &si, MQTTClientWrapper &mqttClient, String &output, bool prettyJson=false);

#endif // SPAUTILS_H
