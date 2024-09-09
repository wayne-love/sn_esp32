#ifndef WEBUI_H
#define WEBUI_H

#if defined(ESP8266)
  #include <ESP8266WebServer.h>
#elif defined(ESP32)
  #include <WebServer.h>
  #include <Update.h>
#endif

#include "SpaInterface.h"

class WebUI {
    public:
        #if defined(ESP8266)
            std::unique_ptr<ESP8266WebServer> server;
        #elif defined(ESP32)
            std::unique_ptr<WebServer> server;
        #endif
        
        char indexPage[1024] = "\0";

        WebUI(SpaInterface *spa);
        void begin();
        bool initialised = false;

    private:
        char buffer[1024];
        SpaInterface *_spa;
        const char *indexPageTemplate =
            "<!DOCTYPE html>"
            "<html>"
            "<meta http-equiv=\"refresh\" content=\"10\">"
            "<body>"
            "<h1>ESC32 Spa Controller</h1>"
            "<p>Spa temperature is - %.1f</p>"
            "<p>Spa status is - %s</p>"
            "</body>"
            "</html>";

        const char *fotaPage =
            "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
            "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
            "<input type='file' name='update'>"
            "<input type='submit' value='Update'>"
            "</form>"
            "<div id='prg'>progress: 0%</div>"
            "<script>"
            "$('form').submit(function(e){"
            "e.preventDefault();"
            "var form = $('#upload_form')[0];"
            "var data = new FormData(form);"
            " $.ajax({"
            "url: '/update',"
            "type: 'POST',"
            "data: data,"
            "contentType: false,"
            "processData:false,"
            "xhr: function() {"
            "var xhr = new window.XMLHttpRequest();"
            "xhr.upload.addEventListener('progress', function(evt) {"
            "if (evt.lengthComputable) {"
            "var per = evt.loaded / evt.total;"
            "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
            "}"
            "}, false);"
            "return xhr;"
            "},"
            "success:function(d, s) {"
            "console.log('success!')"
            "},"
            "error: function (a, b, c) {"
            "}"
            "});"
            "});"
            "</script>";

};

#endif