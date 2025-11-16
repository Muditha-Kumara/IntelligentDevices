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

  // Initialize display
  displayManager.begin();
  displayManager.showInitializing();

  // Calibrate sensors
  displayManager.showCalibrating(0);
  vehicleMonitor.calibrateSensors();
  displayManager.flashLEDs(COLOR_GREEN, 2, 500);

  // Show system status
  displayManager.showSystemStatus(
      WiFi.status() == WL_CONNECTED,
      vehicleMonitor.isCalibrationComplete());
  delay(1000);

  // Draw interactive UI for monitoring (round display)
  displayManager.drawMonitoringUI();
  displayManager.updateWarningMsg(""); // Initialize warning message area

  systemReady = true;
  Serial.println("System ready for vehicle monitoring!");
}

void loop()
{
  if (!systemReady)
    return;

  unsigned long currentTime = millis();
  Thresholds &thresholds = vehicleMonitor.getThresholds();

  static unsigned long lastDrivingMonitor = 0;
  static unsigned long lastDisplayRefresh = 0;
  static unsigned long lastTempHumidity = 0;
  static VehicleData lastVehicleData;
  static float lastTemp = -1000;
  static float lastHumidity = -1000;

  // Monitor driving status every 100ms
  if (currentTime - lastDrivingMonitor >= 100)
  {
    vehicleMonitor.readSensors();
    vehicleMonitor.detectVehicleEvents();
    VehicleData data = vehicleMonitor.getSensorData();
    lastVehicleData = data;
    lastDrivingMonitor = currentTime;

    // Update sensor values in permanent UI
    displayManager.updateAccelValue(lastVehicleData.accel_x, lastVehicleData.accel_y, lastVehicleData.accel_z);
    // displayManager.updateGyroValue(abs(lastVehicleData.gyro_z));
    displayManager.updateTempValue(lastTemp);
    displayManager.updateHumidityValue(lastHumidity);
    displayManager.updatePressureValue(lastVehicleData.pressure);

    // Update warning message in reserved area
    if (vehicleMonitor.isWarningActive())
    {
      displayManager.updateWarningMsg(vehicleMonitor.getLastWarningType() + " (" + vehicleMonitor.getLastWarningLevel() + ")");
      if (millis() > vehicleMonitor.getWarningEndTime())
      {
        vehicleMonitor.clearWarning();
        displayManager.updateWarningMsg("");
      }
    }
  }
  // Measure temperature and humidity every 5s
  if (currentTime - lastTempHumidity >= 5000)
  {
    float temp = carrier.Env.readTemperature();
    float humidity = carrier.Env.readHumidity();
    if (temp != lastTemp || humidity != lastHumidity)
    {
      VehicleData tempData = lastVehicleData;
      tempData.temperature = temp;
      tempData.humidity = humidity;
      // TODO: Send tempData via API POST request here
      displayManager.updateTempValue(temp);
      displayManager.updateHumidityValue(humidity);
      displayManager.updatePressureValue(lastVehicleData.pressure);
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
    displayManager.showCalibrating(0);
    vehicleMonitor.calibrateSensors();
    displayManager.flashLEDs(COLOR_GREEN, 2, 500);
    displayManager.showSystemStatus(
        WiFi.status() == WL_CONNECTED,
        vehicleMonitor.isCalibrationComplete());
    displayManager.drawMonitoringUI();
    delay(1000);
  }
}
