#include "HAAutoDiscovery.h"

void generateAdJSON(
   String& output,
   const BaseAdConfig& config,
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

   JsonDocument json;

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

   // Check the type and add specific fields
   if (type == "sensor") {
     // Cast to SensorAdConfig if additional fields are needed
     const SensorAdConfig* sensorConfig = static_cast<const SensorAdConfig*>(&config);
     if (!sensorConfig->unitOfMeasure.isEmpty()) json["unit_of_measurement"] = sensorConfig->unitOfMeasure;
     if (!sensorConfig->stateClass.isEmpty()) json["state_class"] = sensorConfig->stateClass;
   } else if (type == "binary_sensor") {
      //const BinarySensorAdConfig* sensorConfig = static_cast<const BinarySensorAdConfig*>(&config);
   } else if (type == "text") {
      const TextAdConfig* textConfig = static_cast<const TextAdConfig*>(&config);
      json["command_topic"] = spa.commandTopic + "/" + config.propertyId;
      if (!textConfig->regex.isEmpty()) json["pattern"] = textConfig->regex;
   } else if (type == "switch") {
      //const SwitchAdConfig* sensorConfig = static_cast<const SwitchAdConfig*>(&config);
      json["command_topic"] = spa.commandTopic + "/" + config.propertyId;
   } else if ( type == "select") {
      const SelectAdConfig* selectConfig = static_cast<const SelectAdConfig*>(&config);
      json["command_topic"] = spa.commandTopic + "/" + config.propertyId;
      JsonArray opts = json["options"].to<JsonArray>();
      for (const auto& o : selectConfig->options) opts.add(o);
   } else if ( type == "fan") {
      //const FanAdConfig* fanConfig = static_cast<const FanAdConfig*>(&config);

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
   } else if (type == "light") {
      const LightAdConfig* lightConfig = static_cast<const LightAdConfig*>(&config);
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
      for (const auto& effect: lightConfig->colorModes) effect_list.add(effect);
      JsonArray color_modes = json["supported_color_modes"].to<JsonArray>();
      color_modes.add("hs");
   } else if (type == "climate") {
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
   }
   discoveryTopic = "homeassistant/" + type + "/" + spa.spaSerialNumber + "/" + spa.spaSerialNumber + "-" + config.propertyId + "/config";

   serializeJson(json, output);

}

void fanADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category, String deviceClass) {
  FanAdConfig config;
  String output;
  String discoveryTopic;

  config.displayName = name;
  config.valueTemplate = valueTemplate;
  config.propertyId = propertyId;
  config.entityCategory = category;
  config.deviceClass = deviceClass;
  generateAdJSON(output, config, spa, discoveryTopic, "fan");

  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
}

void lightADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category, String deviceClass, std::vector<String> colorModes) {
  LightAdConfig config;
  String output;
  String discoveryTopic;

  config.displayName = name;
  config.valueTemplate = valueTemplate;
  config.propertyId = propertyId;
  config.entityCategory = category;
  config.deviceClass = deviceClass;
  config.colorModes = colorModes;
  generateAdJSON(output, config, spa, discoveryTopic, "light");

  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
}

void selectADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category, String deviceClass, std::vector<String> options) {
  SelectAdConfig config;
  String output;
  String discoveryTopic;

  config.displayName = name;
  config.valueTemplate = valueTemplate;
  config.propertyId = propertyId;
  config.entityCategory = category;
  config.deviceClass = deviceClass;
  config.options = options;
  generateAdJSON(output, config, spa, discoveryTopic, "select");

  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
}

void switchADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category, String deviceClass) {
  SwitchAdConfig config;
  String output;
  String discoveryTopic;

  config.displayName = name;
  config.valueTemplate = valueTemplate;
  config.propertyId = propertyId;
  config.entityCategory = category;
  config.deviceClass = deviceClass;
  generateAdJSON(output, config, spa, discoveryTopic, "switch");

  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
}

void climateADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category, String deviceClass) {
  ClimateAdConfig config;
  String output;
  String discoveryTopic;

  config.displayName = name;
  config.valueTemplate = valueTemplate;
  config.propertyId = propertyId;
  config.entityCategory = category;
  config.deviceClass = deviceClass;
  generateAdJSON(output, config, spa, discoveryTopic, "climate");

  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
}
