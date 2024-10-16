#ifndef HAAUTODISCOVERY_H
#define HAAUTODISCOVERY_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <variant>
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
    virtual String getType() const { return "base"; }
};

/// @brief Configuration structure for sensor autodiscovery - https://www.home-assistant.io/integrations/sensor.mqtt/
struct SensorAdConfig : public BaseAdConfig {
    String stateClass;          // https://developers.home-assistant.io/docs/core/entity/sensor/#long-term-statistics (empty string accepted)
    String unitOfMeasure;       // V, W, A, mV, etc (empty string accepted)
    virtual String getType() const { return "sensor"; }
};

/// @brief Configureation struction for binary sensor autodiscovery - https://www.home-assistant.io/integrations/binary_sensor.mqtt/
struct BinarySensorAdConfig : public BaseAdConfig {
    virtual String getType() const { return "binary_sensor"; }
};

/// @brief Configuration structure for text autodiscovery - https://www.home-assistant.io/integrations/text.mqtt/
struct TextAdConfig : public BaseAdConfig {
    String regex;               // https://www.home-assistant.io/integrations/text.mqtt/#pattern
    virtual String getType() const { return "text"; }
};

/// @brief Configuration structure for switch autodiscovery - https://www.home-assistant.io/integrations/switch.mqtt/
struct SwitchAdConfig : public BaseAdConfig {
    virtual String getType() const { return "switch"; }
};

/// @brief Configuration structure for select autodiscovery - https://www.home-assistant.io/integrations/select.mqtt/
struct SelectAdConfig : public BaseAdConfig {
    std::vector<String> options;
    virtual String getType() const { return "select"; }
};

/// @brief Configuration structure for switch autodiscovery - https://www.home-assistant.io/integrations/fan.mqtt/
struct FanAdConfig : public BaseAdConfig {
    virtual String getType() const { return "fan"; }
};

/// @brief Configuration structure for switch autodiscovery - https://www.home-assistant.io/integrations/light.mqtt/
struct LightAdConfig : public BaseAdConfig {
    std::vector<String> colorModes;
    virtual String getType() const { return "light"; }
};

/// @brief Configuration structure for switch autodiscovery - https://www.home-assistant.io/integrations/climate.mqtt/
struct ClimateAdConfig : public BaseAdConfig {
    virtual String getType() const { return "climate"; }
};

// Define a variant type that can hold either Sensor or Binary Sensor configuration
using AdConfigVariant = std::variant<
    SensorAdConfig,
    BinarySensorAdConfig,
    TextAdConfig,
    SwitchAdConfig,
    SelectAdConfig,
    FanAdConfig,
    LightAdConfig,
    ClimateAdConfig
>;

/// @brief Generate JSON string to publish for Sensor auto discovery
/// @param output String to revceive JSON output
/// @param config Structure to define JSON output
/// @param discoveryTopic String to retun discovrery topic
void generateAdJSON(String& output, const AdConfigVariant& configVariant, const SpaAdConfig& spa, String &discoveryTopic);

void handleFanAdJSON(JsonDocument& json, const FanAdConfig& config, const SpaAdConfig& spa);
void handleLightAdJSON(JsonDocument& json, const LightAdConfig& config, const SpaAdConfig& spa);
void handleClimateAdJSON(JsonDocument& json, const ClimateAdConfig& config, const SpaAdConfig& spa);

void fanADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category="", String deviceClass="");
void lightADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category="", String deviceClass="", std::vector<String> colorModes={});
void selectADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category="", String deviceClass="", std::vector<String> options={});
void switchADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category="", String deviceClass="");
void climateADPublish(PubSubClient& mqttClient, SpaAdConfig spa, String name, String valueTemplate, String propertyId, String category="", String deviceClass="");

#endif
