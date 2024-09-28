#include "HAAutoDiscovery.h"

void sensorAdJSON(
    String& output,
    String displayName,
    String spaName,
    String spaSerialNumber,
    String stateTopic,
    String valueTemplate,
    String propertyId,
    String availabilityTopic,
    String deviceClass,
    String entityCategory,
    String stateClass,
    String unitOfMeasure
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

    if (deviceClass != "") json["device_class"] = deviceClass;
    json["state_topic"] = stateTopic;
    if (unitOfMeasure != "") json["unit_of_measurement"] = unitOfMeasure;
    json["value_template"] = valueTemplate;
    json["unique_id"] = spaSerialNumber + "-" + propertyId;
    json["device"]["identifiers"][0] = spaSerialNumber;
    json["device"]["serial_number"] = spaSerialNumber;
    json["device"]["name"] = spaName;
    json["name"]=displayName;
    if (entityCategory != "") json["entity_category"] = entityCategory;
    if (stateClass != "") { json["state_class"] = stateClass; }
    json["availability_topic"] = availabilityTopic;

    serializeJson(json, output);

}

