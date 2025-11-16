# IntelligentDevices Vehicle Monitor (IoT Demo)

![Project](https://img.shields.io/badge/Project-IoT%20Demo-blue) ![Board-Arduino%20Opla](https://img.shields.io/badge/Board-Arduino%20Opla-orange)

## Overview


This project is a complete IoT demonstration for vehicle driving quality monitoring, built on the Arduino Opla kit. It connects to WiFi, synchronizes time using NTP for real-world UTC timestamps, and sends sensor data and event alerts to a cloud API endpoint. The system is designed to:
- Improve driving safety
- Increase passenger comfort
- Protect the vehicle from damage
- Reduce maintenance costs by detecting risky driving events early


**Key workflow:**
- Connects to WiFi automatically on boot (see `src/saveVehicle.ino`)
- Synchronizes time with NTP (see `src/AlertManager.cpp`)
- Reads all built-in sensors (IMU, microphone, light, temperature, humidity, pressure)
- Detects events: hard acceleration, hard braking, sharp turns, bumps/impacts
- Triggers local alerts: LED, buzzer, display
- Sends event and telemetry data to the cloud via HTTP POST (JSON, real UTC timestamp)


## Key Features
- WiFi auto-connect and status reporting
- Real-world UTC timestamps via NTP (ISO8601 format)
- Local alerts: buzzer, LED, display
- Event detection: hard acceleration, hard braking, sharp turn, bump/impact
- Cloud event logging via HTTP POST (JSON)
- Configurable thresholds and calibration

## Hardware (recommended)

- Arduino Opla kit (required — use all built-in sensors)
- Optional: external IMU (MPU6050/MPU9250) for higher accuracy
- Buzzer or small speaker
- RGB/status LED
- Small OLED/LCD display
- Stable power source for demo

## Software stack

- Firmware: Arduino framework with PlatformIO
- Libraries: WiFi, HTTP client, IMU/microphone/display drivers
- Cloud: API Gateway endpoint and Lambda function for logging/notification


## API Endpoint & Message Format

Endpoint:
`POST https://wvnlo3ttu6.execute-api.eu-north-1.amazonaws.com/vehicle/events`

Event JSON format (matches deployed AWS Lambda):
```
{
  "device_id": "vehicle-1",
  "event_type": "hard_braking",
  "level": "alert",
  "timestamp": "2025-11-17T12:34:56Z",
  "data": {
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
  }
}
```


## Event Detection Thresholds
- Hard braking: `accel_y < -0.5` (alert), `< -0.8` (critical)
- Hard acceleration: `accel_y > 0.4` (alert), `> 0.8` (critical)
- Sharp turn: `|accel_x| > 0.4`
- Bump/impact: Acceleration magnitude > 2.0
- Sampling rate: 100 ms
- Telemetry interval: 2000 ms
*Tune thresholds as needed for your vehicle and mounting.*


## Configuration & Calibration
- WiFi SSID, password, device ID, and API endpoint are set in `src/Config.h` / `src/Config.cpp`
- Thresholds and sampling rates are configurable in code
- Calibration: Touch Button 2 while stationary to calibrate sensors (bias removal)


## Quick Start
1. Install PlatformIO
2. Clone this repo
3. Update WiFi and API settings in `src/Config.h` / `src/Config.cpp`
4. Build and upload to Arduino Opla kit
5. Open serial monitor for logs and status


## Demo Flow
1. Power on device; it connects to WiFi and syncs time via NTP
2. Device monitors sensors and detects driving events
3. Local alerts (LED, buzzer, display) activate on risky events
4. Device sends event data to cloud API with real UTC timestamp
5. Cloud Lambda logs events and can trigger notifications


- Arduino firmware source (PlatformIO project)
- Cloud Lambda/API logger (see cloud/)
- Updated README, calibration guide, demo script
- Demo video or live demonstration

## Cloud Architecture & Integration


See [cloud/README.md](cloud/README.md) for full details and setup instructions.

## API Testing & Reporting

For instructions on testing the cloud API endpoint and reporting results, see [test/README.md](test/README.md).

This documents how to validate the end-to-end flow from device to cloud, including sample payloads, expected output, and integration notes.



### Cloud Architecture Overview

This project includes a complete cloud backend for event logging and notifications:

- **AWS API Gateway**: Receives HTTP POSTs from devices
- **AWS Lambda (Go, Docker image)**: Processes events, stores in DynamoDB, triggers notifications
- **Amazon DynamoDB**: Stores all event and telemetry data
- **Amazon SNS**: Sends email/SMS notifications for alerts/critical events
- **Terraform**: Provisions all AWS resources (API Gateway, Lambda, DynamoDB, SNS, IAM, CloudWatch)

### Lambda Function

- Written in Go (`cloud/lambda/main.go`)
- Accepts event JSON from API Gateway
- Stores event in DynamoDB
- Publishes to SNS if level is `alert` or `critical`

### Terraform Setup

- See `cloud/main.tf` and `cloud/modules/`
- Deploys all AWS resources and configures environment variables

### How it works

1. Device sends event JSON to API Gateway
2. API Gateway triggers Lambda
3. Lambda stores event in DynamoDB
4. Lambda sends notification via SNS if needed
5. All resources are managed via Terraform

For full cloud setup, see [cloud/README.md](cloud/README.md).

- Arduino firmware source (PlatformIO project)
- Cloud Lambda/API logger (see cloud/)
- Updated README, calibration guide, demo script
- Demo video or live demonstration


## License

MIT or other permissive license recommended. See `LICENSE`.

---

# Directory Structure
## Directory Structure
```
LICENSE                  # Project license
platformio.ini           # PlatformIO configuration
README.md                # Project overview
cloud/
  ├── main.tf            # Terraform root module
  ├── modules/           # Terraform modules for reusable resources
  ├── lambda/            # Go source code for Lambda functions with Docker
  ├── build_and_push.sh  # Helper script for Lambda Docker image
  ├── terraform.tfvars   # Terraform variables
  ├── terraform.tfstate  # Terraform state file
  ├── terraform.tfstate.backup # Terraform state backup
  └── README.md          # Cloud setup instructions
src/
  ├── saveVehicle.ino    # Main Arduino sketch
  ├── AlertManager.cpp/h # Event detection and alert logic
  ├── Config.cpp/h       # Device and network configuration
  ├── DisplayManager.cpp/h # Display handling
  ├── VehicleMonitor.cpp/h # Sensor reading and monitoring
  ├── aws_certificates.h # AWS IoT certificates
  ├── pitches.h          # Buzzer sound definitions
  ├── visuals.h          # Display visuals
  └── sketch.json        # PlatformIO project metadata
test/
  ├── test_api.py        # Cloud API test script
  └── README.md          # Test instructions
```

