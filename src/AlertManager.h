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

class AlertManager {
public:
  AlertManager(MKRIoTCarrier& carrier, DisplayManager& display);
  
  void triggerAlert(String eventType, String level);
  
private:
  MKRIoTCarrier& carrier;
  DisplayManager& display;
  
  void playWarningTone();
  void playLEDPattern(String level);
};

#endif // ALERT_MANAGER_H
