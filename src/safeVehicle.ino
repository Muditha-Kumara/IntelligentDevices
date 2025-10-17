#include <Arduino_MKRIoTCarrier.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "visuals.h"
#include "pitches.h"

MKRIoTCarrier carrier;

// WiFi and MQTT configuration
const char* ssid = "SLEngineers";
const char* password = "slengnet1";
const char* mqtt_server = "broker.hivemq.com";  // Free MQTT broker
const int mqtt_port = 1883;
const char* device_id = "opla-01";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// MQTT Topics
String telemetryTopic = "devices/" + String(device_id) + "/telemetry";
String eventsTopic = "devices/" + String(device_id) + "/events";
String configTopic = "devices/" + String(device_id) + "/config";

// Vehicle monitoring variables
struct VehicleData {
  float accel_x, accel_y, accel_z;
  float gyro_x, gyro_y, gyro_z;
  float temperature, humidity, pressure;
  float mic_level;
  int light_level;
} sensorData;

// Detection thresholds (configurable via MQTT)
struct Thresholds {
  float caution_accel = 2.0;      // m/s²
  float alert_accel = 3.5;        // m/s²
  float critical_accel = 5.0;     // m/s²
  float hard_braking = 3.5;       // m/s²
  float sharp_turn = 150.0;       // degrees/s
  float bump_impact = 6.0;        // m/s²
  int sampling_rate = 100;        // ms
  int telemetry_interval = 2000;  // ms
} thresholds;

// Timing variables
unsigned long lastTelemetry = 0;
unsigned long lastSample = 0;

// Calibration offsets
float accel_offset_x = 0, accel_offset_y = 0, accel_offset_z = 0;
float gyro_offset_x = 0, gyro_offset_y = 0, gyro_offset_z = 0;

String state = "monitoring";

uint32_t colorRed = carrier.leds.Color(200, 0, 0);
uint32_t colorGreen = carrier.leds.Color(0, 200, 0);
uint32_t colorBlue = carrier.leds.Color(0, 0, 200);
uint32_t colorYellow = carrier.leds.Color(200, 200, 0);
uint32_t colorOrange = carrier.leds.Color(255, 165, 0);

// notes in the melody:
int finalMelody[] = {
  NOTE_E6, NOTE_G6, NOTE_E7, NOTE_C7, NOTE_D7, NOTE_G7, NOTE_E7, NOTE_G6
};

int celebrationMelody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

//Declare functions
void setupWiFi();
void setupMQTT();
void reconnectMQTT();
void calibrateSensors();
void readSensors();
void detectVehicleEvents();
void publishTelemetry();
void publishEvent(String eventType, String level, String description);
void displayStatus();
void triggerAlert(String eventType, String level);
void processMQTTMessage(char* topic, byte* payload, unsigned int length);
void playWarningTone();
void showSystemStatus();

// System flags
bool systemReady = false;
bool wifiConnected = false;
bool mqttConnected = false;
bool calibrationComplete = false;


void setup()
{
  CARRIER_CASE = false;
  Serial.begin(9600);
  delay(1500);

  Serial.println("=== Vehicle Driving Quality Monitor ===");
  Serial.println("Initializing Arduino Opla IoT Carrier...");

  if (!carrier.begin())
  {
    Serial.println("ERROR: Carrier not connected, check connections");
    while (1);
  }

  // Initialize display
  carrier.display.fillScreen(0x0000);
  carrier.display.setRotation(0);
  carrier.display.setTextWrap(true);
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(20, 20);
  carrier.display.println("Vehicle Monitor");
  carrier.display.setCursor(30, 50);
  carrier.display.println("Initializing...");

  // Setup connections
  setupWiFi();
  setupMQTT();
  
  // Calibrate sensors
  calibrateSensors();
  
  // Show ready status
  showSystemStatus();
  
  systemReady = true;
  Serial.println("System ready for vehicle monitoring!");
}

void loop()
{
  if (!systemReady) return;
  
  // Maintain MQTT connection
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
  
  unsigned long currentTime = millis();
  
  // Read sensors at configured sampling rate
  if (currentTime - lastSample >= thresholds.sampling_rate) {
    readSensors();
    detectVehicleEvents();
    displayStatus();
    lastSample = currentTime;
  }
  
  // Publish telemetry at configured interval
  if (currentTime - lastTelemetry >= thresholds.telemetry_interval) {
    publishTelemetry();
    lastTelemetry = currentTime;
  }
  
  // Check for button presses for manual calibration
  carrier.Buttons.update();
  if (carrier.Buttons.onTouchDown(TOUCH2)) {
    Serial.println("Manual recalibration initiated...");
    calibrateSensors();
    showSystemStatus();
  }
  
  delay(10); // Small delay to prevent watchdog issues
}

void setupWiFi() {
  Serial.print("Connecting to WiFi");
  carrier.display.setCursor(20, 80);
  carrier.display.println("Connecting WiFi...");
  
  WiFi.begin(ssid, password);
  
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
    
    carrier.display.setCursor(20, 100);
    carrier.display.println("WiFi Connected!");
  } else {
    Serial.println("WiFi connection failed!");
    carrier.display.setCursor(20, 100);
    carrier.display.println("WiFi Failed!");
  }
}

void setupMQTT() {
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(processMQTTMessage);
  
  carrier.display.setCursor(20, 120);
  carrier.display.println("Setting up MQTT...");
  
  reconnectMQTT();
}

void reconnectMQTT() {
  while (!mqttClient.connected() && wifiConnected) {
    Serial.print("Attempting MQTT connection...");
    
    if (mqttClient.connect(device_id)) {
      mqttConnected = true;
      Serial.println("MQTT connected!");
      
      // Subscribe to config topic
      mqttClient.subscribe(configTopic.c_str());
      
      // Publish ready message
      StaticJsonDocument<200> readyMsg;
      readyMsg["device_id"] = device_id;
      readyMsg["status"] = "ready";
      readyMsg["timestamp"] = millis();
      
      String readyJson;
      serializeJson(readyMsg, readyJson);
      mqttClient.publish(telemetryTopic.c_str(), readyJson.c_str());
      
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void calibrateSensors() {
  Serial.println("Calibrating sensors...");
  carrier.display.fillScreen(0x0000);
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(20, 20);
  carrier.display.println("Calibrating...");
  carrier.display.setCursor(20, 40);
  carrier.display.println("Keep device still");
  
  // Show progress indicator
  carrier.leds.fill(colorYellow, 0, 5);
  carrier.leds.show();
  
  float temp_accel_x = 0, temp_accel_y = 0, temp_accel_z = 0;
  float temp_gyro_x = 0, temp_gyro_y = 0, temp_gyro_z = 0;
  
  const int samples = 100;
  
  for (int i = 0; i < samples; i++) {
    float ax, ay, az, gx, gy, gz;
    
    if (carrier.IMUmodule.accelerationAvailable()) {
      carrier.IMUmodule.readAcceleration(ax, ay, az);
      temp_accel_x += ax;
      temp_accel_y += ay;
      temp_accel_z += az;
    }
    
    if (carrier.IMUmodule.gyroscopeAvailable()) {
      carrier.IMUmodule.readGyroscope(gx, gy, gz);
      temp_gyro_x += gx;
      temp_gyro_y += gy;
      temp_gyro_z += gz;
    }
    
    // Show progress
    if (i % 20 == 0) {
      carrier.display.setCursor(20, 80);
      carrier.display.print("Progress: ");
      carrier.display.print(i * 100 / samples);
      carrier.display.println("%");
    }
    
    delay(50);
  }
  
  // Calculate offsets
  accel_offset_x = temp_accel_x / samples;
  accel_offset_y = temp_accel_y / samples;
  accel_offset_z = temp_accel_z / samples - 1.0; // Subtract gravity (1g)
  gyro_offset_x = temp_gyro_x / samples;
  gyro_offset_y = temp_gyro_y / samples;
  gyro_offset_z = temp_gyro_z / samples;
  
  calibrationComplete = true;
  carrier.leds.fill(colorGreen, 0, 5);
  carrier.leds.show();
  delay(1000);
  carrier.leds.fill(0, 0, 5);
  carrier.leds.show();
  
  Serial.println("Calibration complete!");
  // Print calibration results
  Serial.print("Accel offsets: ");
  Serial.print(accel_offset_x, 3); Serial.print(", ");
  Serial.print(accel_offset_y, 3); Serial.print(", ");
  Serial.println(accel_offset_z, 3);
  Serial.print("Gyro offsets: ");
  Serial.print(gyro_offset_x, 3); Serial.print(", ");
  Serial.print(gyro_offset_y, 3); Serial.print(", ");
  Serial.println(gyro_offset_z, 3);
}

/* === CONFIGURATION INSTRUCTIONS ===
 * 
 * Before uploading, please update these settings:
 * 
 * 1. WiFi Settings (lines 16-17):
 *    - Replace "YOUR_WIFI_SSID" with your WiFi network name
 *    - Replace "YOUR_WIFI_PASSWORD" with your WiFi password
 * 
 * 2. MQTT Settings (line 18):
 *    - Default uses free HiveMQ broker: "broker.hivemq.com"
 *    - For production, use your own MQTT broker
 * 
 * 3. Device ID (line 20):
 *    - Change "opla-01" to a unique identifier for your device
 * 
 * 4. Testing the system:
 *    - Monitor Serial output for debug information
 *    - Use MQTT client to subscribe to topics:
 *      * devices/opla-01/telemetry (sensor data every 5 seconds)
 *      * devices/opla-01/events (driving events when detected)
 *    - Send configuration updates to: devices/opla-01/config
 * 
 * 5. Example configuration message (send to config topic):
 *    {
 *      "thresholds": {
 *        "alert_accel": 3.0,
 *        "critical_accel": 4.5,
 *        "hard_braking": 3.0,
 *        "sharp_turn": 120.0,
 *        "sampling_rate": 200,
 *        "telemetry_interval": 10000
 *      }
 *    }
 * 
 * === DEMONSTRATION GUIDE ===
 * 
 * 1. Setup:
 *    - Power on the device
 *    - Wait for WiFi and MQTT connection
 *    - Keep device still during calibration
 * 
 * 2. Testing events:
 *    - Tilt device forward quickly (hard braking simulation)
 *    - Tilt device backward quickly (hard acceleration)
 *    - Rotate device left/right quickly (sharp turn)
 *    - Shake device (bump/impact simulation)
 * 
 * 3. Monitoring:
 *    - Watch display for real-time sensor readings
 *    - Observe LED alerts and buzzer warnings
 *    - Check MQTT messages in your broker/client
 * 
 * 4. Calibration:
 *    - Press center button (TOUCH2) to recalibrate sensors
 *    - Keep device still during recalibration process
 */




/* ========================================
 * VEHICLE DRIVING QUALITY MONITOR v1.0
 * ========================================
 * 
 * This Arduino sketch transforms your MKR IoT Carrier into a comprehensive
 * vehicle driving quality monitoring system. The device continuously monitors
 * acceleration, gyroscope, and environmental sensors to detect and alert on
 * risky driving behaviors.
 * 
 * Key Features:
 * - Real-time sensor monitoring and calibration
 * - Event detection: hard braking, acceleration, sharp turns, bumps
 * - WiFi connectivity with MQTT telemetry publishing
 * - Visual, audio, and remote alerts
 * - Configurable thresholds via MQTT
 * - Comprehensive logging and event tracking
 * 
 * Hardware Requirements:
 * - Arduino MKR IoT Carrier (with built-in sensors)
 * - Stable power supply for continuous operation
 * 
 * The system is designed for educational/demonstration purposes and provides
 * a foundation for building more advanced vehicle monitoring solutions.
 * 
 * For more information, see README.md
 * ========================================
 */

void readSensors() {
  // Read IMU data
  if (carrier.IMUmodule.accelerationAvailable()) {
    carrier.IMUmodule.readAcceleration(sensorData.accel_x, sensorData.accel_y, sensorData.accel_z);
    // Apply calibration offsets
    sensorData.accel_x -= accel_offset_x;
    sensorData.accel_y -= accel_offset_y;
    sensorData.accel_z -= accel_offset_z;
  }
  
  if (carrier.IMUmodule.gyroscopeAvailable()) {
    carrier.IMUmodule.readGyroscope(sensorData.gyro_x, sensorData.gyro_y, sensorData.gyro_z);
    // Apply calibration offsets and convert to degrees/second
    sensorData.gyro_x = (sensorData.gyro_x - gyro_offset_x) * 180.0 / PI;
    sensorData.gyro_y = (sensorData.gyro_y - gyro_offset_y) * 180.0 / PI;
    sensorData.gyro_z = (sensorData.gyro_z - gyro_offset_z) * 180.0 / PI;
  }
  
  // Read environmental sensors
  sensorData.temperature = carrier.Env.readTemperature();
  sensorData.humidity = carrier.Env.readHumidity();
  sensorData.pressure = carrier.Pressure.readPressure();
  
  // Read light sensor (estimate brightness from RGB)
  int r = 0, g = 0, b = 0;
  if (carrier.Light.colorAvailable()) {
    carrier.Light.readColor(r, g, b);
    sensorData.light_level = (r + g + b) / 3; // Simple brightness estimate
  } else {
    sensorData.light_level = 0;
  }
  
  // Simulate microphone level (Arduino Opla doesn't have direct mic access)
  sensorData.mic_level = random(0, 100) / 100.0; // Placeholder
}

void detectVehicleEvents() {
  if (!calibrationComplete) return;
  
  // Calculate acceleration magnitude
  float accel_magnitude = sqrt(sensorData.accel_x * sensorData.accel_x + 
                              sensorData.accel_y * sensorData.accel_y + 
                              sensorData.accel_z * sensorData.accel_z);
  
  // Calculate angular rate magnitude
  float gyro_magnitude = sqrt(sensorData.gyro_x * sensorData.gyro_x + 
                             sensorData.gyro_y * sensorData.gyro_y + 
                             sensorData.gyro_z * sensorData.gyro_z);
  
  // Detect hard braking (negative X acceleration)
  if (sensorData.accel_x < -thresholds.hard_braking) {
    String level = (abs(sensorData.accel_x) > thresholds.critical_accel) ? "critical" : "alert";
    publishEvent("hard_braking", level, "Hard braking detected");
    triggerAlert("HARD BRAKING", level);
  }
  
  // Detect hard acceleration (positive X acceleration)
  else if (sensorData.accel_x > thresholds.alert_accel) {
    String level = (sensorData.accel_x > thresholds.critical_accel) ? "critical" : "alert";
    publishEvent("hard_acceleration", level, "Hard acceleration detected");
    triggerAlert("HARD ACCEL", level);
  }
  
  // Detect sharp turns (high angular velocity on Z axis)
  if (abs(sensorData.gyro_z) > thresholds.sharp_turn) {
    String direction = (sensorData.gyro_z > 0) ? "left" : "right";
    publishEvent("sharp_turn", "alert", "Sharp " + direction + " turn detected");
    triggerAlert("SHARP TURN", "alert");
  }
  
  // Detect bumps/impacts (sudden acceleration spikes)
  if (accel_magnitude > thresholds.bump_impact) {
    publishEvent("bump_impact", "alert", "Road bump or impact detected");
    triggerAlert("BUMP/IMPACT", "alert");
  }
  
  // General driving quality warnings
  else if (accel_magnitude > thresholds.caution_accel) {
    // Only show visual warning, don't publish event for minor issues
    carrier.leds.setPixelColor(0, colorYellow);
    carrier.leds.setPixelColor(4, colorYellow);
    carrier.leds.show();
    delay(100);
    carrier.leds.fill(0, 0, 5);
    carrier.leds.show();
  }
}

void publishTelemetry() {
  if (!mqttConnected) return;
  
  StaticJsonDocument<400> doc;
  doc["device_id"] = device_id;
  doc["timestamp"] = millis();
  
  JsonObject values = doc.createNestedObject("values");
  values["accel_x"] = round(sensorData.accel_x * 100) / 100.0;
  values["accel_y"] = round(sensorData.accel_y * 100) / 100.0;
  values["accel_z"] = round(sensorData.accel_z * 100) / 100.0;
  values["gyro_x"] = round(sensorData.gyro_x * 100) / 100.0;
  values["gyro_y"] = round(sensorData.gyro_y * 100) / 100.0;
  values["gyro_z"] = round(sensorData.gyro_z * 100) / 100.0;
  values["temperature"] = round(sensorData.temperature * 10) / 10.0;
  values["humidity"] = round(sensorData.humidity * 10) / 10.0;
  values["pressure"] = round(sensorData.pressure * 10) / 10.0;
  values["light_level"] = sensorData.light_level;
  values["mic_level"] = round(sensorData.mic_level * 100) / 100.0;
  
  String telemetryJson;
  serializeJson(doc, telemetryJson);
  
  mqttClient.publish(telemetryTopic.c_str(), telemetryJson.c_str());
  
  Serial.println("Telemetry published: " + telemetryJson);
}

void publishEvent(String eventType, String level, String description) {
  if (!mqttConnected) return;
  
  StaticJsonDocument<300> doc;
  doc["device_id"] = device_id;
  doc["timestamp"] = millis();
  doc["type"] = eventType;
  doc["level"] = level;
  doc["description"] = description;
  
  JsonObject values = doc.createNestedObject("values");
  values["accel_x"] = round(sensorData.accel_x * 100) / 100.0;
  values["accel_y"] = round(sensorData.accel_y * 100) / 100.0;
  values["accel_z"] = round(sensorData.accel_z * 100) / 100.0;
  values["gyro_z"] = round(sensorData.gyro_z * 100) / 100.0;
  values["mic_level"] = round(sensorData.mic_level * 100) / 100.0;
  
  String eventJson;
  serializeJson(doc, eventJson);
  
  mqttClient.publish(eventsTopic.c_str(), eventJson.c_str());
  
  Serial.println("Event published: " + eventJson);
}

void displayStatus() {
  carrier.display.fillScreen(0x0000);
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setTextSize(1);
  
  // Title
  carrier.display.setCursor(60, 5);
  carrier.display.setTextSize(2);
  carrier.display.println("Vehicle Monitor");
  
  // Connection status
  carrier.display.setTextSize(1);
  carrier.display.setCursor(5, 25);
  carrier.display.print("WiFi: ");
  carrier.display.setTextColor(wifiConnected ? 0x07E0 : 0xF800);
  carrier.display.println(wifiConnected ? "Connected" : "Disconnected");
  
  carrier.display.setCursor(5, 35);
  carrier.display.setTextColor(0xFFFF);
  carrier.display.print("MQTT: ");
  carrier.display.setTextColor(mqttConnected ? 0x07E0 : 0xF800);
  carrier.display.println(mqttConnected ? "Connected" : "Disconnected");
  
  // Sensor readings
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setCursor(5, 50);
  carrier.display.print("Accel: ");
  carrier.display.print(sqrt(sensorData.accel_x*sensorData.accel_x + 
                           sensorData.accel_y*sensorData.accel_y + 
                           sensorData.accel_z*sensorData.accel_z), 2);
  carrier.display.println(" m/s2");
  
  carrier.display.setCursor(5, 60);
  carrier.display.print("Gyro: ");
  carrier.display.print(abs(sensorData.gyro_z), 1);
  carrier.display.println(" deg/s");
  
  carrier.display.setCursor(5, 75);
  carrier.display.print("Temp: ");
  carrier.display.print(sensorData.temperature, 1);
  carrier.display.println(" C");
  
  carrier.display.setCursor(5, 85);
  carrier.display.print("Humidity: ");
  carrier.display.print(sensorData.humidity, 1);
  carrier.display.println(" %");
  
  carrier.display.setCursor(5, 95);
  carrier.display.print("Pressure: ");
  carrier.display.print(sensorData.pressure, 1);
  carrier.display.println(" kPa");
  
  // Thresholds
  carrier.display.setCursor(5, 110);
  carrier.display.setTextColor(0x07E0);
  carrier.display.print("Thresholds:");
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setCursor(5, 120);
  carrier.display.print("Alert: ");
  carrier.display.print(thresholds.alert_accel, 1);
  carrier.display.print(" Critical: ");
  carrier.display.println(thresholds.critical_accel, 1);
  
  // Instructions
  carrier.display.setCursor(5, 135);
  carrier.display.setTextColor(0x07E0);
  carrier.display.println("Press center button");
  carrier.display.setCursor(5, 145);
  carrier.display.println("to recalibrate");
  
  // Status indicator LEDs
  if (systemReady && wifiConnected && mqttConnected) {
    carrier.leds.setPixelColor(2, colorGreen); // Center LED green when all OK
  } else {
    carrier.leds.setPixelColor(2, colorRed); // Center LED red when issues
  }
  carrier.leds.show();
}

void triggerAlert(String eventType, String level) {
  Serial.println("ALERT: " + eventType + " (" + level + ")");
  
  // Visual alert on display
  carrier.display.fillScreen(level == "critical" ? 0xF800 : 0xF8E0); // Red for critical, yellow for alert
  carrier.display.setTextColor(0x0000); // Black text
  carrier.display.setTextSize(2);
  carrier.display.setCursor(40, 60);
  carrier.display.println("WARNING!");
  carrier.display.setCursor(20, 90);
  carrier.display.setTextSize(1);
  carrier.display.println(eventType);
  carrier.display.setCursor(20, 110);
  carrier.display.println("Level: " + level);
  
  // LED alert pattern
  uint32_t alertColor = (level == "critical") ? colorRed : colorOrange;
  for (int i = 0; i < 3; i++) {
    carrier.leds.fill(alertColor, 0, 5);
    carrier.leds.show();
    delay(200);
    carrier.leds.fill(0, 0, 5);
    carrier.leds.show();
    delay(200);
  }
  
  // Audio alert
  playWarningTone();
  
  // Show alert for 2 seconds
  delay(2000);
}

void playWarningTone() {
  int warningTone[] = {NOTE_G4, NOTE_G4, NOTE_G4, NOTE_C5};
  int warningDurations[] = {8, 8, 8, 4};
  
  for (int i = 0; i < 4; i++) {
    int duration = 1000 / warningDurations[i];
    carrier.Buzzer.sound(warningTone[i]);
    delay(duration);
    carrier.Buzzer.noSound();
    delay(50);
  }
}

void showSystemStatus() {
  carrier.display.fillScreen(0x0000);
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(20, 20);
  carrier.display.println("System Status");
  
  carrier.display.setTextSize(1);
  carrier.display.setCursor(20, 50);
  carrier.display.print("WiFi: ");
  carrier.display.setTextColor(wifiConnected ? 0x07E0 : 0xF800);
  carrier.display.println(wifiConnected ? "OK" : "FAILED");
  
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setCursor(20, 65);
  carrier.display.print("MQTT: ");
  carrier.display.setTextColor(mqttConnected ? 0x07E0 : 0xF800);
  carrier.display.println(mqttConnected ? "OK" : "FAILED");
  
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setCursor(20, 80);
  carrier.display.print("Calibration: ");
  carrier.display.setTextColor(calibrationComplete ? 0x07E0 : 0xF800);
  carrier.display.println(calibrationComplete ? "OK" : "PENDING");
  
  carrier.display.setTextColor(0xFFFF);
  carrier.display.setCursor(20, 100);
  carrier.display.println("Device ID: " + String(device_id));
  
  carrier.display.setCursor(20, 115);
  carrier.display.println("MQTT Topics:");
  carrier.display.setCursor(20, 125);
  carrier.display.setTextSize(1);
  carrier.display.println("- " + telemetryTopic);
  carrier.display.setCursor(20, 135);
  carrier.display.println("- " + eventsTopic);
  
  if (systemReady && wifiConnected && mqttConnected && calibrationComplete) {
    carrier.display.setCursor(20, 150);
    carrier.display.setTextColor(0x07E0);
    carrier.display.println("READY FOR MONITORING!");
    
    // Success LED pattern
    for (int i = 0; i < 2; i++) {
      carrier.leds.fill(colorGreen, 0, 5);
      carrier.leds.show();
      delay(500);
      carrier.leds.fill(0, 0, 5);
      carrier.leds.show();
      delay(200);
    }
  }
  
  delay(3000);
}

void processMQTTMessage(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("MQTT message received on topic: " + String(topic));
  Serial.println("Message: " + message);
  
  // Process configuration updates
  if (String(topic) == configTopic) {
    StaticJsonDocument<300> doc;
    deserializeJson(doc, message);
    
    if (doc.containsKey("thresholds")) {
      JsonObject thresholdObj = doc["thresholds"];
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
      
      // Acknowledge configuration update
      StaticJsonDocument<100> ackDoc;
      ackDoc["device_id"] = device_id;
      ackDoc["config_updated"] = true;
      ackDoc["timestamp"] = millis();
      
      String ackJson;
      serializeJson(ackDoc, ackJson);
      mqttClient.publish(telemetryTopic.c_str(), ackJson.c_str());
    }
  }
}