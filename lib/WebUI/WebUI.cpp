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
        char buffer[1024];
        SpaInterface &si = *_spa;
        float current_temp = float(si.getWTMP()) / 10;
        String status = si.getStatus();
        sprintf(buffer, indexPageTemplate, current_temp, status, __DATE__, __TIME__);
        server->send(200, "text/html", buffer);
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
        server->send(200, "text/html",
            "<!DOCTYPE html>"
            "<html lang=\"en\">"
            "<head>"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
            "<meta name=\"color-scheme\" content=\"dark light\">"
            "<META http-equiv=\"refresh\" content=\"5;URL=/\">"
            "<link rel=\"stylesheet\" href=\"/styles.css\">"
            "<title>Firmware Update</title>"
            "</head>"
            "<body>"
            "Rebooting ESP..."
            "</body>"
            "</html>");
        writeRebootFlag(false);
        delay(200);
        server->client().stop();
        ESP.restart();
    });

    server->on("/styles.css", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->send(200, "text/css", styleSheet);
    });

    server->on("/fota", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", fotaPage);
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
                debugD("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
                debugD("Update Error: %s",getError());
            }
        }
    });

    server->on("/config", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        readConfigFile();
        const size_t bufferSize = strlen(configPageTemplate) + 300;  // Add 300 for variables
        char pageContent[bufferSize];
        snprintf(pageContent, bufferSize, configPageTemplate, spaName.c_str(), mqttServer.c_str(), mqttPort.c_str(), mqttUserName.c_str(), mqttPassword.c_str(), updateFrequency);
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", pageContent);
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

    server->on("/wifi-manager", HTTP_GET, [&]() {
        debugD("uri: %s", server->uri().c_str());
        triggerWiFiManager = true;
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", "WiFi Manager launching, connect to ESP WiFi...");
    });

    server->begin();

    initialised = true;
}