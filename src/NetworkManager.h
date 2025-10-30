/**
 * NetworkManager.h
 * Handles WiFi and MQTT connectivity
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFiNINA.h>
#include <WiFiSSLClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "aws_certificates.h"

class NetworkManager {
public:
  NetworkManager();
  
  void begin();
  void update();
  bool isWiFiConnected();
  bool isMQTTConnected();
  
  void publishTelemetry(const VehicleData& data);
  void publishEvent(String eventType, String level, String description, const VehicleData& data);
  
  void setMQTTCallback(void (*callback)(char*, byte*, unsigned int));
  
private:
  WiFiSSLClient wifiClient;
  PubSubClient mqttClient;
  
  bool wifiConnected;
  bool mqttConnected;
  
  void setupWiFi();
  void reconnectMQTT();
};

#endif // NETWORK_MANAGER_H
