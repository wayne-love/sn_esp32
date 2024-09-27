#include "SpaUtils.h"

// Function to convert integer to time in HH:mm format
String convertToTime(int data) {
  // Extract hours and minutes from data
  int hours = (data >> 8) & 0xFF; // High byte for hours
  int minutes = data & 0xFF;      // Low byte for minutes

  // If minutes are greater than or equal to 100, adjust hours and minutes
  if (minutes >= 100) {
    int extraHours = minutes / 100;
    minutes = minutes % 100;
    hours += extraHours;
  }

  String timeStr = String(hours / 10) + String(hours % 10) + ":" +
                   String(minutes / 10) + String(minutes % 10);

  // Print debug information
  debugV("data: %i, timeStr: %s", data, timeStr.c_str());

  return timeStr;
}

int convertToInteger(String &timeStr) {
  int data = -1;

  // Check for an empty string
  if (timeStr.length() == 0) {
    return data;
  }

  // Find the position of the colon
  int colonIndex = timeStr.indexOf(':');
  if (colonIndex == -1) {
    return data; // Invalid format
  }

  // Extract hours and minutes as substrings
  int hours = timeStr.substring(0, colonIndex).toInt();
  int minutes = timeStr.substring(colonIndex + 1).toInt();

  // Validate hours and minutes ranges
  if (hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60) {
    data = (hours * 256) + minutes;
  }

  // Print debug information
  debugV("data: %i, timeStr: %s", data, timeStr.c_str());

  return data;
}

bool generateStatusJson(SpaInterface &si, String &output, bool prettyJson) {
  JsonDocument json;

  json["temperatures"]["water"] = si.getWTMP() / 10.0;
  json["temperatures"]["heater"] = si.getHeaterTemperature() / 10.0;
  json["temperatures"]["case"] = si.getCaseTemperature(); 
  json["temperatures"]["heatpumpAmbient"] = si.getHP_Ambient();
  json["temperatures"]["hpcondtemp"] = si.getHP_Condensor();

  json["power"]["voltage"] = si.getMainsVoltage();
  json["power"]["current"]= si.getMainsCurrent() / 10.0;
  json["power"]["energy"] = si.getPower() / 10.0;
  json["power"]["totalenergy"]= si.getPower_kWh() / 100.0; // convert to kWh to Wh.

  json["status"]["heatingActive"] = si.getRB_TP_Heater()? "ON": "OFF";
  json["status"]["ozoneActive"] = si.getRB_TP_Ozone()? "ON": "OFF";
  json["status"]["temperatureSetPoint"] = si.getSTMP() / 10.0;
  json["status"]["state"] = si.getStatus();
  json["status"]["spaMode"] = si.getMode();

  json["heatpump"]["mode"] = si.HPMPStrings[si.getHPMP()];
  json["heatpump"]["auxheat"] = si.getHELE()==0? "OFF" : "ON";

  json["pumps"]["pump1"] = si.getRB_TP_Pump1()==0? "OFF" : "ON"; // we're ignoring auto here
  json["pumps"]["pump2"] = si.getRB_TP_Pump2()==0? "OFF" : "ON"; // we're ignoring auto here
  json["pumps"]["pump3"] = si.getRB_TP_Pump3()==0? "OFF" : "ON"; // we're ignoring auto here
  json["pumps"]["pump4"] = si.getRB_TP_Pump4()==0? "OFF" : "ON"; // we're ignoring auto here
  json["pumps"]["pump5"] = si.getRB_TP_Pump5()==0? "OFF" : "ON"; // we're ignoring auto here


  String y=String(year(si.getSpaTime()));
  String m=String(month(si.getSpaTime()));
  if (month(si.getSpaTime())<10) m = "0"+m;
  String d=String(day(si.getSpaTime()));
  if (day(si.getSpaTime())<10) d = "0"+d;
  String h=String(hour(si.getSpaTime()));
  if (hour(si.getSpaTime())<10) h = "0"+h;
  String min=String(minute(si.getSpaTime()));
  if (minute(si.getSpaTime())<10) min = "0"+min;
  String s=String(second(si.getSpaTime()));
  if (second(si.getSpaTime())<10) s = "0"+s;

  json["status"]["datetime"]=y+"-"+m+"-"+d+" "+h+":"+min+":"+s;

  json["blower"]["state"] = si.getOutlet_Blower()==2? "OFF" : "ON";
  json["blower"]["mode"] = si.getOutlet_Blower()==1? "Ramp" : "Variable";
  json["blower"]["speed"] = si.getOutlet_Blower() ==2? "0" : String(si.getVARIValue());

  for (uint count = 0; count < sizeof(si.sleepCodeMap); count++){
    if (si.sleepCodeMap[count] == si.getL_1SNZ_DAY())
      json["sleepTimers"]["one"]["state"]=si.sleepStringMap[count];
    if (si.sleepCodeMap[count] == si.getL_2SNZ_DAY())
      json["sleepTimers"]["two"]["state"]=si.sleepStringMap[count];
  }
  json["sleepTimers"]["one"]["begin"]=convertToTime(si.getL_1SNZ_BGN());
  json["sleepTimers"]["one"]["end"]=convertToTime(si.getL_1SNZ_END());
  json["sleepTimers"]["two"]["begin"]=convertToTime(si.getL_2SNZ_BGN());
  json["sleepTimers"]["two"]["end"]=convertToTime(si.getL_2SNZ_END());

  json["lights"]["speed"] = si.getLSPDValue();
  json["lights"]["state"] = si.getRB_TP_Light()? "ON": "OFF";
  json["lights"]["effect"] = si.colorModeStrings[si.getColorMode()];
  json["lights"]["brightness"] = si.getLBRTValue();

  // 0 = white, if white, then set the hue and saturation to white so the light displays correctly in HA.
  if (si.getColorMode() == 0) {
    json["lights"]["color"]["h"] = 0;
    json["lights"]["color"]["s"] = 0;
  } else {
    int hue = 4;
    for (uint count = 0; count < sizeof(si.colorMap); count++){
      if (si.colorMap[count] == si.getCurrClr()) {
        hue = count * 15;
      }
    }
    json["lights"]["color"]["h"] = hue;
    json["lights"]["color"]["s"] = 100;
  }
  json["lights"]["color_mode"] = "hs";

  int jsonSize;
  if (prettyJson) {
    jsonSize = serializeJsonPretty(json, output);
  } else {
    jsonSize = serializeJson(json, output);
  }
  // serializeJson returns the size of the json output. If this is greater than zero we consider this successful
  return (jsonSize > 0);
}

