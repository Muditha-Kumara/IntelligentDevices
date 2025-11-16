/**
 * Config.h
 * Configuration and constants for Vehicle Monitoring System
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ========================================
// WiFi Configuration
// ========================================
extern const char* WIFI_SSID;
extern const char *WIFI_PASSWORD;
extern const char* DEVICE_ID;

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
const uint16_t DISPLAY_YELLOW = 0xFFE0;

// Additional vibrant colors for UI
const uint16_t COLOR_CYAN = 0x07FF;    // Bright cyan
const uint16_t COLOR_MAGENTA = 0xF81F; // Magenta
const uint16_t COLOR_PURPLE = 0x780F;  // Purple
const uint16_t COLOR_PINK = 0xFE19;    // Pink
const uint16_t COLOR_LIME = 0x07E0;    // Lime green
const uint16_t COLOR_TEAL = 0x0410;    // Teal
const uint16_t COLOR_NAVY = 0x000F;    // Navy blue
const uint16_t COLOR_MAROON = 0x7800;  // Maroon
const uint16_t COLOR_WHITE = 0xFFFF;   // White

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
  // Acceleration thresholds (m/s²)
  float alert_accel = 0.4;    // Alert level acceleration
  float critical_accel = 0.8; // Critical level acceleration
  float hard_braking = 0.5;   // Ha0rd braking threshold
  float sharp_turn = 0.4;     // Sharp turn threshold (m/s²)
  float bump_impact = 2.0;    // Bump/impact threshold

  // Sampling and telemetry intervals
  int sampling_rate = 100;       // Sensor sampling rate (ms)
  int telemetry_interval = 2000; // Telemetry send interval (ms)
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
