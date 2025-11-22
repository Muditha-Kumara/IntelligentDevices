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
  void showSystemStatus(bool wifiConnected, bool calibrationComplete);
  void showMonitoringStatus(const VehicleData& data, const Thresholds& thresholds, 
                            bool wifiConnected, bool mqttConnected, bool systemReady);
  void showAlert(String eventType, String level);
  void showCriticalStopWarning();
  void updateStatusLED(bool systemReady, bool wifiConnected);
  void setLEDColor(uint32_t color);
  void clearLEDs();
  void flashLEDs(uint32_t color, int times, int delayMs);

  // Enhanced UI methods - efficient partial updates
  void drawInitialUI();
  void updateAccelGauge(float x, float y, float z);
  void updateTempGauge(float temp);
  void updateSensorValues(const VehicleData &data);
  void updateConnectionStatus(bool wifiConnected, bool mqttConnected);
  void updateEventIndicator(const String &eventType, const String &level);
  void clearEventIndicator();

  // Boot and calibration
  void animateBootSequence();
  void showCalibrationProgress(int progress);

  // Helper drawing functions
  void drawCarIcon(int x, int y, uint16_t color);
  void drawProgressBar(int y, int progress, uint16_t color);
  void drawGauge(int centerX, int centerY, int radius, float value, float maxValue, uint16_t color);
  void drawStaticUI();

private:
  MKRIoTCarrier& carrier;
  // Cache last values to avoid unnecessary redraws
  float lastAccelMagnitude = -1;
  float lastTemp = -999;
  float lastHumidity = -999;
  float lastPressure = -999;
  bool lastWifiStatus = false;
  bool lastMqttStatus = false;
};

#endif // DISPLAY_MANAGER_H
