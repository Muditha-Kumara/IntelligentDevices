#include <Arduino_MKRIoTCarrier.h>
#include <WiFiNINA.h>
#include "Config.h"
#include "DisplayManager.h"
#include "AlertManager.h"
#include "VehicleMonitor.h"

MKRIoTCarrier carrier;
DisplayManager displayManager(carrier);
AlertManager alertManager(carrier, displayManager);
VehicleMonitor vehicleMonitor(carrier, alertManager);

bool systemReady = false;
unsigned long lastTelemetry = 0;
unsigned long lastSample = 0;

void setup()
{
  CARRIER_CASE = false;
  Serial.begin(9600);

  Serial.println("=== Vehicle Driving Quality Monitor ===");
  Serial.println("Initializing Arduino Opla IoT Carrier...");

  // Initialize carrier
  if (!carrier.begin())
  {
    Serial.println("ERROR: Carrier not connected, check connections");
    while (1)
      ;
  }

  // Connect to WiFi before any network operations
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int wifiTimeout = 20000; // 20s timeout
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < wifiTimeout)
  {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("WiFi connection failed!");
    // Optionally halt or continue in offline mode
  }

  // Initialize display
  displayManager.begin();
  displayManager.animateBootSequence();

  // Calibrate sensors
  vehicleMonitor.calibrateSensors();
  displayManager.flashLEDs(COLOR_GREEN, 2, 500);
  delay(500);

  // Draw initial UI (static elements + gauges)
  displayManager.drawInitialUI();

  systemReady = true;
  Serial.println("System ready for vehicle monitoring!");
}

void loop()
{
  if (!systemReady)
    return;

  unsigned long currentTime = millis();

  static unsigned long lastDrivingMonitor = 0;
  static unsigned long lastDisplayRefresh = 0;
  static unsigned long lastTempHumidity = 0;
  static VehicleData lastVehicleData;
  static float lastTemp = -1000;
  static float lastHumidity = -1000;
  static bool lastEventActive = false;

  // Monitor driving status every 100ms
  if (currentTime - lastDrivingMonitor >= 100)
  {
    vehicleMonitor.readSensors();
    vehicleMonitor.detectVehicleEvents();
    VehicleData data = vehicleMonitor.getSensorData();
    lastVehicleData = data;
    lastDrivingMonitor = currentTime;
  }

  // Update display efficiently every 200ms (partial updates only)
  if (currentTime - lastDisplayRefresh >= 200)
  {
    // Update acceleration gauge
    displayManager.updateAccelGauge(
        lastVehicleData.accel_x,
        lastVehicleData.accel_y,
        lastVehicleData.accel_z);

    // Update temperature gauge
    if (lastTemp == -1000)
    {
      lastTemp = carrier.Env.readTemperature();
    }
    displayManager.updateTempGauge(lastTemp);

    // Update connection status indicators
    displayManager.updateConnectionStatus(
        WiFi.status() == WL_CONNECTED,
        false);

    lastDisplayRefresh = currentTime;

    // Handle event indicators
    if (vehicleMonitor.isWarningActive())
    {
      if (!lastEventActive)
      {
        lastEventActive = true;
      }

      if (millis() > vehicleMonitor.getWarningEndTime())
      {
        vehicleMonitor.clearWarning();
        displayManager.clearEventIndicator();
        lastEventActive = false;
      }
    }
    else if (lastEventActive)
    {
      displayManager.clearEventIndicator();
      lastEventActive = false;
    }
  }

  // Measure temperature and humidity every 5s
  if (currentTime - lastTempHumidity >= 5000)
  {
    float temp = carrier.Env.readTemperature();
    float humidity = carrier.Env.readHumidity();

    if (temp != lastTemp || humidity != lastHumidity)
    {
      lastVehicleData.temperature = temp;
      lastVehicleData.humidity = humidity;
      lastVehicleData.pressure = carrier.Pressure.readPressure();

      // Update sensor values efficiently
      displayManager.updateSensorValues(lastVehicleData);

      lastTemp = temp;
      lastHumidity = humidity;
    }
    lastTempHumidity = currentTime;
  }

  // Check for button press for manual calibration
  carrier.Buttons.update();
  if (carrier.Buttons.onTouchDown(TOUCH2))
  {
    Serial.println("Manual recalibration initiated...");
    displayManager.showCalibrationProgress(0);
    vehicleMonitor.calibrateSensors();
    displayManager.flashLEDs(COLOR_GREEN, 2, 500);
    delay(500);
    // Redraw UI after calibration
    displayManager.drawInitialUI();
  }
}
