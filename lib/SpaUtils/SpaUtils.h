#ifndef SPAUTILS_H
#define SPAUTILS_H

#include <Arduino.h>
#include <RemoteDebug.h>
#include <ArduinoJson.h>
#include <vector>
#include <time.h>
#include <TimeLib.h>
#include "SpaInterface.h"

extern RemoteDebug Debug;

String convertToTime(int data);
int convertToInteger(String &timeStr);
bool getPumpModes(SpaInterface &si, int pumpNumber, JsonObject pumps);
bool generateStatusJson(SpaInterface &si, String &output, bool prettyJson=false);

#endif