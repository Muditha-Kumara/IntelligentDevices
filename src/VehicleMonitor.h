/**
 * VehicleMonitor.h
 * Core vehicle monitoring logic
 */

#ifndef VEHICLE_MONITOR_H
#define VEHICLE_MONITOR_H

#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include "Config.h"
#include "NetworkManager.h"
#include "AlertManager.h"

class VehicleMonitor {
public:
  VehicleMonitor(MKRIoTCarrier& carrier, NetworkManager& network, AlertManager& alert);
  
  void calibrateSensors();
  void readSensors();
  void detectVehicleEvents();
  
  VehicleData getSensorData() const { return sensorData; }
  Thresholds& getThresholds() { return thresholds; }
  bool isCalibrationComplete() const { return calibration.complete; }
  
private:
  MKRIoTCarrier& carrier;
  NetworkManager& network;
  AlertManager& alert;
  
  VehicleData sensorData;
  Thresholds thresholds;
  CalibrationData calibration;
};

#endif // VEHICLE_MONITOR_H
