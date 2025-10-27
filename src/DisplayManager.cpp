/**
 * DisplayManager.cpp
 * Implementation of display and LED operations
 */

#include "DisplayManager.h"

DisplayManager::DisplayManager(MKRIoTCarrier& carrier) : carrier(carrier) {
}

void DisplayManager::begin() {
  carrier.display.fillScreen(DISPLAY_BLACK);
  carrier.display.setRotation(0);
  carrier.display.setTextWrap(true);
}

void DisplayManager::showInitializing() {
  carrier.display.fillScreen(DISPLAY_BLACK);
  carrier.display.setTextColor(DISPLAY_WHITE);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(30, 75);
  carrier.display.println("Vehicle Monitor");
  carrier.display.setCursor(30, 150);
  carrier.display.println("Initializing...");
}

void DisplayManager::showCalibrating(int progress) {
  carrier.display.fillScreen(DISPLAY_BLACK);
  carrier.display.setTextColor(DISPLAY_WHITE);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(20, 20);
  carrier.display.println("Calibrating...");
  carrier.display.setCursor(20, 40);
  carrier.display.println("Keep device still");
  
  carrier.display.setCursor(20, 80);
  carrier.display.print("Progress: ");
  carrier.display.print(progress);
  carrier.display.println("%");
  
  // Show progress indicator
  carrier.leds.fill(COLOR_YELLOW, 0, 5);
  carrier.leds.show();
}

void DisplayManager::showSystemStatus(bool wifiConnected, bool mqttConnected, bool calibrationComplete) {
  carrier.display.fillScreen(DISPLAY_BLACK);
  carrier.display.setTextColor(DISPLAY_WHITE);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(20, 20);
  carrier.display.println("System Status");
  
  carrier.display.setTextSize(1);
  carrier.display.setCursor(20, 50);
  carrier.display.print("WiFi: ");
  carrier.display.setTextColor(wifiConnected ? DISPLAY_GREEN : DISPLAY_RED);
  carrier.display.println(wifiConnected ? "OK" : "FAILED");
  
  carrier.display.setTextColor(DISPLAY_WHITE);
  carrier.display.setCursor(20, 65);
  carrier.display.print("MQTT: ");
  carrier.display.setTextColor(mqttConnected ? DISPLAY_GREEN : DISPLAY_RED);
  carrier.display.println(mqttConnected ? "OK" : "FAILED");
  
  carrier.display.setTextColor(DISPLAY_WHITE);
  carrier.display.setCursor(20, 80);
  carrier.display.print("Calibration: ");
  carrier.display.setTextColor(calibrationComplete ? DISPLAY_GREEN : DISPLAY_RED);
  carrier.display.println(calibrationComplete ? "OK" : "PENDING");
  
  carrier.display.setTextColor(DISPLAY_WHITE);
  carrier.display.setCursor(20, 100);
  carrier.display.println("Device ID: " + String(DEVICE_ID));
  
  carrier.display.setCursor(20, 115);
  carrier.display.println("MQTT Topics:");
  carrier.display.setCursor(20, 125);
  carrier.display.setTextSize(1);
  carrier.display.println("- " + TELEMETRY_TOPIC);
  carrier.display.setCursor(20, 135);
  carrier.display.println("- " + EVENTS_TOPIC);
  
  if (wifiConnected && mqttConnected && calibrationComplete) {
    carrier.display.setCursor(20, 150);
    carrier.display.setTextColor(DISPLAY_GREEN);
    carrier.display.println("READY FOR MONITORING!");
    
    // Success LED pattern
    for (int i = 0; i < 2; i++) {
      carrier.leds.fill(COLOR_GREEN, 0, 5);
      carrier.leds.show();
      delay(500);
      carrier.leds.fill(0, 0, 5);
      carrier.leds.show();
      delay(200);
    }
  }
}

void DisplayManager::showMonitoringStatus(const VehicleData& data, const Thresholds& thresholds,
                                          bool wifiConnected, bool mqttConnected, bool systemReady) {
  carrier.display.fillScreen(DISPLAY_BLACK);
  carrier.display.setTextColor(DISPLAY_WHITE);
  carrier.display.setTextSize(1);
  
  // Title
  carrier.display.setCursor(60, 5);
  carrier.display.setTextSize(2);
  carrier.display.println("Vehicle Monitor");
  
  // Connection status
  carrier.display.setTextSize(1);
  carrier.display.setCursor(5, 25);
  carrier.display.print("WiFi: ");
  carrier.display.setTextColor(wifiConnected ? DISPLAY_GREEN : DISPLAY_RED);
  carrier.display.println(wifiConnected ? "Connected" : "Disconnected");
  
  carrier.display.setCursor(5, 35);
  carrier.display.setTextColor(DISPLAY_WHITE);
  carrier.display.print("MQTT: ");
  carrier.display.setTextColor(mqttConnected ? DISPLAY_GREEN : DISPLAY_RED);
  carrier.display.println(mqttConnected ? "Connected" : "Disconnected");
  
  // Sensor readings
  carrier.display.setTextColor(DISPLAY_WHITE);
  carrier.display.setCursor(5, 50);
  carrier.display.print("Accel: ");
  carrier.display.print(sqrt(data.accel_x * data.accel_x + 
                           data.accel_y * data.accel_y + 
                           data.accel_z * data.accel_z), 2);
  carrier.display.println(" m/s2");
  
  carrier.display.setCursor(5, 60);
  carrier.display.print("Gyro: ");
  carrier.display.print(abs(data.gyro_z), 1);
  carrier.display.println(" deg/s");
  
  carrier.display.setCursor(5, 75);
  carrier.display.print("Temp: ");
  carrier.display.print(data.temperature, 1);
  carrier.display.println(" C");
  
  carrier.display.setCursor(5, 85);
  carrier.display.print("Humidity: ");
  carrier.display.print(data.humidity, 1);
  carrier.display.println(" %");
  
  carrier.display.setCursor(5, 95);
  carrier.display.print("Pressure: ");
  carrier.display.print(data.pressure, 1);
  carrier.display.println(" kPa");
  
  // Thresholds
  carrier.display.setCursor(5, 110);
  carrier.display.setTextColor(DISPLAY_GREEN);
  carrier.display.print("Thresholds:");
  carrier.display.setTextColor(DISPLAY_WHITE);
  carrier.display.setCursor(5, 120);
  carrier.display.print("Alert: ");
  carrier.display.print(thresholds.alert_accel, 1);
  carrier.display.print(" Critical: ");
  carrier.display.println(thresholds.critical_accel, 1);
  
  // Instructions
  carrier.display.setCursor(5, 135);
  carrier.display.setTextColor(DISPLAY_GREEN);
  carrier.display.println("Press center button");
  carrier.display.setCursor(5, 145);
  carrier.display.println("to recalibrate");
}

void DisplayManager::showAlert(String eventType, String level) {
  carrier.display.setTextSize(2);
  carrier.display.fillRect(90, 200, 200, 50, DISPLAY_BLACK);
  carrier.display.setCursor(90, 200);
  carrier.display.setTextColor(DISPLAY_YELLOW);
  carrier.display.println(eventType);
}

void DisplayManager::updateStatusLED(bool systemReady, bool wifiConnected, bool mqttConnected) {
  if (systemReady && wifiConnected && mqttConnected) {
    carrier.leds.setPixelColor(2, COLOR_GREEN);
  } else {
    carrier.leds.setPixelColor(2, COLOR_RED);
  }
  carrier.leds.show();
}

void DisplayManager::setLEDColor(uint32_t color) {
  carrier.leds.fill(color, 0, 5);
  carrier.leds.show();
}

void DisplayManager::clearLEDs() {
  carrier.leds.fill(0, 0, 5);
  carrier.leds.show();
}

void DisplayManager::flashLEDs(uint32_t color, int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    carrier.leds.fill(color, 0, 5);
    carrier.leds.show();
    delay(delayMs);
    carrier.leds.fill(0, 0, 5);
    carrier.leds.show();
    delay(delayMs);
  }
}

void DisplayManager::drawMonitoringUI() {
  carrier.display.fillScreen(DISPLAY_BLACK);
  carrier.display.setTextColor(DISPLAY_WHITE);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(60, 10);
  carrier.display.println("Vehicle Monitor");

  carrier.display.setTextSize(2);
  carrier.display.setCursor(30, 50);
  carrier.display.println("Accel:");
  // carrier.display.setCursor(30, 80);
  // carrier.display.println("Gyro:");
  carrier.display.setCursor(30, 110);
  carrier.display.println("Temp:");
  carrier.display.setCursor(30, 140);
  carrier.display.println("Humidity:");
  carrier.display.setCursor(30, 170);
  carrier.display.println("Pressure:");

  // Reserve area for warning message at bottom
  carrier.display.setTextSize(2);
  carrier.display.setCursor(40, 200);
  carrier.display.setTextColor(DISPLAY_YELLOW);
  carrier.display.println("Msg:");
}

void DisplayManager::updateWarningMsg(const String& msg) {
  
}

void DisplayManager::updateAccelValue(float x, float y, float z)
{
  carrier.display.setTextColor(DISPLAY_GREEN);
  carrier.display.setTextSize(2);
  carrier.display.fillRect(00, 80, 240, 50, DISPLAY_BLACK); // Clear previous value area for three rows
  carrier.display.setCursor(5, 80);
  carrier.display.print("X:");
  carrier.display.print(x, 2);
  carrier.display.setCursor(80, 80);
  carrier.display.print("Y:");
  carrier.display.print(y, 2);
  carrier.display.setCursor(160, 80);
  carrier.display.print("Z:");
  carrier.display.print(z, 2);
  carrier.display.setCursor(180, 50);
  carrier.display.print("m/s2"); // Show unit only once at the end
}

void DisplayManager::updateGyroValue(float value) {
  carrier.display.setTextColor(DISPLAY_GREEN);
  carrier.display.setTextSize(2);
  carrier.display.fillRect(160, 80, 80, 25, DISPLAY_BLACK);
  carrier.display.setCursor(160, 80);
  carrier.display.print(value, 1);
  carrier.display.print(" deg/s");
}

void DisplayManager::updateTempValue(float value) {
  carrier.display.setTextColor(DISPLAY_GREEN);
  carrier.display.setTextSize(2);
  carrier.display.fillRect(160, 110, 80, 25, DISPLAY_BLACK);
  carrier.display.setCursor(160, 110);
  carrier.display.print(value, 1);
  carrier.display.print(" C");
}

void DisplayManager::updateHumidityValue(float value) {
  carrier.display.setTextColor(DISPLAY_GREEN);
  carrier.display.setTextSize(2);
  carrier.display.fillRect(160, 140, 80, 25, DISPLAY_BLACK);
  carrier.display.setCursor(160, 140);
  carrier.display.print(value, 1);
  carrier.display.print(" %");
}

void DisplayManager::updatePressureValue(float value) {
  carrier.display.setTextColor(DISPLAY_GREEN);
  carrier.display.setTextSize(2);
  carrier.display.fillRect(160, 170, 80, 25, DISPLAY_BLACK);
  carrier.display.setCursor(160, 170);
  carrier.display.print(value, 1);
  carrier.display.print(" kPa");
}
