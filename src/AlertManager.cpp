/**
 * AlertManager.cpp
 * Implementation of alert notifications
 */

#include "AlertManager.h"

AlertManager::AlertManager(MKRIoTCarrier& carrier, DisplayManager& display) 
  : carrier(carrier), display(display) {
}

void AlertManager::triggerAlert(String eventType, String level) {
  Serial.println("ALERT: " + eventType + " (" + level + ")");

  // Show alert indicator in bottom bar (doesn't reset UI)
  display.updateEventIndicator(eventType, level);

  // Play LED pattern
  playLEDPattern(level);
  
  // Play audio alert
  playWarningTone();

  // No delay - let the main loop handle timing
}

void AlertManager::playLEDPattern(String level) {
  uint32_t alertColor = (level == "critical") ? COLOR_RED : COLOR_ORANGE;
  
  for (int i = 0; i < 3; i++) {
    carrier.leds.fill(alertColor, 0, 5);
    carrier.leds.show();
    delay(200);
    carrier.leds.fill(0, 0, 5);
    carrier.leds.show();
    delay(200);
  }
}

void AlertManager::playWarningTone() {
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
