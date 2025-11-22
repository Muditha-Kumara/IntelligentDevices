/**
 * AlertManager.h
 * Handles alert notifications (visual, audio, etc.)
 */

#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>
#include "Config.h"
#include "DisplayManager.h"
#include "pitches.h"

class AlertManager
{
public:
  AlertManager(MKRIoTCarrier &carrier, DisplayManager &display);

  // Refactored: Accept sensor data and send alert in one call
  void triggerAlert(String eventType, String level, const VehicleData &data);
  
  // Handle critical stop warning from cloud response
  void triggerCriticalStopWarning();

private:
  MKRIoTCarrier& carrier;
  DisplayManager& display;

  void playWarningTone();
  void playLEDPattern(String level);
  void sendAlertData(const VehicleData &data, const String &eventType, const String &level);
};

#endif // ALERT_MANAGER_H
