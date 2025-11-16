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
  animateBootSequence();
}

void DisplayManager::showCalibrationProgress(int progress)
{
  static int lastProgress = -1;

  if (lastProgress < 0)
  {
    // First time - draw everything
    carrier.display.fillScreen(0x0000);
    drawCarIcon(112, 50, COLOR_CYAN);

    carrier.display.setTextSize(2);
    carrier.display.setCursor(45, 100);
    carrier.display.setTextColor(COLOR_CYAN);
    carrier.display.println("CALIBRATE");

    carrier.display.setTextSize(1);
    carrier.display.setCursor(55, 120);
    carrier.display.setTextColor(COLOR_ORANGE);
    carrier.display.println("Keep still");
  }

  // Only update progress bar if changed
  if (progress != lastProgress)
  {
    drawProgressBar(150, progress, COLOR_LIME);
    lastProgress = progress;

    // Update LEDs
    int ledsToLight = progress / 20;
    for (int i = 0; i < 5; i++)
    {
      carrier.leds.setPixelColor(i, i < ledsToLight ? COLOR_YELLOW : 0);
    }
    carrier.leds.show();
  }

  if (progress >= 100)
    lastProgress = -1; // Reset for next time
}

void DisplayManager::showSystemStatus(bool wifiConnected, bool calibrationComplete)
{
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
  carrier.display.setCursor(20, 80);
  carrier.display.print("Calibration: ");
  carrier.display.setTextColor(calibrationComplete ? DISPLAY_GREEN : DISPLAY_RED);
  carrier.display.println(calibrationComplete ? "OK" : "PENDING");

  // ...existing code...
}

void DisplayManager::drawStaticUI()
{
  // Draw static elements that don't change
  // 256x256 display, but use safe area to avoid rounded corners
  carrier.display.fillScreen(0x0000);

  // Top bar - keep within safe area (leave margin for rounded corners)
  carrier.display.fillRoundRect(15, 5, 226, 30, 5, COLOR_NAVY);
  drawCarIcon(40, 8, COLOR_ORANGE);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(80, 12);
  carrier.display.setTextColor(COLOR_CYAN);
  carrier.display.println("VEHICLE");

  // Gauge backgrounds - centered and spaced properly
  carrier.display.drawCircle(75, 95, 35, COLOR_CYAN);
  carrier.display.drawCircle(180, 95, 35, COLOR_ORANGE);

  // Labels under gauges
  carrier.display.setTextSize(1);
  carrier.display.setCursor(62, 135);
  carrier.display.setTextColor(COLOR_WHITE);
  carrier.display.println("Accel");
  carrier.display.setCursor(170, 135);
  carrier.display.println("Temp");

  // Data area labels - compact layout
  carrier.display.setCursor(20, 160);
  carrier.display.setTextColor(COLOR_TEAL);
  carrier.display.print("Humid:");

  carrier.display.setCursor(20, 175);
  carrier.display.setTextColor(COLOR_PURPLE);
  carrier.display.print("Press:");

  // Bottom bar - within safe area
  carrier.display.fillRoundRect(15, 195, 226, 60, 5, COLOR_NAVY);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(50, 205);
  carrier.display.setTextColor(COLOR_LIME);
  carrier.display.println("MONITORING...");
}

void DisplayManager::drawInitialUI()
{
  drawStaticUI();
  lastAccelMagnitude = -1;
  lastTemp = -999;
  lastHumidity = -999;
  lastPressure = -999;
  lastWifiStatus = false;
  lastMqttStatus = false;
}

void DisplayManager::showAlert(String eventType, String level)
{
  // Determine color based on level
  uint16_t alertColor = COLOR_YELLOW;
  if (level == "critical")
  {
    alertColor = COLOR_RED;
  }
  else if (level == "warning")
  {
    alertColor = COLOR_ORANGE;
  }

  // Flash animation
  for (int i = 0; i < 3; i++)
  {
    carrier.display.fillRect(0, 0, 240, 240, alertColor);
    delay(100);
    carrier.display.fillRect(0, 0, 240, 240, 0x0000);
    delay(100);
  }

  // Draw alert box
  carrier.display.fillScreen(0x0000);
  carrier.display.fillRoundRect(20, 70, 200, 100, 10, alertColor);
  carrier.display.fillRoundRect(25, 75, 190, 90, 8, 0x0000);

  // Alert icon (exclamation mark)
  carrier.display.fillCircle(120, 100, 15, alertColor);
  carrier.display.fillRect(117, 90, 6, 15, 0x0000);
  carrier.display.fillCircle(120, 110, 3, 0x0000);

  // Alert text
  carrier.display.setTextSize(2);
  carrier.display.setCursor(50, 130);
  carrier.display.setTextColor(alertColor);
  String upperLevel = level;
  upperLevel.toUpperCase();
  carrier.display.println(upperLevel);

  carrier.display.setTextSize(1);
  carrier.display.setCursor(40, 150);
  carrier.display.setTextColor(COLOR_WHITE);
  carrier.display.println(eventType);

  // Flash LEDs
  flashLEDs(alertColor, 5, 200);
}

void DisplayManager::updateStatusLED(bool systemReady, bool wifiConnected)
{
  if (systemReady && wifiConnected)
  {
    carrier.leds.setPixelColor(2, COLOR_GREEN);
  }
  else
  {
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

// Update acceleration gauge only
void DisplayManager::updateAccelGauge(float x, float y, float z)
{
  float magnitude = sqrt(x * x + y * y + z * z) - 1;

  // Only update if changed significantly
  if (abs(magnitude - lastAccelMagnitude) < 0.1)
    return;
  lastAccelMagnitude = magnitude;

  // Clear gauge area
  carrier.display.fillCircle(75, 95, 33, 0x0000);

  // Draw gauge value
  drawGauge(75, 95, 32, magnitude * 10, 10.0, COLOR_CYAN);

  // Update numeric value
  carrier.display.setTextSize(2);
  carrier.display.fillRect(52, 90, 46, 10, 0x0000);
  carrier.display.setCursor(55, 90);
  carrier.display.setTextColor(COLOR_CYAN);
  carrier.display.print(magnitude, 1);
}

// Update temperature gauge only
void DisplayManager::updateTempGauge(float temp)
{
  if (abs(temp - lastTemp) < 0.5)
    return;
  lastTemp = temp;

  // Clear gauge area
  carrier.display.fillCircle(180, 95, 33, 0x0000);

  // Draw gauge
  drawGauge(180, 95, 32, temp, 50.0, COLOR_ORANGE);

  // Update numeric value
  carrier.display.setTextSize(2);
  carrier.display.fillRect(160, 90, 40, 10, 0x0000);
  carrier.display.setCursor(163, 90);
  carrier.display.setTextColor(COLOR_ORANGE);
  carrier.display.print(temp, 1);
}

// Update sensor values efficiently
void DisplayManager::updateSensorValues(const VehicleData &data)
{
  carrier.display.setTextSize(1);

  // Update humidity if changed - compact display
  if (abs(data.humidity - lastHumidity) > 0.5)
  {
    lastHumidity = data.humidity;
    carrier.display.fillRect(70, 160, 70, 8, 0x0000);
    carrier.display.setCursor(70, 160);
    carrier.display.setTextColor(COLOR_WHITE);
    carrier.display.print(data.humidity, 1);
    carrier.display.print("%");
  }

  // Update pressure if changed - compact display
  if (abs(data.pressure - lastPressure) > 0.1)
  {
    lastPressure = data.pressure;
    carrier.display.fillRect(70, 175, 80, 8, 0x0000);
    carrier.display.setCursor(70, 175);
    carrier.display.setTextColor(COLOR_WHITE);
    carrier.display.print(data.pressure, 1);
    carrier.display.print("kPa");
  }
}

// Update connection status indicators
void DisplayManager::updateConnectionStatus(bool wifiConnected, bool mqttConnected)
{
  // WiFi indicator - positioned safely within top bar
  if (wifiConnected != lastWifiStatus)
  {
    lastWifiStatus = wifiConnected;
    carrier.display.fillCircle(200, 18, 5, wifiConnected ? COLOR_LIME : DISPLAY_RED);
  }

  // MQTT indicator - positioned safely within top bar
  if (mqttConnected != lastMqttStatus)
  {
    lastMqttStatus = mqttConnected;
    carrier.display.fillCircle(220, 18, 5, mqttConnected ? COLOR_LIME : DISPLAY_RED);
  }
}

// Show event indicator in bottom bar
void DisplayManager::updateEventIndicator(const String &eventType, const String &level)
{
  uint16_t color = COLOR_YELLOW;
  if (level == "critical")
    color = DISPLAY_RED;
  else if (level == "alert")
    color = COLOR_ORANGE;

  // Update bottom bar text - compact and centered
  carrier.display.fillRect(25, 195, 200, 60, COLOR_NAVY);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(50, 198);
  carrier.display.setTextColor(color);
  carrier.display.print(eventType);
  carrier.display.setCursor(80, 215);
  carrier.display.setTextColor(COLOR_WHITE);
  carrier.display.print(level);

  // Flash LEDs
  for (int i = 0; i < 5; i++)
  {
    carrier.leds.setPixelColor(i, color == DISPLAY_RED ? COLOR_RED : color == COLOR_ORANGE ? COLOR_ORANGE
                                                                                           : COLOR_YELLOW);
  }
  carrier.leds.show();
}

// Clear event indicator
void DisplayManager::clearEventIndicator()
{
  carrier.display.fillRect(25, 195, 200, 60, COLOR_NAVY);
  carrier.display.setTextSize(2);
  carrier.display.setCursor(50, 205);
  carrier.display.setTextColor(COLOR_LIME);
  carrier.display.println("MONITORING...");

  carrier.leds.clear();
  carrier.leds.show();
}

// Draw car icon
void DisplayManager::drawCarIcon(int x, int y, uint16_t color)
{
  // Simple car body
  carrier.display.fillRoundRect(x, y + 8, 32, 16, 4, color);
  carrier.display.fillRoundRect(x + 6, y, 20, 12, 3, color);

  // Windows
  carrier.display.fillRect(x + 8, y + 2, 6, 6, 0x0000);
  carrier.display.fillRect(x + 18, y + 2, 6, 6, 0x0000);

  // Wheels
  carrier.display.fillCircle(x + 8, y + 24, 4, COLOR_WHITE);
  carrier.display.fillCircle(x + 24, y + 24, 4, COLOR_WHITE);
  carrier.display.fillCircle(x + 8, y + 24, 2, 0x4208);
  carrier.display.fillCircle(x + 24, y + 24, 2, 0x4208);

  // Lights
  carrier.display.fillCircle(x + 2, y + 12, 2, COLOR_YELLOW);
  carrier.display.fillCircle(x + 30, y + 12, 2, COLOR_RED);
}

// Draw progress bar with color
void DisplayManager::drawProgressBar(int y, int progress, uint16_t color)
{
  int barWidth = 180;
  int barHeight = 18;
  int x = (256 - barWidth) / 2; // Center on 256px display

  // Border
  carrier.display.drawRoundRect(x, y, barWidth, barHeight, 5, COLOR_WHITE);

  // Fill
  int fillWidth = (barWidth - 4) * progress / 100;
  carrier.display.fillRoundRect(x + 2, y + 2, fillWidth, barHeight - 4, 3, color);

  // Percentage text
  carrier.display.setTextSize(1);
  carrier.display.setCursor(x + barWidth / 2 - 12, y + 5);
  carrier.display.setTextColor(progress > 50 ? 0x0000 : COLOR_WHITE);
  carrier.display.print(progress);
  carrier.display.print("%");
}

// Animated boot sequence
void DisplayManager::animateBootSequence()
{
  carrier.display.fillScreen(0x0000);

  // Step 1: Logo fade in - centered for 256x256
  for (int i = 0; i < 5; i++)
  {
    carrier.display.fillScreen(0x0000);
    drawCarIcon(100, 80, i % 2 == 0 ? COLOR_CYAN : COLOR_ORANGE);
    delay(200);
  }

  carrier.display.fillScreen(0x0000);
  drawCarIcon(100, 80, COLOR_ORANGE);

  // Step 2: Loading text - centered
  carrier.display.setTextSize(2);
  carrier.display.setCursor(65, 130);
  carrier.display.setTextColor(COLOR_CYAN);
  carrier.display.println("Starting");

  // Step 3: Progress bar
  for (int p = 0; p <= 100; p += 10)
  {
    drawProgressBar(1550, p, COLOR_LIME);
    delay(100);
  }

  delay(500);
}

// Draw gauge arc efficiently
void DisplayManager::drawGauge(int centerX, int centerY, int radius, float value, float maxValue, uint16_t color)
{
  // Draw arc for value (simplified - just fill segments)
  float percentage = (value / maxValue);
  if (percentage > 1.0)
    percentage = 1.0;

  int segments = (int)(percentage * 10); // 0-10 segments

  for (int i = 0; i < segments; i++)
  {
    float angle = 180 - (i * 18); // Distribute across 180 degrees
    float rad = angle * 3.14159 / 180.0;
    int x1 = centerX + (radius - 8) * cos(rad);
    int y1 = centerY - (radius - 8) * sin(rad);
    int x2 = centerX + (radius - 2) * cos(rad);
    int y2 = centerY - (radius - 2) * sin(rad);

    for (int j = 0; j < 3; j++)
    {
      float rad2 = (angle - j) * 3.14159 / 180.0;
      int x3 = centerX + (radius - 5) * cos(rad2);
      int y3 = centerY - (radius - 5) * sin(rad2);
      carrier.display.drawLine(x3, y3, centerX + radius * cos(rad2), centerY - radius * sin(rad2), color);
    }
  }
}