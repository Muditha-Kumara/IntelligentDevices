/**
 * VehicleMonitor.h
 * Core vehicle monitoring logic
 */

#ifndef VEHICLE_MONITOR_H
#define VEHICLE_MONITOR_H

#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include "Config.h"
// ...existing code...
#include "AlertManager.h"

class VehicleMonitor {
public:
  VehicleMonitor(MKRIoTCarrier &carrier, AlertManager &alert);

  void calibrateSensors();
  void readSensors();
  void detectVehicleEvents();
  
  VehicleData getSensorData() const { return sensorData; }
  Thresholds& getThresholds() { return thresholds; }
  bool isCalibrationComplete() const { return calibration.complete; }
  bool isWarningActive() const { return warningActive; }
  String getLastWarningType() const { return lastWarningType; }
  String getLastWarningLevel() const { return lastWarningLevel; }
  unsigned long getWarningEndTime() const { return warningEndTime; }
  void clearWarning() { warningActive = false; lastWarningType = ""; lastWarningLevel = ""; }
  
private:
  MKRIoTCarrier &carrier;
  AlertManager& alert;

  VehicleData sensorData;
  Thresholds thresholds;
  CalibrationData calibration;

  bool warningActive = false;
  unsigned long warningEndTime = 0;
  String lastWarningType = "";
  String lastWarningLevel = "";
};

#endif // VEHICLE_MONITOR_H
