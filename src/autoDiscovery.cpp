#include "autoDiscovery.h"
/*
DynamicJsonDocument mqttSensorJson(DynamicJsonDocument base, String dataPointId, String dataPointName) {
    String spaId = base["device"]["identifiers"];
    base["state_topic"] = mqtt.baseTopic + dataPointId + "/value";
    base["name"] = dataPointName;
    base["unique_id"] = "spanet_" + spaId + "_" + dataPointId;
    return base;
}

void mqttBinarySensorADPublish(DynamicJsonDocument base, String dataPointId, String dataPointName, String deviceClass) {
  base = mqttSensorJson(base, dataPointId, dataPointName);
  String spaId = base["device"]["identifiers"];

  if (deviceClass!="") {
    base["device_class"]=deviceClass;
  }

  base["payload_on"] = "1";
  base["payload_off"] = "0";

  String topic = "homeassistant/binary_sensor/spanet_"+spaId+"/"+dataPointId+"/config";
  String output;
  serializeJsonPretty(base,output);
  mqttClient.publish(topic.c_str(),output.c_str(),true);
}

void mqttSensorADPublish(DynamicJsonDocument base, String dataPointId, String dataPointName, String deviceClass, String uom){
  base = mqttSensorJson(base, dataPointId, dataPointName);
  String spaId = base["device"]["identifiers"];

  base["unit_of_measurement"]=uom;
  base["device_class"]=deviceClass;
  
  String topic = "homeassistant/sensor/spanet_"+spaId+"/"+dataPointId+"/config";
  String output;
  serializeJsonPretty(base,output);
  mqttClient.publish(topic.c_str(),output.c_str(),true);
}

void mqttSensorADPublish(DynamicJsonDocument base, String dataPointId, String dataPointName, String deviceClass, String stateClass, String uom){
  base = mqttSensorJson(base, dataPointId, dataPointName);
  String spaId = base["device"]["identifiers"];

  base["unit_of_measurement"]=uom;
  base["device_class"]=deviceClass;
  base["state_class"]=stateClass;
  
  String topic = "homeassistant/sensor/spanet_"+spaId+"/"+dataPointId+"/config";
  String output;
  serializeJsonPretty(base,output);
  mqttClient.publish(topic.c_str(),output.c_str(),true);
}


DynamicJsonDocument mqttSwitchJson(DynamicJsonDocument base, String dataPointId, String dataPointName) {
  base = mqttSensorJson(base, dataPointId, dataPointName);
  base["command_topic"]=mqtt.baseTopic+dataPointId+"/set";
  base["payload_on"]="1";
  base["payload_off"]="0";
  return base;
}

void mqttSwitchADPublish(DynamicJsonDocument base, String dataPointId, String dataPointName) {
  String spaId = base["device"]["identifiers"];
  base = mqttSwitchJson(base, dataPointId, dataPointName);
  String topic = "homeassistant/switch/spanet_"+spaId+"/"+dataPointId+"/config";
  String output;
  serializeJsonPretty(base,output);
  mqttClient.publish(topic.c_str(),output.c_str(),true);

}

void mqttLightsADPublish(DynamicJsonDocument base, String dataPointId, String dataPointName) {
  String spaId = base["device"]["identifiers"];
  base = mqttSensorJson(base, dataPointId, dataPointName);
  base["command_topic"]=mqtt.baseTopic+dataPointId+"/set";
  base["schema"] = "json";
  base["brightness"] = true;
  base["color_mode"] = true;
  base["effect"] = true;
  JsonArray effect_list = base.createNestedArray("effect_list");
  effect_list.add("White");
  effect_list.add("Color");
  effect_list.add("Fade");
  effect_list.add("Step");
  effect_list.add("Party");
  JsonArray color_modes = base.createNestedArray("supported_color_modes");
  color_modes.add("hs");

  String topic = "homeassistant/light/spanet_" + spaId + "/" + dataPointId + "/config";
  String output;
  serializeJsonPretty(base,output);
  mqttClient.publish(topic.c_str(),output.c_str(),true);
}

void mqttPumpSelectADPublish(DynamicJsonDocument base, String dataPointId, String dataPointName) {
  String spaId = base["device"]["identifiers"];
  base["state_topic"] = mqtt.baseTopic + dataPointId + "_text/value";
  base["command_topic"] = mqtt.baseTopic + dataPointId + "_text/set";
  base["name"] = dataPointName;
  base["unique_id"] = "spanet_" + spaId + "_" + dataPointId;
  
  JsonArray options = base.createNestedArray("options");
  for (int i = 0; i < PUMP_MODES_COUNT; i++) {
    if (!strEquals(Pump::pump_modes[i],"")) {
      options.add(Pump::pump_modes[i]);
    }
  }

  String topic = "homeassistant/select/spanet_" + spaId + "/" + dataPointId + "/config";
  String output;
  serializeJsonPretty(base,output);
  mqttClient.publish(topic.c_str(),output.c_str(),true);
}

void mqttClimateADPublish(DynamicJsonDocument base) {
  String spaId = base["device"]["identifiers"];
  base["unique_id"] = "spanet_" + spaId + "_thermostat";
  base["temperature_state_topic"] = mqtt.baseTopic + "water_temp_set_point/value";
  base["temperature_command_topic"] = mqtt.baseTopic + "water_temp_set_point/set";
  base["current_temperature_topic"] = mqtt.baseTopic + "water_temp/value";

  JsonArray modes = base.createNestedArray("modes");
  modes.add("off");
  modes.add("cool");
  modes.add("heat");
  modes.add("auto");
  
  base["mode_state_topic"] = mqtt.baseTopic + "heat_pump_mode_txt/value";
  base["mode_command_topic"] = mqtt.baseTopic + "heat_pump_mode_txt/set";
  base["max_temp"] = 41;
  base["min_temp"] = 5;
  base["precision"] = 0.1;
  base["temp_step"] = 0.5;
  base["name"] = base["device"]["name"];

  String topic = "homeassistant/climate/spanet_" + spaId + "/config";
  String output;
  serializeJsonPretty(base,output);
  mqttClient.publish(topic.c_str(),output.c_str(),true);
}
*/

/// @brief Publish a Sensor via MQTT auto discovery
/// @param deviceClass Sensor, etc
/// @param stateTopic Mqtt topic to read state information from.
/// @param unitOfMeasurement V, W, A, mV, etc
/// @param valueTemplate HA value template to parse topic payload to derive value
/// @param uniqueID Spa serial number + sensor id eg 123456-789012_temperature
/// @param deviceName Spa name eg MySpa
/// @param deviceIdentifier Spa serial number eg 123456-789012
void sensorADPublish(String deviceClass, String stateTopic, String unitOfMeasurement, String valueTemplate, String uniqueID, String deviceName, String deviceIdentifier ) {

/*
{ 
   "device_class":"temperature",
   "state_topic":"homeassistant/sensor/sensorBedroom/state",
   "unit_of_measurement":"°C",
   "value_template":"{{ value_json.temperature}}",
   "unique_id":"temp01ae",
   "device":{
      "identifiers":[
         "bedroom01ae"
      ],
      "name":"Bedroom"
   }
}"
*/

  StaticJsonDocument<512> json;

  json["device_class"] = deviceClass;
  json["state_topic"] = stateTopic;
  json["unit_of_measurement"] = unitOfMeasurement;
  json["value_template"] = valueTemplate;
  json["unique_id"] = uniqueID;
  JsonObject device = json.createNestedObject("device");
  device["name"] = deviceName;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(deviceIdentifier);

  // <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
  String discoveryTopic = "homeassistant/sensor/"+uniqueID+"/config";
  String output = "";
  serializeJson(json,output);
  mqttClient.publish(discoveryTopic.c_str(),output.c_str(),true);

}

void mqttHaAutoDiscovery() {
  String spaName = "MySpa"; //TODO - This needs to be a settable parameter.
  String spaSerialNumber = sni.getSerialNo1()+"-"+sni.getSerialNo2();

  debugI("Publishing Home Assistant auto discovery");

  sensorADPublish("temperature","sn_esp32/"+spaSerialNumber+"/state","°C","{{ value_json.temperature }}",spaSerialNumber+"-Temperature", spaName, spaSerialNumber);

 /*                             
  String output,topic;

  DynamicJsonDocument haTemplate(1024);

  debugI("Publishing Home Assistant auto discovery");

  JsonObject device = haTemplate.createNestedObject("device");

  device["identifiers"]=spaSerialNumber;
  device["name"]=spaName;

  haTemplate["availability_topic"]=mqtt.baseTopic+"available";

  mqttSensorADPublish(haTemplate, "voltage", "Supply Voltage", "voltage", "v");
  mqttSensorADPublish(haTemplate, "current", "Supply Current", "current", "A");
  mqttBinarySensorADPublish(haTemplate, "heating_active", "Heating Active", "");
  mqttBinarySensorADPublish(haTemplate, "uv_ozone_active", "UV/Ozone Active", "");
  mqttBinarySensorADPublish(haTemplate, "sanatise_running", "Sanatise Cycle Running", "");
  mqttSensorADPublish(haTemplate, "hpump_amb_temp", "Heatpump Ambient Temperature", "temperature","°C");
  mqttSensorADPublish(haTemplate, "hpump_con_temp", "Heatpump Condensor Temperature", "temperature", "°C");
  mqttSensorADPublish(haTemplate, "water_temp", "Water Temperature", "temperature", "°C");  //Publish this as a sensor as well as HVAC so as to allow eaiser trending
  mqttLightsADPublish(haTemplate, "lights", "Lights");
  mqttClimateADPublish(haTemplate);

  mqttSensorADPublish(haTemplate, "total_energy", "Total Energy", "energy", "total_increasing", "kWh");
  mqttSensorADPublish(haTemplate, "energy_today", "Energy Today", "energy", "total_increasing", "kWh");
  mqttSensorADPublish(haTemplate, "power_consumption", "Power Consumption", "power", "W");
  
  
  mqttSwitchADPublish(haTemplate,"resitive_heating","Aux Resitive Heating");



  for (int x = 0; x < 5; x++) {
    Pump *pump = snc.getPump(x);
    if (pump->isInstalled()) {
      String id = "pump" + String(x+1) + "_operating_mode";
      String name = "Pump " + String(x+1);
      if (!pump->isAutoModeSupported()){
        mqttSwitchADPublish(haTemplate, id, name);  // Pumps should not be published as switches, rather fans, so to support mutispeed pumps.
      } else {
        mqttPumpSelectADPublish(haTemplate, id, name);
      }
    }
  }
*/
}
