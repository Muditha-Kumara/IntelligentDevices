
#include <Arduino_MKRIoTCarrier.h>
#include "Config.h"
#include "NetworkManager.h"
#include "DisplayManager.h"
#include "AlertManager.h"
#include "VehicleMonitor.h"

MKRIoTCarrier carrier;
NetworkManager networkManager;
DisplayManager displayManager(carrier);
AlertManager alertManager(carrier, displayManager);
VehicleMonitor vehicleMonitor(carrier, networkManager, alertManager);

bool systemReady = false;
unsigned long lastTelemetry = 0;
unsigned long lastSample = 0;

void processMQTTMessage(char* topic, byte* payload, unsigned int length);


void setup() {
  CARRIER_CASE = false;
  Serial.begin(9600);
  delay(1000);

  Serial.println("=== Vehicle Driving Quality Monitor ===");
  Serial.println("Initializing Arduino Opla IoT Carrier...");

  // Initialize carrier
  if (!carrier.begin()) {
    Serial.println("ERROR: Carrier not connected, check connections");
    while (1);
  }

  // Initialize display
  displayManager.begin();
  displayManager.showInitializing();
  
  // Setup network connectivity
  networkManager.begin();
  networkManager.setMQTTCallback(processMQTTMessage);
  
  // Calibrate sensors
  displayManager.showCalibrating(0);
  vehicleMonitor.calibrateSensors();
  displayManager.flashLEDs(COLOR_GREEN, 2, 500);
  
  // Show system status
  displayManager.showSystemStatus(
    networkManager.isWiFiConnected(),
    networkManager.isMQTTConnected(),
    vehicleMonitor.isCalibrationComplete()
  );
  
  delay(3000);
  
  systemReady = true;
  Serial.println("System ready for vehicle monitoring!");
}

void loop() {
  if (!systemReady) return;
  
  // Maintain network connection
  networkManager.update();
  
  unsigned long currentTime = millis();
  
  // Read sensors at configured sampling rate
  Thresholds& thresholds = vehicleMonitor.getThresholds();
  if (currentTime - lastSample >= thresholds.sampling_rate) {
    vehicleMonitor.readSensors();
    vehicleMonitor.detectVehicleEvents();
    
    VehicleData data = vehicleMonitor.getSensorData();
    displayManager.showMonitoringStatus(
      data,
      thresholds,
      networkManager.isWiFiConnected(),
      networkManager.isMQTTConnected(),
      systemReady
    );
    displayManager.updateStatusLED(
      systemReady,
      networkManager.isWiFiConnected(),
      networkManager.isMQTTConnected()
    );
    
    lastSample = currentTime;
  }
  
  // Publish telemetry at configured interval
  if (currentTime - lastTelemetry >= thresholds.telemetry_interval) {
    VehicleData data = vehicleMonitor.getSensorData();
    networkManager.publishTelemetry(data);
    lastTelemetry = currentTime;
  }
  
  // Check for button press for manual calibration
  carrier.Buttons.update();
  if (carrier.Buttons.onTouchDown(TOUCH2)) {
    Serial.println("Manual recalibration initiated...");
    displayManager.showCalibrating(0);
    vehicleMonitor.calibrateSensors();
    displayManager.flashLEDs(COLOR_GREEN, 2, 500);
    displayManager.showSystemStatus(
      networkManager.isWiFiConnected(),
      networkManager.isMQTTConnected(),
      vehicleMonitor.isCalibrationComplete()
    );
    delay(3000);
  }
  
  delay(10); // Small delay to prevent watchdog issues
}

void processMQTTMessage(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("MQTT message received on topic: " + String(topic));
  Serial.println("Message: " + message);
  
  // Process configuration updates
  if (String(topic) == CONFIG_TOPIC) {
    StaticJsonDocument<300> doc;
    deserializeJson(doc, message);
    
    if (doc.containsKey("thresholds")) {
      JsonObject thresholdObj = doc["thresholds"];
      Thresholds& thresholds = vehicleMonitor.getThresholds();
      
      if (thresholdObj.containsKey("alert_accel")) {
        thresholds.alert_accel = thresholdObj["alert_accel"];
      }
      if (thresholdObj.containsKey("critical_accel")) {
        thresholds.critical_accel = thresholdObj["critical_accel"];
      }
      if (thresholdObj.containsKey("hard_braking")) {
        thresholds.hard_braking = thresholdObj["hard_braking"];
      }
      if (thresholdObj.containsKey("sharp_turn")) {
        thresholds.sharp_turn = thresholdObj["sharp_turn"];
      }
      if (thresholdObj.containsKey("sampling_rate")) {
        thresholds.sampling_rate = thresholdObj["sampling_rate"];
      }
      if (thresholdObj.containsKey("telemetry_interval")) {
        thresholds.telemetry_interval = thresholdObj["telemetry_interval"];
      }
      
      Serial.println("Configuration updated via MQTT");
    }
  }
}
