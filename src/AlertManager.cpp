/**
 * AlertManager.cpp
 * Implementation of alert notifications
 */

#include "AlertManager.h"

AlertManager::AlertManager(MKRIoTCarrier& carrier, DisplayManager& display) 
  : carrier(carrier), display(display) {
}

// Refactored: Accept sensor data and send alert in one call
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

void AlertManager::triggerAlert(String eventType, String level, const VehicleData &data)
{
  Serial.println("ALERT: " + eventType + " (" + level + ")");

  // Show alert indicator in bottom bar (doesn't reset UI)
  display.updateEventIndicator(eventType, level);

  // Play LED pattern
  playLEDPattern(level);

  // Play audio alert
  playWarningTone();

  // Send sensor data to remote URL
  sendAlertData(data, eventType, level);
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

void AlertManager::playWarningTone()
{
  int warningTone[] = {NOTE_G4, NOTE_G4, NOTE_G4, NOTE_C5};
  int warningDurations[] = {8, 8, 8, 4};

  for (int i = 0; i < 4; i++)
  {
    int duration = 1000 / warningDurations[i];
    carrier.Buzzer.sound(warningTone[i]);
    delay(duration);
    carrier.Buzzer.noSound();
    delay(50);
  }
}

// Send sensor data to remote URL as JSON
void AlertManager::sendAlertData(const VehicleData &data, const String &eventType, const String &level)
{
  // Use server and URL from Config.h for best practice
  extern const char *URL;
  const int port = 443;
  String urlPath;
  String server;
  // Parse server and path from URL
  String urlStr = String(URL);
  int idx = urlStr.indexOf("/", 8); // after https://
  if (idx > 0)
  {
    server = urlStr.substring(8, idx); // skip https://
    urlPath = urlStr.substring(idx);
  }
  else
  {
    server = urlStr.substring(8);
    urlPath = "/";
  }

  WiFiSSLClient client;
  HttpClient http(client, server, port);

  // Build VehicleEvent JSON expected by the Lambda
  // { device_id, event_type, level, timestamp, data: { ... } }
  String timestamp = String(millis()); // Fallback timestamp (ms since boot). Replace with real RTC if available.

  String jsonPayload = "{";
  jsonPayload += "\"device_id\":\"" + String(DEVICE_ID) + "\",";
  jsonPayload += "\"event_type\":\"" + eventType + "\",";
  jsonPayload += "\"level\":\"" + level + "\",";
  jsonPayload += "\"timestamp\":\"" + timestamp + "\",";
  jsonPayload += "\"data\":{";
  jsonPayload += "\"accel_x\":" + String(data.accel_x, 3) + ",";
  jsonPayload += "\"accel_y\":" + String(data.accel_y, 3) + ",";
  jsonPayload += "\"accel_z\":" + String(data.accel_z, 3) + ",";
  jsonPayload += "\"gyro_x\":" + String(data.gyro_x, 3) + ",";
  jsonPayload += "\"gyro_y\":" + String(data.gyro_y, 3) + ",";
  jsonPayload += "\"gyro_z\":" + String(data.gyro_z, 3) + ",";
  jsonPayload += "\"temperature\":" + String(data.temperature, 2) + ",";
  jsonPayload += "\"humidity\":" + String(data.humidity, 2) + ",";
  jsonPayload += "\"pressure\":" + String(data.pressure, 2) + ",";
  jsonPayload += "\"mic_level\":" + String(data.mic_level, 2) + ",";
  jsonPayload += "\"light_level\":" + String(data.light_level);
  jsonPayload += "}}";

  // Send with one retry on failure
  const int maxAttempts = 2;
  int attempt = 0;
  int statusCode = -1;
  String response;

  while (attempt < maxAttempts)
  {
    attempt++;
    http.beginRequest();
    http.post(urlPath, "application/json", jsonPayload);
    http.endRequest();

    statusCode = http.responseStatusCode();
    response = http.responseBody();

    if (statusCode >= 200 && statusCode < 300)
      break; // success
    Serial.print("Alert POST failed (attempt ");
    Serial.print(attempt);
    Serial.print(") status=");
    Serial.println(statusCode);
    delay(200); // short backoff
  }

  Serial.print("Alert data sent. Status: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}
