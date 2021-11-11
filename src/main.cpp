#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WebServer.h>
#include <RemoteDebug.h>
#include <PubSubClient.h>
#include <LITTLEFS.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "Blinker.h"
#include "SpaNetController.h"

  //See file .../hardware/espressif/esp32/variants/(esp32|doitESP32devkitV1)/pins_arduino.h
  #define LED_BUILTIN       2         // Pin D2 mapped to pin GPIO2/ADC12 of ESP32, control on-board LED
  
  #define PIN_D0            0         // Pin D0 mapped to pin GPIO0/BOOT/ADC11/TOUCH1 of ESP32
  #define PIN_D1            1         // Pin D1 mapped to pin GPIO1/TX0 of ESP32
  #define PIN_D2            2         // Pin D2 mapped to pin GPIO2/ADC12/TOUCH2 of ESP32
  #define PIN_D3            3         // Pin D3 mapped to pin GPIO3/RX0 of ESP32
  #define PIN_D4            4         // Pin D4 mapped to pin GPIO4/ADC10/TOUCH0 of ESP32
  #define PIN_D5            5         // Pin D5 mapped to pin GPIO5/SPISS/VSPI_SS of ESP32
  #define PIN_D6            6         // Pin D6 mapped to pin GPIO6/FLASH_SCK of ESP32
  #define PIN_D7            7         // Pin D7 mapped to pin GPIO7/FLASH_D0 of ESP32
  #define PIN_D8            8         // Pin D8 mapped to pin GPIO8/FLASH_D1 of ESP32
  #define PIN_D9            9         // Pin D9 mapped to pin GPIO9/FLASH_D2 of ESP32
  
  #define PIN_D10           10        // Pin D10 mapped to pin GPIO10/FLASH_D3 of ESP32
  #define PIN_D11           11        // Pin D11 mapped to pin GPIO11/FLASH_CMD of ESP32
  #define PIN_D12           12        // Pin D12 mapped to pin GPIO12/HSPI_MISO/ADC15/TOUCH5/TDI of ESP32
  #define PIN_D13           13        // Pin D13 mapped to pin GPIO13/HSPI_MOSI/ADC14/TOUCH4/TCK of ESP32
  #define PIN_D14           14        // Pin D14 mapped to pin GPIO14/HSPI_SCK/ADC16/TOUCH6/TMS of ESP32
  #define PIN_D15           15        // Pin D15 mapped to pin GPIO15/HSPI_SS/ADC13/TOUCH3/TDO of ESP32
  #define PIN_D16           16        // Pin D16 mapped to pin GPIO16/TX2 of ESP32
  #define PIN_D17           17        // Pin D17 mapped to pin GPIO17/RX2 of ESP32     
  #define PIN_D18           18        // Pin D18 mapped to pin GPIO18/VSPI_SCK of ESP32
  #define PIN_D19           19        // Pin D19 mapped to pin GPIO19/VSPI_MISO of ESP32
  
  #define PIN_D21           21        // Pin D21 mapped to pin GPIO21/SDA of ESP32
  #define PIN_D22           22        // Pin D22 mapped to pin GPIO22/SCL of ESP32
  #define PIN_D23           23        // Pin D23 mapped to pin GPIO23/VSPI_MOSI of ESP32
  #define PIN_D24           24        // Pin D24 mapped to pin GPIO24 of ESP32
  #define PIN_D25           25        // Pin D25 mapped to pin GPIO25/ADC18/DAC1 of ESP32
  #define PIN_D26           26        // Pin D26 mapped to pin GPIO26/ADC19/DAC2 of ESP32
  #define PIN_D27           27        // Pin D27 mapped to pin GPIO27/ADC17/TOUCH7 of ESP32     
  
  #define PIN_D32           32        // Pin D32 mapped to pin GPIO32/ADC4/TOUCH9 of ESP32
  #define PIN_D33           33        // Pin D33 mapped to pin GPIO33/ADC5/TOUCH8 of ESP32
  #define PIN_D34           34        // Pin D34 mapped to pin GPIO34/ADC6 of ESP32
  
  //Only GPIO pin < 34 can be used as output. Pins >= 34 can be only inputs
  //See .../cores/esp32/esp32-hal-gpio.h/c
  //#define digitalPinIsValid(pin)          ((pin) < 40 && esp32_gpioMux[(pin)].reg)
  //#define digitalPinCanOutput(pin)        ((pin) < 34 && esp32_gpioMux[(pin)].reg)
  //#define digitalPinToRtcPin(pin)         (((pin) < 40)?esp32_gpioMux[(pin)].rtc:-1)
  //#define digitalPinToAnalogChannel(pin)  (((pin) < 40)?esp32_gpioMux[(pin)].adc:-1)
  //#define digitalPinToTouchChannel(pin)   (((pin) < 40)?esp32_gpioMux[(pin)].touch:-1)
  //#define digitalPinToDacChannel(pin)     (((pin) == 25)?0:((pin) == 26)?1:-1)
  
  #define PIN_D35           35        // Pin D35 mapped to pin GPIO35/ADC7 of ESP32
  #define PIN_D36           36        // Pin D36 mapped to pin GPIO36/ADC0/SVP of ESP32
  #define PIN_D39           39        // Pin D39 mapped to pin GPIO39/ADC3/SVN of ESP32
  
  #define PIN_RX0            3        // Pin RX0 mapped to pin GPIO3/RX0 of ESP32
  #define PIN_TX0            1        // Pin TX0 mapped to pin GPIO1/TX0 of ESP32
  
  #define PIN_SCL           22        // Pin SCL mapped to pin GPIO22/SCL of ESP32
  #define PIN_SDA           21        // Pin SDA mapped to pin GPIO21/SDA of ESP32   

const int TRIGGER_PIN = PIN_D0;   // Pin D0 mapped to pin GPIO0/BOOT/ADC11/TOUCH1 of ESP32
const int TRIGGER_PIN2 = PIN_D25; // Pin D25 mapped to pin GPIO25/ADC18/DAC1 of ESP32

#define LED_BUILTIN       2 // Pin D2 mapped to pin GPIO2/ADC12 of ESP32, control on-board LED

WebServer server(80);
RemoteDebug Debug;
Blinker led(LED_BUILTIN);
WiFiClient wifi;
PubSubClient mqttClient(wifi);
SpaNetController snc;



const char* serverIndex =
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


// MQTT Interface




class MQTT {
  public:
    String server;
    String port;
    String baseTopic;
};


MQTT mqtt;


// End MQTT



bool saveConfig = false;

void saveConfigCallback(){
  saveConfig = true;
}


void checkButton(){
  if(digitalRead(TRIGGER_PIN) == LOW) {
    delay(100);
    if(digitalRead(TRIGGER_PIN) == LOW) {
      server.stop();
      
      WiFiManager wm;
      WiFiManagerParameter custom_mqtt_server("server", "MQTT server", mqtt.server.c_str(), 40);
      WiFiManagerParameter custom_mqtt_port("port", "MQTT port", mqtt.port.c_str(), 6);
      wm.addParameter(&custom_mqtt_server);
      wm.addParameter(&custom_mqtt_port);
      wm.setBreakAfterConfig(true);
      wm.setSaveConfigCallback(saveConfigCallback);
      
      debugI("Button Pressed, Starting Portal");
      wm.startConfigPortal();
      debugI("Exiting Portal");


      if (saveConfig) {

        mqtt.server= String(custom_mqtt_server.getValue());
        mqtt.port= String(custom_mqtt_port.getValue());

        debugI("Updating config file");
        DynamicJsonDocument json(1024);
        json["mqtt_server"] = mqtt.server;
        json["mqtt_port"] = mqtt.port;

        File configFile = LITTLEFS.open("/config.json","w");
        if (!configFile) {
          debugE("Failed to open config file for writing");
        } else {
          serializeJson(json, configFile);
          configFile.close();
          debugI("Config file updated");
        }
      }

      ESP.restart();  // restart, dirty but easier than trying to restart services one by one
    }
  }
}


void mqttCallback(char* topic, byte* payload, unsigned int length){
  String t = String(topic);
  
  int start = t.lastIndexOf("/",t.lastIndexOf("/")-1)+1; //get second last "/"
  String item = t.substring(start, t.lastIndexOf("/"));

  String p = "";
  for (int x = 0; x<length; x++ ) {
    p = p + char(*payload);
    payload++;
  }

  debugI("Got update for %s of %s",item.c_str(),p.c_str());

  snc.pushCommand(item, p);
  
}



void mqttPublishStatus(SpaNetController *s){

  // This gets called each time the spa does a successful poll
  // it takes as a pearemeter a pointer to the calling instance.

  mqttClient.publish((mqtt.baseTopic+"voltage/value").c_str(),String(s->getVolts()).c_str());
  mqttClient.publish((mqtt.baseTopic+"current/value").c_str(),String(s->getAmps()).c_str());
  mqttClient.publish((mqtt.baseTopic+"hpump_amb_temp/value").c_str(),String(s->getHpumpAmbTemp()).c_str());
  mqttClient.publish((mqtt.baseTopic+"hpump_con_temp/value").c_str(),String(s->getHpumpConTemp()).c_str());
  mqttClient.publish((mqtt.baseTopic+"lights/value").c_str(),String(s->isLightsOn()).c_str());
}


DynamicJsonDocument mqttSensorJson(DynamicJsonDocument base,String dataPointId,String dataPointName){
  String spaId = base["device"]["identifiers"];
  base["state_topic"]=mqtt.baseTopic+dataPointId+"/value";
  base["name"]=dataPointName;
  base["unique_id"]="spanet_"+spaId+"_"+dataPointId;
  return base;

}

void mqttSensorADPublish(DynamicJsonDocument base,String dataPointId,String dataPointName,String deviceClass, String uom){
  base = mqttSensorJson(base, dataPointId, dataPointName);
  String spaId = base["device"]["identifiers"];

  base["unit_of_measurement"]=uom;
  base["device_class"]=deviceClass;
  
  String topic = "homeassistant/sensor/spanet_"+spaId+"/"+dataPointId+"/config";
  String output;
  serializeJsonPretty(base,output);
  mqttClient.publish(topic.c_str(),output.c_str(),true);
}


DynamicJsonDocument mqttSwitchJson(DynamicJsonDocument base,String dataPointId,String dataPointName) {
  base = mqttSensorJson(base, dataPointId, dataPointName);
  base["command_topic"]=mqtt.baseTopic+dataPointId+"/set";
  base["payload_on"]="1";
  base["payload_off"]="0";
  return base;
}

void mqttLightsADPublish(DynamicJsonDocument base,String dataPointId,String dataPointName) {
  String spaId = base["device"]["identifiers"];
  base = mqttSwitchJson(base, dataPointId, dataPointName);

  String topic = "homeassistant/light/spanet_"+spaId+"/"+dataPointId+"/config";
  String output;
  serializeJsonPretty(base,output);
  mqttClient.publish(topic.c_str(),output.c_str(),true);

}


void mqttHaAutoDiscovery()
{

  String spaName = "MySpa";
  String spaSerialNumber = "11223344";
  String output,topic;

  DynamicJsonDocument haTemplate(1024);

  debugI("Publishing Home Assistant auto discovery");

  JsonObject device = haTemplate.createNestedObject("device");

  device["identifiers"]=spaSerialNumber;
  device["name"]=spaName;

  haTemplate["availability_topic"]=mqtt.baseTopic+"available";

  mqttSensorADPublish(haTemplate,"voltage","Supply Voltage","voltage","v");
  mqttSensorADPublish(haTemplate,"current","Supply Current","current","A");
  mqttSensorADPublish(haTemplate,"hpump_amb_temp","Heatpump Ambient Temperature","temperature","℃");
  mqttSensorADPublish(haTemplate,"hpump_con_temp","Heatpump Condensor Temperature","temperature","℃");
  mqttLightsADPublish(haTemplate,"lights","Lights");

}



void setup() {

  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN2, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  delay(200);

  WiFi.mode(WIFI_STA); 
  WiFi.begin();


  Debug.begin(WiFi.getHostname());
  Debug.setResetCmdEnabled(true);
  Debug.showProfiler(true);
  Debug.setSerialEnabled(true);

  debugA("Starting...");


  debugI("Mounting FS");
  if (LITTLEFS.begin()){
    debugI("Mounted FS");
    debugI("Reading config file");
    File configFile = LITTLEFS.open("/config.json","r");
    if (configFile) {
      debugI("Reading config file");
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);

      DynamicJsonDocument json(1024);
      auto deserializeError = deserializeJson(json, buf.get());
      serializeJson(json, Serial);
      if ( ! deserializeError ) {
        debugI("Parsed JSON");
        mqtt.server = json["mqtt_server"].as<String>();
        mqtt.port = json["mqtt_port"].as<String>();
      } else {
        debugW("Failed to parse config file");
      }
      configFile.close();
    }

  } else {
    debugW("Failed to mount file system");
  }
  
  if (mqtt.server == "") { mqtt.server = "mqtt"; }
  if (mqtt.port == "") { mqtt.port = "1883"; }

  mqtt.baseTopic = "sn_esp32/MySpa/";  // TODO - this should be "sn_esp32"+spa.serialNumber

  mqttClient.setServer(mqtt.server.c_str(),mqtt.port.toInt());
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(2048);
  snc.subscribeUpdate(mqttPublishStatus);

  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
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
  server.begin();    

};

long mqttLastConnect = 0;

void loop() {
  checkButton();
  led.tick();
  server.handleClient();
  snc.tick();

  if (WiFi.status() == WL_CONNECTED) {
  
      if (!mqttClient.connected()) {
        long now=millis();
        if (now - mqttLastConnect > 5000) {
          led.setInterval(500);
          debugW("MQTT not connected, attempting connection to %s:%s",mqtt.server.c_str(),mqtt.port.c_str());
          mqttLastConnect = now;
          if (mqttClient.connect("sn_esp32", (mqtt.baseTopic+"available").c_str(),2,true,"offline")) {
            debugI("MQTT connected");
            mqttClient.subscribe((mqtt.baseTopic+"+/set").c_str());
            mqttClient.publish((mqtt.baseTopic+"available").c_str(),"online",true);
            mqttHaAutoDiscovery();
            snc.forceUpdate();
          } else {
            debugW("MQTT connection failed");
          }
        }
      }
      else {
        led.setInterval(2000);
      }

  } else {
      led.setInterval(100);
      long now = millis();
      if (now-wifiLastConnect > 10000) {
        wifiLastConnect = now;
        WiFi.disconnect();
        WiFi.reconnect();
      }
  }

  Debug.handle();
  mqttClient.loop();

  delay(1);

};