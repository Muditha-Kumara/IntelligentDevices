/**
 * DisplayManager.h
 * Handles all display and LED operations
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include "Config.h"

class DisplayManager {
public:
  DisplayManager(MKRIoTCarrier& carrier);
  void begin();
  void showInitializing();
  void showCalibrating(int progress);
  void showSystemStatus(bool wifiConnected, bool mqttConnected, bool calibrationComplete);
  void showMonitoringStatus(const VehicleData& data, const Thresholds& thresholds, 
                            bool wifiConnected, bool mqttConnected, bool systemReady);
  void showAlert(String eventType, String level);
  void updateStatusLED(bool systemReady, bool wifiConnected, bool mqttConnected);
  void setLEDColor(uint32_t color);
  void clearLEDs();
  void flashLEDs(uint32_t color, int times, int delayMs);

  // New UI methods for round display
  void drawMonitoringUI();
  void updateAccelValue(float value);
  void updateGyroValue(float value);
  void updateTempValue(float value);
  void updateHumidityValue(float value);
  void updatePressureValue(float value);
  void updateWarningMsg(const String& msg);

private:
  MKRIoTCarrier& carrier;
};

#endif // DISPLAY_MANAGER_H
