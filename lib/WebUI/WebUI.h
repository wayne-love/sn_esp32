#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>

#include "ESPAsyncWebServer.h"
#include <Update.h>

#include "SpaInterface.h"
#include "SpaUtils.h"
#include "Config.h"
#include "MQTTClientWrapper.h"

extern RemoteDebug Debug;

class WebUI {
    public:
        WebUI(SpaInterface *spa, Config *config, MQTTClientWrapper *mqttClient);

        /// @brief Set the function to be called when properties have been updated.
        /// @param f
        void setWifiManagerCallback(void (*f)());
        void begin();
        bool initialised = false;

    private:
        AsyncWebServer server{80};
        SpaInterface *_spa;
        Config *_config;
        MQTTClientWrapper *_mqttClient;
        void (*_wifiManagerCallback)() = nullptr;

        const char* getError();
};

#endif // WEBUI_H
