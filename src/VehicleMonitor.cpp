
/**
 * VehicleMonitor.cpp
 * Implementation of core vehicle monitoring logic
 */

#include "VehicleMonitor.h"

VehicleMonitor::VehicleMonitor(MKRIoTCarrier &carrier, AlertManager &alert)
    : carrier(carrier), alert(alert)
{
}

void VehicleMonitor::calibrateSensors() {
  Serial.println("Calibrating sensors...");
  
  float temp_accel_x = 0, temp_accel_y = 0, temp_accel_z = 0;
  float temp_gyro_x = 0, temp_gyro_y = 0, temp_gyro_z = 0;
  
  const int samples = 100;
  
  for (int i = 0; i < samples; i++) {
    float ax, ay, az, gx, gy, gz;
    
    if (carrier.IMUmodule.accelerationAvailable()) {
      carrier.IMUmodule.readAcceleration(ax, ay, az);
      temp_accel_x += ax;
      temp_accel_y += ay;
      temp_accel_z += az;
    }
    
    if (carrier.IMUmodule.gyroscopeAvailable()) {
      carrier.IMUmodule.readGyroscope(gx, gy, gz);
      temp_gyro_x += gx;
      temp_gyro_y += gy;
      temp_gyro_z += gz;
    }
    
    delay(50);
  }
  
  // Calculate offsets
  calibration.accel_offset_x = temp_accel_x / samples;
  calibration.accel_offset_y = temp_accel_y / samples;
  calibration.accel_offset_z = temp_accel_z / samples - 1.0; // Subtract gravity (1g)
  calibration.gyro_offset_x = temp_gyro_x / samples;
  calibration.gyro_offset_y = temp_gyro_y / samples;
  calibration.gyro_offset_z = temp_gyro_z / samples;
  
  calibration.complete = true;
  
  Serial.println("Calibration complete!");
  Serial.print("Accel offsets: ");
  Serial.print(calibration.accel_offset_x, 3); Serial.print(", ");
  Serial.print(calibration.accel_offset_y, 3); Serial.print(", ");
  Serial.println(calibration.accel_offset_z, 3);
  Serial.print("Gyro offsets: ");
  Serial.print(calibration.gyro_offset_x, 3); Serial.print(", ");
  Serial.print(calibration.gyro_offset_y, 3); Serial.print(", ");
  Serial.println(calibration.gyro_offset_z, 3);
}

void VehicleMonitor::readSensors() {
  // Read IMU data
  if (carrier.IMUmodule.accelerationAvailable()) {
    carrier.IMUmodule.readAcceleration(sensorData.accel_x, sensorData.accel_y, sensorData.accel_z);
    // Apply calibration offsets
    sensorData.accel_x -= calibration.accel_offset_x;
    sensorData.accel_y -= calibration.accel_offset_y;
    sensorData.accel_z -= calibration.accel_offset_z;
  }
  
  if (carrier.IMUmodule.gyroscopeAvailable()) {
    carrier.IMUmodule.readGyroscope(sensorData.gyro_x, sensorData.gyro_y, sensorData.gyro_z);
    // Apply calibration offsets and convert to degrees/second
    sensorData.gyro_x = (sensorData.gyro_x - calibration.gyro_offset_x) * 180.0 / PI;
    sensorData.gyro_y = (sensorData.gyro_y - calibration.gyro_offset_y) * 180.0 / PI;
    sensorData.gyro_z = (sensorData.gyro_z - calibration.gyro_offset_z) * 180.0 / PI;
  }
  
  // Read environmental sensors
  sensorData.temperature = carrier.Env.readTemperature();
  sensorData.humidity = carrier.Env.readHumidity();
  sensorData.pressure = carrier.Pressure.readPressure();
  
  // Read light sensor (estimate brightness from RGB)
  int r = 0, g = 0, b = 0;
  if (carrier.Light.colorAvailable()) {
    carrier.Light.readColor(r, g, b);
    sensorData.light_level = (r + g + b) / 3; // Simple brightness estimate
  } else {
    sensorData.light_level = 0;
  }
  
  // Simulate microphone level (Arduino Opla doesn't have direct mic access)
  sensorData.mic_level = random(0, 100) / 100.0; // Placeholder
}

void VehicleMonitor::detectVehicleEvents() {
  if (!calibration.complete) return;
  warningActive = false;

  // Calculate acceleration magnitude
  float accel_magnitude = sqrt(sensorData.accel_x * sensorData.accel_x + 
                              sensorData.accel_y * sensorData.accel_y + 
                              sensorData.accel_z * sensorData.accel_z);

  // Detect hard braking (negative Y acceleration)
  if (sensorData.accel_y < -thresholds.hard_braking)
  {
    String level = (abs(sensorData.accel_y) > thresholds.critical_accel) ? "critical" : "alert";
    alert.triggerAlert("HARD BRAKING", level);
    warningActive = true;
    lastWarningType = "HARD BRAKING";
    lastWarningLevel = level;
  }
  // Detect hard acceleration (positive Y acceleration)
  else if (sensorData.accel_y > thresholds.alert_accel)
  {
    String level = (sensorData.accel_x > thresholds.critical_accel) ? "critical" : "alert";
    alert.triggerAlert("HARD ACCEL", level);
    warningActive = true;
    lastWarningType = "HARD ACCEL";
    lastWarningLevel = level;
  }
  // Detect sharp turns (high angular velocity on Z axis)
  else if (abs(sensorData.accel_x) > thresholds.sharp_turn)
  {
    String direction = (sensorData.gyro_x > 0) ? "left" : "right";
    alert.triggerAlert("SHARP TURN", "alert");
    warningActive = true;
    lastWarningType = "SHARP TURN";
    lastWarningLevel = "alert";
  }
  else if (accel_magnitude > thresholds.bump_impact)
  {
    alert.triggerAlert("BUMP/IMPACT", "alert");
    warningActive = true;
    lastWarningType = "BUMP/IMPACT";
    lastWarningLevel = "alert";
  }
}
