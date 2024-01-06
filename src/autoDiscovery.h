#ifndef AUTODISCOVERY_H
#define AUTODISCOVERY_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <RemoteDebug.h>
#include "SpaNetInterface.h"

#ifndef DEBUG_
#define DEBUG_
RemoteDebug Debug;
#endif

extern PubSubClient mqttClient;
extern SpaNetInterface sni;

// MQTT Interface

class MQTT {
  public:
    String server;
    String port;
    String baseTopic;
};

MQTT mqtt;

void mqttHaAutoDiscovery();



#endif