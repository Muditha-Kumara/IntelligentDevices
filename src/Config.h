/**
 * Config.h
 * Configuration and constants for Vehicle Monitoring System
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ========================================
// WiFi and MQTT Configuration
// ========================================
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;
extern const char* MQTT_SERVER;
extern const int MQTT_PORT;
extern const char* DEVICE_ID;

// ========================================
// MQTT Topics
// ========================================
const String TELEMETRY_TOPIC = "devices/" + String(DEVICE_ID) + "/telemetry";
const String EVENTS_TOPIC = "devices/" + String(DEVICE_ID) + "/events";
const String CONFIG_TOPIC = "devices/" + String(DEVICE_ID) + "/config";

// ========================================
// LED Colors
// ========================================
const uint32_t COLOR_RED = 0xC80000;       // RGB(200, 0, 0)
const uint32_t COLOR_GREEN = 0x00C800;     // RGB(0, 200, 0)
const uint32_t COLOR_BLUE = 0x0000C8;      // RGB(0, 0, 200)
const uint32_t COLOR_YELLOW = 0xC8C800;    // RGB(200, 200, 0)
const uint32_t COLOR_ORANGE = 0xFFA500;    // RGB(255, 165, 0)

// ========================================
// Display Colors (16-bit RGB565)
// ========================================
const uint16_t DISPLAY_BLACK = 0x0000;
const uint16_t DISPLAY_WHITE = 0xFFFF;
const uint16_t DISPLAY_RED = 0xF800;
const uint16_t DISPLAY_GREEN = 0x07E0;
const uint16_t DISPLAY_YELLOW = 0xF8E0;

// ========================================
// Data Structures
// ========================================
struct VehicleData {
  float accel_x, accel_y, accel_z;
  float gyro_x, gyro_y, gyro_z;
  float temperature, humidity, pressure;
  float mic_level;
  int light_level;
};

struct Thresholds {
  float caution_accel = 2.0;      // m/s²
  float alert_accel = 3.5;        // m/s²
  float critical_accel = 5.0;     // m/s²
  float hard_braking = 3.5;       // m/s²
  float sharp_turn = 150.0;       // degrees/s
  float bump_impact = 6.0;        // m/s²
  int sampling_rate = 100;        // ms
  int telemetry_interval = 2000;  // ms
};

struct CalibrationData {
  float accel_offset_x = 0;
  float accel_offset_y = 0;
  float accel_offset_z = 0;
  float gyro_offset_x = 0;
  float gyro_offset_y = 0;
  float gyro_offset_z = 0;
  bool complete = false;
};

#endif // CONFIG_H
