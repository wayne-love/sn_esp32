#include "WebUI.h"

WebUI::WebUI(SpaNetController *spa) {
    _spa = spa;
}

void WebUI::begin() {
        
    server.reset(new WebServer(80));

    server->on("/", HTTP_GET, [&]() {
        server->sendHeader("Connection", "close");
        char buffer[1024];
        sprintf(buffer, indexPageTemplate, _spa->getWaterTemp(), _spa->getStatus());
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
