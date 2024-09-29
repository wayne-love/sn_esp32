#include "HAAutoDiscovery.h"

void sensorAdJSON(
   String& output,
   const SensorAdConfig& config,
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

   //String uniqueID = spaSerialNumber + "-" + propertyId;

   if (config.deviceClass != "") json["device_class"] = config.deviceClass;
   json["state_topic"] = spa.stateTopic;
   if (config.unitOfMeasure != "") json["unit_of_measurement"] = config.unitOfMeasure;
   json["value_template"] = config.valueTemplate;
   json["unique_id"] = spa.spaSerialNumber + "-" + config.propertyId;
   json["device"]["identifiers"][0] = spa.spaSerialNumber;
   json["device"]["serial_number"] = spa.spaSerialNumber;
   json["device"]["name"] = spa.spaName;
   json["name"] = config.displayName;
   if (config.entityCategory != "") json["entity_category"] = config.entityCategory;
   if (config.stateClass != "") { json["state_class"] = config.stateClass; }
   json["availability_topic"] = spa.availabilityTopic;

   discoveryTopic = "homeassistant/sensor/" + spa.spaSerialNumber + "/" + spa.spaSerialNumber + "-" + config.propertyId + "/config";

   serializeJson(json, output);

}

void binarySensorAdJSON(
   String& output, 
   const BinarySensorAdConfig& config, 
   const SpaAdConfig& spa,
   String &discoveryTopic){
/*
{
  "name":null,
  "device_class":"motion",
  "state_topic":"homeassistant/binary_sensor/garden/state",
  "unique_id":"motion01ad",
  "device":{
    "identifiers":[
        "01ad"
    ],
    "name":"Garden"
  }
}*/

   JsonDocument json;

   if (config.deviceClass != "") json["device_class"] = config.deviceClass;
   json["state_topic"] = spa.stateTopic;
   json["value_template"] = config.valueTemplate;
   json["unique_id"] = spa.spaSerialNumber + "-" + config.propertyId;
   json["device"]["identifiers"][0] = spa.spaSerialNumber;
   json["device"]["serial_number"] = spa.spaSerialNumber;
   json["device"]["name"] = spa.spaName;
   json["name"] = config.displayName;
   json["availability_topic"] = spa.availabilityTopic;

   discoveryTopic = "homeassistant/binary_sensor/" + spa.spaSerialNumber + "/" + spa.spaSerialNumber + "-" + config.propertyId + "/config";

   serializeJson(json, output);

}
