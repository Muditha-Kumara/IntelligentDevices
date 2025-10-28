# saveVehicle (IoT Demonstration)

![Project](https://img.shields.io/badge/Project-IoT%20Demo-blue) ![Board-Arduino%20Opla](https://img.shields.io/badge/Board-Arduino%20Opla-orange)

## Overview

An IoT demonstration for an IT BSc degree. The system monitors vehicle driving quality using the Arduino Opla kit. Its main goals are to improve driving safety, increase passenger comfort, protect the vehicle from damage, and reduce maintenance costs by detecting risky driving events early.

The device reads all built-in sensors on the Opla kit (IMU, microphone, light, temperature, etc.), detects events such as hard acceleration, harsh braking, sharp turns and bumps, triggers local warnings (LED, display, buzzer) and publishes timestamped telemetry and events to the cloud via MQTT over Wi‑Fi.

## Key Features

- Use Arduino Opla kit and all built-in sensors
- Local alerts: buzzer, LED, optional display
- Event classification: hard acceleration, hard braking, sharp turn, bump/impact
- Cloud telemetry and event logging via MQTT
- Remote configuration via MQTT (thresholds, sampling rate)

## Hardware (recommended)

- Arduino Opla kit (required — use all built-in sensors)
- Optional: external IMU (MPU6050/MPU9250) for higher accuracy
- Buzzer or small speaker
- RGB/status LED
- Small OLED/LCD display (optional)
- Stable power source for demo

## Software stack

- Firmware: Arduino framework with PlatformIO
- Libraries: WiFi, PubSubClient or Async MQTT client, IMU/microphone/display drivers
- Cloud: MQTT broker (HiveMQ, Mosquitto, or cloud service) and a simple logger/notification service

## MQTT topics & message formats

Suggested topics:
- Telemetry: `devices/<device_id>/telemetry`
- Events: `devices/<device_id>/events`
- Config: `devices/<device_id>/config`

Example event JSON:

{
  "device_id": "opla-01",
  "timestamp": "2025-10-17T12:34:56Z",
  "type": "hard_braking",
  "values": {
    "accel_x": -3.8,
    "accel_y": 0.1,
    "accel_z": 9.2,
    "gyro_x": 0.0,
    "gyro_y": 0.0,
    "gyro_z": 0.02,
    "temperature": 25.0,
    "humidity": 60.0,
    "pressure": 1013.25,
    "mic_level": 0.75,
    "light_level": 120
  },
  "level": "alert"
}

Telemetry messages should include:
- `device_id`, `timestamp`
- Sensor values: `accel_x`, `accel_y`, `accel_z`, `gyro_x`, `gyro_y`, `gyro_z`, `temperature`, `humidity`, `pressure`, `mic_level`, `light_level`

## Suggested thresholds (from code)

- Caution: Acceleration magnitude > 0.5 m/s²
- Alert: Acceleration magnitude > 0.5 m/s²
- Critical: Acceleration magnitude > 1.5 m/s²
- Hard braking (deceleration): `accel_y < -1.0 m/s²`
- Hard acceleration: `accel_y > 0.5 m/s²`
- Sharp turn: `|accel_x| > 1.0 m/s²`
- Bump/impact: Acceleration magnitude > 3.0 m/s² (short duration)

Sampling rate: 100 ms  
Telemetry interval: 2000 ms

Tuning is required based on mounting and vehicle type.

## Configuration & calibration

- Support remote config over MQTT on `devices/<device_id>/config`.
- Configurable parameters: sampling_rate, thresholds, telemetry_interval, mqtt settings.

Calibration steps (brief):
1. Place device stationary; collect N samples to estimate bias for accel/gyro.
2. Subtract offsets and apply a low-pass filter to separate gravity from dynamic acceleration.
3. Use sliding-window detection for short spikes (bumps).

## Quick start (development)

1. Install PlatformIO.
2. Install required libraries (MQTT client, sensor drivers).
3. Open the firmware and update Wi‑Fi / MQTT settings in `src/Config.h`.
4. Build and flash to the Arduino Opla kit using PlatformIO.
5. Open the serial monitor to observe logs and telemetry.

## Demonstration flow (short)

1. Power on the Arduino Opla kit and connect to Wi‑Fi.
2. Device publishes a "ready" telemetry message.
3. Simulate events or perform a short test ride.
4. Device activates local warnings and publishes `events` messages.
5. Cloud logger records events and (optionally) forwards notifications to the owner.

## Deliverables

- Firmware source for Arduino Opla kit
- Minimal cloud component: MQTT logger + notification forwarder
- README (this file), calibration guide, and demo script
- Short demo video or live demo

## License

Recommend MIT or another permissive license. Update `LICENSE` as required.

---

