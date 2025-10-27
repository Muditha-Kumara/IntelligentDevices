/**
 * NetworkManager.cpp
 * Implementation of WiFi and MQTT connectivity
 */

#include "NetworkManager.h"

NetworkManager::NetworkManager() 
  : mqttClient(wifiClient), wifiConnected(false), mqttConnected(false) {
}

void NetworkManager::begin() {
  setupWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  reconnectMQTT();
}

void NetworkManager::update() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
}

bool NetworkManager::isWiFiConnected() {
  return wifiConnected;
}

bool NetworkManager::isMQTTConnected() {
  return mqttConnected;
}

void NetworkManager::setMQTTCallback(void (*callback)(char*, byte*, unsigned int)) {
  mqttClient.setCallback(callback);
}

void NetworkManager::setupWiFi() {
  Serial.print("Connecting to WiFi");
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("");
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection failed!");
  }
}

void NetworkManager::reconnectMQTT() {
  while (!mqttClient.connected() && wifiConnected) {
    Serial.print("Attempting MQTT connection...");
    
    if (mqttClient.connect(DEVICE_ID)) {
      mqttConnected = true;
      Serial.println("MQTT connected!");
      
      // Subscribe to config topic
      mqttClient.subscribe(CONFIG_TOPIC.c_str());
      
      // Publish ready message
      StaticJsonDocument<200> readyMsg;
      readyMsg["device_id"] = DEVICE_ID;
      readyMsg["status"] = "ready";
      readyMsg["timestamp"] = millis();
      
      String readyJson;
      serializeJson(readyMsg, readyJson);
      mqttClient.publish(TELEMETRY_TOPIC.c_str(), readyJson.c_str());
      
      break;
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void NetworkManager::publishTelemetry(const VehicleData& data) {
  if (!mqttConnected) return;
  
  StaticJsonDocument<400> doc;
  doc["device_id"] = DEVICE_ID;
  doc["timestamp"] = millis();
  
  JsonObject values = doc.createNestedObject("values");
  values["accel_x"] = round(data.accel_x * 100) / 100.0;
  values["accel_y"] = round(data.accel_y * 100) / 100.0;
  values["accel_z"] = round(data.accel_z * 100) / 100.0;
  values["gyro_x"] = round(data.gyro_x * 100) / 100.0;
  values["gyro_y"] = round(data.gyro_y * 100) / 100.0;
  values["gyro_z"] = round(data.gyro_z * 100) / 100.0;
  values["temperature"] = round(data.temperature * 10) / 10.0;
  values["humidity"] = round(data.humidity * 10) / 10.0;
  values["pressure"] = round(data.pressure * 10) / 10.0;
  values["light_level"] = data.light_level;
  values["mic_level"] = round(data.mic_level * 100) / 100.0;
  
  String telemetryJson;
  serializeJson(doc, telemetryJson);
  
  mqttClient.publish(TELEMETRY_TOPIC.c_str(), telemetryJson.c_str());
  
  Serial.println("Telemetry published: " + telemetryJson);
}

void NetworkManager::publishEvent(String eventType, String level, String description, const VehicleData& data) {
  if (!mqttConnected) return;
  
  StaticJsonDocument<300> doc;
  doc["device_id"] = DEVICE_ID;
  doc["timestamp"] = millis();
  doc["type"] = eventType;
  doc["level"] = level;
  doc["description"] = description;
  
  JsonObject values = doc.createNestedObject("values");
  values["accel_x"] = round(data.accel_x * 100) / 100.0;
  values["accel_y"] = round(data.accel_y * 100) / 100.0;
  values["accel_z"] = round(data.accel_z * 100) / 100.0;
  values["gyro_x"] = round(data.gyro_x * 100) / 100.0;
  values["gyro_y"] = round(data.gyro_y * 100) / 100.0;
  values["gyro_z"] = round(data.gyro_z * 100) / 100.0;
  values["temperature"] = round(data.temperature * 10) / 10.0;
  values["humidity"] = round(data.humidity * 10) / 10.0;
  values["pressure"] = round(data.pressure * 10) / 10.0;
  values["light_level"] = data.light_level;
  values["mic_level"] = round(data.mic_level * 100) / 100.0;
  
  String eventJson;
  serializeJson(doc, eventJson);
  
  mqttClient.publish(EVENTS_TOPIC.c_str(), eventJson.c_str());
  
  Serial.println("Event published: " + eventJson);
}
