#ifndef HAAUTODISCOVERY_H
#define HAAUTODISCOVERY_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>


/// @brief Configuration structure for the data elements for the Spa.
struct SpaAdConfig {
    String spaName;             // Spa name (eg MySpa)
    String spaSerialNumber;     // Spa serial number
    String stateTopic;          // MQTT topic where staus informaion is published.
    String availabilityTopic;   // MQTT topic for availability of the Spa (not sensor)
    String commandTopic;        // MQTT topic where command information is sent.
};

/// @brief Base configuration structure for common data elements
struct AutoDiscoveryConfig {
    String displayName;         // Display name for the sensor
    String valueTemplate;       // Value template to extract the status information
    String propertyId;          // Unique ID of the sensor
    String deviceClass;         // Device class (empty string accepted)
    String entityCategory;      // https://developers.home-assistant.io/blog/2021/10/26/config-entity?_highlight=diagnostic#entity-categories (empty string accepted)
};

/// @brief Generate JSON string to publish for Sensor auto discovery
/// @param output String to revceive JSON output
/// @param config Structure to define JSON output
/// @param discoveryTopic String to retun discovrery topic
/// @param type String to provide the type
void generateCommonAdJSON(JsonDocument& json, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic, String type);

void generateSensorAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic, String stateClass="", String unitOfMeasure="");
void generateBinarySensorAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic);
void generateTextAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic, String regex="");
void generateSwitchAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic);
void generateSelectAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic, std::vector<String> options={});
void generateFanAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic);
void generateLightAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic, std::vector<String> colorModes={});
void generateClimateAdJSON(String& output, const AutoDiscoveryConfig& config, const SpaAdConfig& spa, String &discoveryTopic);

/*
struct SensorAdConfig {
    String stateClass;          // https://developers.home-assistant.io/docs/core/entity/sensor/#long-term-statistics (empty string accepted)
    String unitOfMeasure;       // V, W, A, mV, etc (empty string accepted)
};*/

#endif
