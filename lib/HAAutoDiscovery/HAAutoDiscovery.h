#ifndef HAAUTODISCOVERY_H
#define HAAUTODISCOVERY_H

#include <Arduino.h>
#include <ArduinoJson.h>
//#include <variant>
#include <vector>
#include <PubSubClient.h>


/// @brief Configuration structure for the data elements for the Spa.
struct SpaAdConfig {
    String spaName;             // Spa name (eg MySpa)
    String spaSerialNumber;     // Spa serial number
    String stateTopic;          // MQTT topic where staus informaion is published.
    String availabilityTopic;   // MQTT topic for availability of the Spa (not sensor)
    String commandTopic;        // MQTT topic where command information is sent.
};

/// @brief Base configuration structure for common data elements
struct BaseAdConfig {
    String displayName;         // Display name for the sensor
    String valueTemplate;       // Value template to extract the status information
    String propertyId;          // Unique ID of the sensor
    String deviceClass;         // Device class (empty string accepted)
    String entityCategory;      // https://developers.home-assistant.io/blog/2021/10/26/config-entity?_highlight=diagnostic#entity-categories (empty string accepted)
};

/// @brief Configuration structure for sensor autodiscovery - https://www.home-assistant.io/integrations/sensor.mqtt/
struct SensorAdConfig : public BaseAdConfig {
    String stateClass;          // https://developers.home-assistant.io/docs/core/entity/sensor/#long-term-statistics (empty string accepted)
    String unitOfMeasure;       // V, W, A, mV, etc (empty string accepted)
};

/// @brief Configureation struction for binary sensor autodiscovery - https://www.home-assistant.io/integrations/binary_sensor.mqtt/
struct BinarySensorAdConfig : public BaseAdConfig {

};

/// @brief Configuration structure for text autodiscovery - https://www.home-assistant.io/integrations/text.mqtt/
struct TextAdConfig : public BaseAdConfig {
    String regex;               // https://www.home-assistant.io/integrations/text.mqtt/#pattern
};

/// @brief Configuration structure for switch autodiscovery - https://www.home-assistant.io/integrations/switch.mqtt/
struct SwitchAdConfig : public BaseAdConfig {

};

/// @brief Configuration structure for select autodiscovery - https://www.home-assistant.io/integrations/select.mqtt/
struct SelectAdConfig : public BaseAdConfig {
    std::vector<String> options;
};

/// @brief Configuration structure for switch autodiscovery - https://www.home-assistant.io/integrations/fan.mqtt/
struct FanAdConfig : public BaseAdConfig {

};

/// @brief Configuration structure for switch autodiscovery - https://www.home-assistant.io/integrations/light.mqtt/
struct LightAdConfig : public BaseAdConfig {
    std::vector<String> colorModes;
};

/// @brief Configuration structure for switch autodiscovery - https://www.home-assistant.io/integrations/climate.mqtt/
struct ClimateAdConfig : public BaseAdConfig {

};

// Define a variant type that can hold either Sensor or Binary Sensor configuration
//AdConfigVariant = std::variant<SensorAdConfig, BinarySensorAdConfig>;

/// @brief Generate JSON string to publish for Sensor auto discovery
/// @param output String to revceive JSON output
/// @param config Structure to define JSON output
/// @param discoveryTopic String to retun discovrery topic
/// @param type String to provide the type
//void generateAdJSON(String& output, const AdConfigVariant& config, const SpaAdConfig& spa, String &discoveryTopic, String type);
void generateAdJSON(String& output, const BaseAdConfig& config, const SpaAdConfig& spa, String &discoveryTopic, String type);

void fanADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category="", String deviceClass="");
void lightADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category="", String deviceClass="", std::vector<String> colorModes={});
void selectADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category="", String deviceClass="", std::vector<String> options={});
void switchADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category="", String deviceClass="");
void climateADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category="", String deviceClass="");

#endif
