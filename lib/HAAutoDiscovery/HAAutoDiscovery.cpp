#include "HAAutoDiscovery.h"

void sensorAdJSON(
    String& output,
    const SensorAdConfig& config
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
    json["state_topic"] = config.stateTopic;
    if (config.unitOfMeasure != "") json["unit_of_measurement"] = config.unitOfMeasure;
    json["value_template"] = config.valueTemplate;
    json["unique_id"] = config.spaSerialNumber + "-" + config.propertyId;
    json["device"]["identifiers"][0] = config.spaSerialNumber;
    json["device"]["serial_number"] = config.spaSerialNumber;
    json["device"]["name"] = config.spaName;
    json["name"] = config.displayName;
    if (config.entityCategory != "") json["entity_category"] = config.entityCategory;
    if (config.stateClass != "") { json["state_class"] = config.stateClass; }
    json["availability_topic"] = config.availabilityTopic;

    serializeJson(json, output);

}
