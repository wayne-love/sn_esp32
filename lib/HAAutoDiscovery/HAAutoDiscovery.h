#ifndef HAAUTODISCOVERY_H
#define HAAUTODISCOVERY_H

#include <Arduino.h>
#include <ArduinoJson.h>


/// @brief Generate the JSON to publish a Sensor via MQTT auto discovery - https://www.home-assistant.io/integrations/sensor.mqtt/
/// @param ouput String updated with serialised JSON
/// @param displayName Sensor name
/// @param entityCategory https://developers.home-assistant.io/blog/2021/10/26/config-entity?_highlight=diagnostic#entity-categories (empty string accepted)
/// @param deviceClass Sensor, etc (empty string accepted)
/// @param stateTopic Mqtt topic to read state information from.
/// @param unitOfMeasurement V, W, A, mV, etc (empty string accepted)
/// @param valueTemplate HA value template to parse topic payload to derive value
/// @param stateClass https://developers.home-assistant.io/docs/core/entity/sensor/#long-term-statistics (empty string accepted)
/// @param propertId Unique ID of the sensor, will be concated with deviceName to give a globally unique ID
/// @param deviceName Spa name eg MySpa
/// @param deviceIdentifier Spa serial number eg 123456-789012
void sensorAdJSON(
    String& output,
    String displayName,
    String spaName,
    String spaSerialNumber,
    String stateTopic,
    String valueTemplate,
    String propertyId,
    String availabilityTopic,
    String deviceClass="",
    String entityCategory="",
    String stateClass="",
    String unitOfMeasure=""
    );

#endif  