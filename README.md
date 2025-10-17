# Vehicle Driving Quality Monitor (IoT Demonstration)

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

- Firmware: Arduino (ESP32) framework
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
    "gyro_z": 0.02,
    "mic_level": 0.75
  },
  "level": "alert"
}

Telemetry messages should include device id, timestamp, and raw/processed sensor values.

## Suggested thresholds (starting points)

- Caution: Acceleration magnitude > 2.0 m/s²
- Alert: Acceleration magnitude > 3.5 m/s²
- Critical: Acceleration magnitude > 5.0 m/s²

- Hard braking (deceleration): > 3.5 m/s²
- Sharp turn: angular rate > 150°/s
- Bump/impact: accel spike > 6.0 m/s² (short duration)

Tuning is required based on mounting and vehicle type.

## Configuration & calibration

- Support remote config over MQTT on `devices/<device_id>/config`.
- Configurable parameters: sampling_rate, thresholds, telemetry_interval, mqtt settings.

Calibration steps (brief):
1. Place device stationary; collect N samples to estimate bias for accel/gyro.
2. Subtract offsets and apply a low-pass filter to separate gravity from dynamic acceleration.
3. Use sliding-window detection for short spikes (bumps).

## Quick start (development)

1. Install Arduino IDE or PlatformIO and ESP32 board packages.
2. Install required libraries (MQTT client, sensor drivers).
3. Open firmware example (to be provided) and update Wi‑Fi / MQTT settings.
4. Build and flash to Arduino Opla kit.
5. Open serial monitor to observe logs and telemetry.

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

If you want, I can now generate starter firmware (Arduino sketch) for the Arduino Opla kit or an example MQTT logger script. Which would you like first?
