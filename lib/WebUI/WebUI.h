#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WebServer.h>
#elif defined(ESP32)
  #include <WebServer.h>
  #include <Update.h>
#endif

#include "SpaInterface.h"
#include "SpaUtils.h"
#include "Common.h"

class WebUI {
    public:
        #if defined(ESP8266)
            std::unique_ptr<ESP8266WebServer> server;
        #elif defined(ESP32)
            std::unique_ptr<WebServer> server;
        #endif
        
        char indexPage[1024] = "\0";

        WebUI(SpaInterface *spa);
        const char * getError();
        void begin();
        bool initialised = false;

    private:
        char buffer[1024];
        SpaInterface *_spa;
        const char *indexPageTemplate =
            "<!DOCTYPE html>"
            "<html lang=\"en\">"
            "<head>"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
            "<meta http-equiv=\"refresh\" content=\"10\">"
            "<meta name=\"color-scheme\" content=\"dark light\"\n>"
            "<link rel=\"stylesheet\" href=\"/styles.css\">"
            "<script>"
            "function confirmAction(url) {"
            "  if (confirm('Are you sure?')) {"
            "    window.location.href = url;"
            "  }"
            "}"
            "</script>"
            "</head>"
            "<body>"
            "<h1>ESP32 Spa Controller</h1>"
            "<p>Spa temperature is - %.1f&deg;C</p>"
            "<p>Spa status is - %s</p>"
            "<p><a href=\"/json\">Status JSON</button></p>"
            "<p><a href=\"/config\">Configuration</button></p>"
            "<p><a href=\"/fota\">Firmware Update</a></p>"
            "<p><a href=\"#\" onclick=\"confirmAction('/reboot'); return false;\">Reboot ESP</a></p>"
            "<p>Build: %s %s</p>"
            "</body>"
            "</html>";

        const char *fotaPage =
            "<!DOCTYPE html>"
            "<html lang=\"en\">"
            "<head>"
            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
            "<meta name=\"color-scheme\" content=\"dark light\">"
            "<link rel=\"stylesheet\" href=\"/styles.css\">"
            "<title>Firmware Update</title>"
            "</head>"
            "<body>"
            "<h1>Firmware Update</h1>"
            "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
            "<p>"
            "<form method='POST' action='' enctype='multipart/form-data' id='upload_form'>"
            "<input type='file' accept='.bin,.bin.gz' name='update'>"
            "<input type='submit' value='Update'>"
            "</form>"
            "</p>"
            "<div id='prg'>progress: 0%</div>"
            "<div id='msg'></div>"
            "<script>"
            "$('form').submit(function(e){"
            "e.preventDefault();"
            "var form = $('#upload_form')[0];"
            "var data = new FormData(form);"
            " $.ajax({"
            "url: '/fota',"
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
            "if (per < 1) {"
            "$('#msg').html('<p>Uploading...</p>');"
            "} else {"
            "$('#msg').html('<p style=\"color:blue;\">Flashing...</p>');"
            "}"
            "}"
            "}, false);"
            "return xhr;"
            "},"
            "success: function(d, s) {"
            "$('#msg').html('<p style=\"color:green;\">Update successful! Rebooting...</p>');"
            "console.log('success!');"
            "setTimeout(function () { window.location.href = '/'; }, 5000);"
            "},"
            "error: function(a, b, c) {"
            "$('#msg').html('<p style=\"color:red;\">Update failed! Please try again.</p>');"
            "console.error('error', a, b, c);"
            "}"
            "});"
            "});"
            "</script>"
            "</body>"
            "</html>";

        const char *styleSheet =
            "input[type=file]::file-selector-button, input[type=\"submit\"], a, button {\n"
            "  padding: 7px 15px;\n"
            "  border: none;\n"
            "  background: #007BFF;\n"
            "  color: white;\n"
            "  text-decoration: none;\n"
            "  border-radius: 5px;\n"
            "  margin-top: 5px;\n"
            "  display: inline-block;\n"
            "  font-size: 16px;\n"
            "  font-family: Arial, sans-serif;\n"
            "  cursor: pointer;\n"
            "  text-align: center;\n"
            "}\n"
            "input[type=file]::file-selector-button:hover, input[type=\"submit\"]:hover, a:hover, button:hover {\n"
            "  background-color: #0056b3;\n"
            "}";
        const char *rebootPage =
R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<META http-equiv="refresh" content="5;URL=/">
<link rel="stylesheet" href="/styles.css">
<title>Firmware Update</title>
</head>
<body>
Rebooting ESP...
</body>
</html>)";

        const char *configPageTemplate =
R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<link rel="stylesheet" href="/styles.css">
<title>Configuration</title>
</head>
<body>
<h1>Configuration</h1>
<form id='config_form' action='#' method='POST'>
<table>
<tr><td>Spa Name:</td><td><input type='text' name='spaName' value='%s'></td></tr>
<tr><td>MQTT Server:</td><td><input type='text' name='mqttServer' value='%s'></td></tr>
<tr><td>MQTT Port:</td><td><input type='number' name='mqttPort' value='%s'></td></tr>
<tr><td>MQTT Username:</td><td><input type='text' name='mqttUsername' value='%s'></td></tr>
<tr><td>MQTT Password:</td><td><input type='text' name='mqttPassword' value='%s'></td></tr>
<tr><td>Poll Frequency (seconds):</td><td><input type='number' name='updateFrequency' value='%i'></td></tr>
</table>
<input type='submit' value='Save'>
</form>
<div id='msg'></div>  // Message div for success or error messages
<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
<script>
$('#config_form').submit(function(e) {
    e.preventDefault();
    $.ajax({
    url: '/config',
    type: 'POST',
    data: $('#config_form').serialize(),
    success: function(response) {
        $('#msg').html('<p style="color:green;">Configuration updated successfully!</p>');
        setTimeout(function() { $('#msg').html(''); }, 3000);  // Clear message after 3 seconds
    },
    error: function() {
        $('#msg').html('<p style="color:red;">Error updating configuration.</p>');
    }
    });
});
</script>
</body>
</html>)";

};

#endif
