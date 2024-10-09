#include "HAAutoDiscovery.h"

void generateCommonAdJSON(
   JsonDocument& json,
   const AutoDiscoveryConfig& config,
   const SpaAdConfig& spa,
   String& discoveryTopic,
   String type
   ) {
/*
{ 
   "device_class":"temperature",
   "state_topic":"homeassistant/sensor/sensorBedroom/state",
   "unit_of_measurement":"°C",
   "value_template":"{{ value_json.temperature}}",
   "unique_id":"temp01ae",
   "device":{
      "identifiers":[
         "bedroom01ae"
      ],
      "name":"Bedroom"
   }
}
*/

   // Common fields for all types
   json["name"] = config.displayName;
   json["state_topic"] = spa.stateTopic;
   json["value_template"] = config.valueTemplate;
   json["unique_id"] = spa.spaSerialNumber + "-" + config.propertyId;
   json["device"]["identifiers"][0] = spa.spaSerialNumber;
   json["device"]["serial_number"] = spa.spaSerialNumber;
   json["device"]["name"] = spa.spaName;
   json["availability"]["topic"] = spa.availabilityTopic;
   if (!config.deviceClass.isEmpty()) json["device_class"] = config.deviceClass;
   if (!config.entityCategory.isEmpty()) json["entity_category"] = config.entityCategory;

   discoveryTopic = "homeassistant/" + type + "/" + spa.spaSerialNumber + "/" + spa.spaSerialNumber + "-" + config.propertyId + "/config";

}

void generateSensorAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic, String stateClass, String unitOfMeasure) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "sensor");

   if (!unitOfMeasure.isEmpty()) json["unit_of_measurement"] = unitOfMeasure;
   if (!stateClass.isEmpty()) json["state_class"] = stateClass;

   serializeJson(json, output);
}

void generateBinarySensorAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "binary_sensor");

   serializeJson(json, output);
}

void generateTextAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic, String regex) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "text");

   json["command_topic"] = spa.commandTopic + "/" + config.propertyId;
   if (!regex.isEmpty()) json["pattern"] = regex;

   serializeJson(json, output);
}

void generateSwitchAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "switch");

   json["command_topic"] = spa.commandTopic + "/" + config.propertyId;

   serializeJson(json, output);
}

void generateSelectAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic, std::vector<String> options) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "select");

   json["command_topic"] = spa.commandTopic + "/" + config.propertyId;
   JsonArray opts = json["options"].to<JsonArray>();
   for (const auto& o : options) opts.add(o);

   serializeJson(json, output);
}

void generateFanAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "fan");

   // Find the last character that is not a space or curly brace
   int lastIndex = config.valueTemplate.length() - 1;
   while (lastIndex >= 0 && (config.valueTemplate[lastIndex] == ' ' || config.valueTemplate[lastIndex] == '}')) {
      lastIndex--;
   }
   json["state_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".state" + config.valueTemplate.substring(lastIndex + 1);
   json["command_topic"] = spa.commandTopic + "/" + config.propertyId + "_state";

   json["percentage_state_topic"] = spa.stateTopic;
   json["percentage_command_topic"] = spa.commandTopic + "/" + config.propertyId + "_speed";
   json["percentage_value_template"] = "{{ value_json."+ config.propertyId + ".speed }}";

   json["preset_mode_state_topic"] = spa.stateTopic;
   json["preset_mode_command_topic"] = spa.commandTopic + "/" + config.propertyId + "_mode";
   json["preset_mode_value_template"] = "{{ value_json."+ config.propertyId + ".mode }}";

   JsonArray modes = json["preset_modes"].to<JsonArray>();
   modes.add("Variable");
   modes.add("Ramp");
   json["speed_range_min"]=1;
   json["speed_range_max"]=5;

   serializeJson(json, output);
}

void generateLightAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic, std::vector<String> colorModes) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "light");

   json["brightness_state_topic"] = spa.stateTopic;
   json["color_mode_state_topic"] = spa.stateTopic;
   json["effect_state_topic"] = spa.stateTopic;
   json["hs_state_topic"] = spa.stateTopic;

   json["command_topic"] = spa.commandTopic + "/" + config.propertyId + "_state";
   json["brightness_command_topic"] = spa.commandTopic + "/" + config.propertyId + "_brightness";
   json["effect_command_topic"] = spa.commandTopic + "/" + config.propertyId + "_effect";
   json["hs_command_topic"] = spa.commandTopic + "/" + config.propertyId + "_color";

   // Find the last character that is not a space or curly brace
   int lastIndex = config.valueTemplate.length() - 1;
   while (lastIndex >= 0 && (config.valueTemplate[lastIndex] == ' ' || config.valueTemplate[lastIndex] == '}')) {
      lastIndex--;
   }

   // Value templates to extract values from the same topic
   json["state_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".state" + config.valueTemplate.substring(lastIndex + 1);
   json["brightness_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".brightness" + config.valueTemplate.substring(lastIndex + 1);
   json["effect_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".effect" + config.valueTemplate.substring(lastIndex + 1);
   json["hs_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".color.h" + config.valueTemplate.substring(lastIndex + 1) + ","
                              + config.valueTemplate.substring(0, lastIndex + 1) + ".color.s" + config.valueTemplate.substring(lastIndex + 1);
   json["color_mode_value_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".color_mode" + config.valueTemplate.substring(lastIndex + 1);

   json["brightness"] = true;
   json["brightness_scale"]=5;
   json["effect"] = true;
   JsonArray effect_list = json["effect_list"].to<JsonArray>();
   for (const auto& effect: colorModes) effect_list.add(effect);
   JsonArray color_modes = json["supported_color_modes"].to<JsonArray>();
   color_modes.add("hs");

   serializeJson(json, output);
}

void generateClimateAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic) {
   JsonDocument json;
   generateCommonAdJSON(json, config, spa, discoveryTopic, "climate");

   // Find the last character that is not a space or curly brace
   int lastIndex = config.valueTemplate.length() - 1;
   while (lastIndex >= 0 && (config.valueTemplate[lastIndex] == ' ' || config.valueTemplate[lastIndex] == '}')) {
      lastIndex--;
   }

   json["current_temperature_topic"] = spa.stateTopic;
   json["current_temperature_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".water" + config.valueTemplate.substring(lastIndex + 1);

   json["initial"]=36;
   json["max_temp"]=41;
   json["min_temp"]=10;
   JsonArray modes = json["modes"].to<JsonArray>();
   modes.add("auto"); // the actual modes of the heat pump are controlled through a select control to avoid accidently turning off the HP and using the resistance heater
   json["mode_state_template"]="auto";
   json["mode_state_topic"] = spa.stateTopic;

   json["action_topic"] = spa.stateTopic;
   json["action_template"]="{% if value_json.status.heatingActive == 'ON' %}heating{% else %}off{% endif %}";

   json["temperature_command_topic"] = spa.commandTopic + "/temperatures_setPoint";
   json["temperature_state_template"] = config.valueTemplate.substring(0, lastIndex + 1) + ".setPoint" + config.valueTemplate.substring(lastIndex + 1);
   json["temperature_state_topic"] = spa.stateTopic;
   json["temperature_unit"]="C";
   json["temp_step"]=0.2;

   serializeJson(json, output);
}
