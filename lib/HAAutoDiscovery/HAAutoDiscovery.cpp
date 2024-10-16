#include "HAAutoDiscovery.h"

void generateAdJSON(
   String& output,
   const AdConfigVariant& configVariant,
   const SpaAdConfig& spa,
   String& discoveryTopic
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

   std::visit([&](auto&& config) {
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
      if constexpr (std::is_same_v<std::decay_t<decltype(config)>, SensorAdConfig>) {
      if (!config.unitOfMeasure.isEmpty()) json["unit_of_measurement"] = config.unitOfMeasure;
      if (!config.stateClass.isEmpty()) json["state_class"] = config.stateClass;
      } else if constexpr (std::is_same_v<std::decay_t<decltype(config)>, BinarySensorAdConfig>) {
      } else if constexpr (std::is_same_v<std::decay_t<decltype(config)>, TextAdConfig>) {
         json["command_topic"] = spa.commandTopic + "/" + config.propertyId;
         if (!config.regex.isEmpty()) json["pattern"] = config.regex;
      } else if constexpr (std::is_same_v<std::decay_t<decltype(config)>, SwitchAdConfig>) {
         json["command_topic"] = spa.commandTopic + "/" + config.propertyId;
      } else if constexpr (std::is_same_v<std::decay_t<decltype(config)>, SelectAdConfig>) {
         json["command_topic"] = spa.commandTopic + "/" + config.propertyId;
         JsonArray opts = json["options"].to<JsonArray>();
         for (const auto& o : config.options) opts.add(o);
      } else if constexpr (std::is_same_v<std::decay_t<decltype(config)>, FanAdConfig>) {
         handleFanAdJSON(json, config, spa);
      } else if constexpr (std::is_same_v<std::decay_t<decltype(config)>, LightAdConfig>) {
         handleLightAdJSON (json, config, spa);
      } else if constexpr (std::is_same_v<std::decay_t<decltype(config)>, ClimateAdConfig>) {
         handleClimateAdJSON(json, config, spa);
      }
      String type = config.getType();
      discoveryTopic = "homeassistant/" + type + "/" + spa.spaSerialNumber + "/" + spa.spaSerialNumber + "-" + config.propertyId + "/config";

   }, configVariant);

   serializeJson(json, output);

}

void handleFanAdJSON(JsonDocument& json, const FanAdConfig& config, const SpaAdConfig& spa) {
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
}

void handleLightAdJSON(JsonDocument& json, const LightAdConfig& config, const SpaAdConfig& spa) {
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
   for (const auto& effect: config.colorModes) effect_list.add(effect);
   JsonArray color_modes = json["supported_color_modes"].to<JsonArray>();
   color_modes.add("hs");
}

void handleClimateAdJSON(JsonDocument& json, const ClimateAdConfig& config, const SpaAdConfig& spa) {
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

void fanADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category, String deviceClass) {
  FanAdConfig config;
  String output;
  String discoveryTopic;

  config.displayName = name;
  config.valueTemplate = valueTemplate;
  config.propertyId = propertyId;
  config.entityCategory = category;
  config.deviceClass = deviceClass;
  generateAdJSON(output, config, spa, discoveryTopic);

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
  generateAdJSON(output, config, spa, discoveryTopic);

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
  generateAdJSON(output, config, spa, discoveryTopic);

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
  generateAdJSON(output, config, spa, discoveryTopic);

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
  generateAdJSON(output, config, spa, discoveryTopic);

  mqttClient.publish(discoveryTopic.c_str(), output.c_str(), true);
}
