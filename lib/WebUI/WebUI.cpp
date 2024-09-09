#include "WebUI.h"

#if defined(ESP8266)
    #define UPDATE_SIZE_UNKNOWN 0XFFFFFFFF
#endif

WebUI::WebUI(SpaInterface *spa) {
    _spa = spa;
}

void WebUI::begin() {
        
    #if defined(ESP8266)
        server.reset(new ESP8266WebServer(80));
    #elif defined(ESP32)
        server.reset(new WebServer(80));
    #endif

    server->on("/", HTTP_GET, [&]() {
        server->sendHeader("Connection", "close");
        char buffer[1024];
        SpaInterface &si = *_spa;
        float current_temp = (si.getWTMP() / 10);
        String status = si.getStatus();
        sprintf(buffer, indexPageTemplate, current_temp, status);
        server->send(200, "text/html", buffer);
    });

    server->on("/fota", HTTP_GET, [&]() {
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", fotaPage);
    });

    server->on("/update", HTTP_POST, [&]() {
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }, [&]() {
        HTTPUpload& upload = server->upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            /* flashing firmware to ESP*/
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        }
    });

    server->begin();

    initialised = true;
}