#ifndef WEBUI_H
#define WEBUI_H

#define BUILD_INFO __DATE__ " " __TIME__

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

        WebUI(SpaInterface *spa);
        void begin();
        bool initialised = false;

    private:
        SpaInterface *_spa;

        const char* getError();

static constexpr const char *indexPageTemplate PROGMEM =
R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<link rel="stylesheet" href="/styles.css">
<script>
function confirmAction(url) {
    if (confirm('Are you sure?')) {
    window.location.href = url;
    }
}
function fetchStatus() {
  fetch('/json')
    .then(response => response.json())
    .then(value_json => {
      document.getElementById('temperatures_water').innerText = value_json.temperatures.water + "\u00B0C";
      document.getElementById('status_state').innerText = value_json.status.state;
      document.getElementById('temperatures_setPoint').value = value_json.temperatures.setPoint;
    })
    .catch(error => console.error('Error fetching status:', error));
}
window.onload = function() {
    fetchStatus();
    setInterval(fetchStatus, 10000);
};
</script>
</head>
<body>
<h1>ESP32 Spa Controller</h1>
<p>Spa temperature is - <span id="temperatures_water">Loading...</span></p>
<p>Spa status is - <span id="status_state">Loading...</span></p>
<p><a href="/json">Status JSON</a></p>
<p><a href="/config">Configuration</a></p>
<p><a href="/fota">Firmware Update</a></p>
<p><a href="#" onclick="confirmAction('/wifi-manager'); return false;">Wi-Fi Manager</a></p>
<p><a href="#" onclick="confirmAction('/reboot'); return false;">Reboot ESP</a></p>
<p>Build: )" BUILD_INFO R"(</p>
</body>
</html>)";

static constexpr const char *fotaPage PROGMEM =
R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<link rel="stylesheet" href="/styles.css">
<title>Firmware Update</title>
</head>
<body>
<h1>Firmware Update</h1>
<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
<form method='POST' action='' enctype='multipart/form-data' id='upload_form'>
<input type='file' accept='.bin,.bin.gz' name='update'>
<input type='submit' value='Update'>
</form>
<div id='prg'>progress: 0%</div>
<div id='msg'></div>
<script>
$('form').submit(function(e){
  e.preventDefault();
  var form = $('#upload_form')[0];
  var data = new FormData(form);
  $.ajax({
    url: '/fota',
    type: 'POST',
    data: data,
    contentType: false,
    processData: false,
    xhr: function() {
      var xhr = new window.XMLHttpRequest();
        xhr.upload.addEventListener('progress', function(evt) {
        if (evt.lengthComputable) {
            var per = evt.loaded / evt.total;
            $('#prg').html('progress: ' + Math.round(per*100) + '%');
          if (per < 1) {
            $('#msg').html('<p>Uploading...</p>');
          } else {
            $('#msg').html('<p style="color:blue;">Flashing...</p>');
          }
        }
      }, false);
      return xhr;
    },
    success: function() {
      $('#msg').html('<p style="color:green;">Update successful! Rebooting...</p>');
      setTimeout(function () { window.location.href = '/'; }, 5000);
    },
    error: function() {
      $('#msg').html('<p style="color:red;">Update failed! Please try again.</p>');
    }
  });
});
</script>
</body>
</html>)";

static constexpr const char *styleSheet PROGMEM =
R"(
input[type=file]::file-selector-button, input[type="submit"], a, button {
    padding: 7px 15px;
    border: none;
    background: #007BFF;
    color: white;
    text-decoration: none;
    border-radius: 5px;
    margin-top: 5px;
    display: inline-block;
    font-size: 16px;
    font-family: Arial, sans-serif;
    cursor: pointer;
    text-align: center;
}
input[type=file]::file-selector-button:hover, input[type="submit"]:hover, a:hover, button:hover {
    background-color: #0056b3;
})";

static constexpr const char *rebootPage PROGMEM =
R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta name="color-scheme" content="dark light">
<META http-equiv="refresh" content="5;URL=/">
<link rel="stylesheet" href="/styles.css">
<title>Rebooting</title>
</head>
<body>
<h1>Rebooting ESP...</h1>
</body>
</html>)";

static constexpr const char *configPageTemplate PROGMEM =
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
<tr><td>Spa Name:</td><td><input type='text' name='spaName' id='spaName'></td></tr>
<tr><td>MQTT Server:</td><td><input type='text' name='mqttServer' id='mqttServer'></td></tr>
<tr><td>MQTT Port:</td><td><input type='number' name='mqttPort' id='mqttPort'></td></tr>
<tr><td>MQTT Username:</td><td><input type='text' name='mqttUsername' id='mqttUsername'></td></tr>
<tr><td>MQTT Password:</td><td><input type='text' name='mqttPassword' id='mqttPassword'></td></tr>
<tr><td>Poll Frequency (seconds):</td><td><input type='number' name='updateFrequency' id='updateFrequency' step="1" min="10" max="300"></td></tr>
</table>
<input type='submit' value='Save'>
</form>
<div id='msg'></div>
<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
<script>
function loadConfig() {
  fetch('/json/config')
    .then(response => response.json())
    .then(data => {
      document.getElementById('spaName').value = data.spaName;
      document.getElementById('mqttServer').value = data.mqttServer;
      document.getElementById('mqttPort').value = data.mqttPort;
      document.getElementById('mqttUsername').value = data.mqttUsername;
      document.getElementById('mqttPassword').value = data.mqttPassword;
      document.getElementById('updateFrequency').value = data.updateFrequency;
    })
  .catch(error => console.error('Error loading config:', error));
}
document.addEventListener("DOMContentLoaded", loadConfig);
$('#config_form').submit(function(e) {
  e.preventDefault();
  $.ajax({
    url: '/config',
    type: 'POST',
    data: $('#config_form').serialize(),
    success: function() {
      $('#msg').html('<p style="color:green;">Configuration updated successfully!</p>');
      setTimeout(function() { $('#msg').html(''); }, 3000);
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

#endif // WEBUI_H
