#include "WebUI.h"

#if defined(ESP8266)
    #define UPDATE_SIZE_UNKNOWN 0XFFFFFFFF
#endif

WebUI::WebUI(SpaInterface *spa) {
    _spa = spa;
}

const char * WebUI::getError() {
    #if defined(ESP8266)
        return Update.getErrorString().c_str();
    #elif defined(ESP32)
        return Update.errorString();
    #endif
}

void WebUI::begin() {
        
    #if defined(ESP8266)
        server.reset(new ESP8266WebServer(80));
    #elif defined(ESP32)
        server.reset(new WebServer(80));
    #endif

    server->on("/", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", WebUI::indexPageTemplate);
    });

    server->on("/json", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        String json;
        SpaInterface &si = *_spa;
        if (generateStatusJson(si, json, true)) {
            server->send(200, "text/json", json.c_str());
        } else {
            server->send(200, "text/text", "Error generating json");
        }
    });

    server->on("/reboot", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->send(200, "text/html", WebUI::rebootPage);
        writeRebootFlag(false);
        debugD("Rebooting...");
        delay(200);
        server->client().stop();
        ESP.restart();
    });

    server->on("/styles.css", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->send(200, "text/css", WebUI::styleSheet);
    });

    server->on("/fota", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", WebUI::fotaPage);
    });

    server->on("/fota", HTTP_POST, [&]() {
        debugD("uri: %s", server->uri().c_str());
        if (Update.hasError()) {
            server->sendHeader("Connection", "close");
            server->send(200, F("text/plain"), String(F("Update error: ")) + String(getError()));
        } else {
            server->client().setNoDelay(true);
            server->sendHeader("Connection", "close");
            server->send(200, "text/plain", "OK");
            debugD("Rebooting...");
            delay(100);
            server->client().stop();
            ESP.restart();
        }
    }, [&]() {
        debugD("uri: %s", server->uri().c_str());
        HTTPUpload& upload = server->upload();
        if (upload.status == UPLOAD_FILE_START) {
            debugD("Update: %s", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                debugD("Update Error: %s",getError());
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            /* flashing firmware to ESP*/
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                debugD("Update Error: %s",getError());
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
                debugD("Update Success: %u\n", upload.totalSize);
            } else {
                debugD("Update Error: %s",getError());
            }
        }
    });

    server->on("/config", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", WebUI::configPageTemplate);
    });

    server->on("/config", HTTP_POST, [&]() {
        debugD("uri: %s", server->uri().c_str());
        if (server->hasArg("spaName")) spaName = server->arg("spaName");
        if (server->hasArg("mqttServer")) mqttServer = server->arg("mqttServer");
        if (server->hasArg("mqttPort")) mqttPort = server->arg("mqttPort");
        if (server->hasArg("mqttUsername")) mqttUserName = server->arg("mqttUsername");
        if (server->hasArg("mqttPassword")) mqttPassword = server->arg("mqttPassword");
        if (server->hasArg("updateFrequency")) updateFrequency = (server->arg("updateFrequency")).toInt();
        writeConfigFile();
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", "Updated");
    });

    server->on("/json/config", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        readConfigFile();
        String configJson = "{";
        configJson += "\"spaName\":\"" + spaName + "\",";
        configJson += "\"mqttServer\":\"" + mqttServer + "\",";
        configJson += "\"mqttPort\":\"" + mqttPort + "\",";
        configJson += "\"mqttUsername\":\"" + mqttUserName + "\",";
        configJson += "\"mqttPassword\":\"" + mqttPassword + "\",";
        configJson += "\"updateFrequency\":" + String(updateFrequency);
        configJson += "}";
        server->send(200, "application/json", configJson);
    });

    server->on("/set", HTTP_POST, [&]() {
        //In theory with minor modification, we can reuse mqttCallback here
        //for (uint8_t i = 0; i < server->args(); i++) updateSpaSetting("set/" + server->argName(0), server->arg(0));
        if (server->hasArg("temperature")) {
            float newTemperature = server->arg("temperature").toFloat();
            SpaInterface &si = *_spa;
            si.setSTMP(int(newTemperature*10));
            server->send(200, "text/plain", "Temperature updated");
        } else {
            server->send(400, "text/plain", "Invalid temperature value");
        }
    });

    server->on("/wifi-manager", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        triggerWiFiManager = true;
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", "WiFi Manager launching, connect to ESP WiFi...");
    });

    server->on("/json.html", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", WebUI::jsonHTMLTemplate);
    });

    server->begin();

    initialised = true;
}