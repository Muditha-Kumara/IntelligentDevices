
# API Test & Report for Vehicle Monitoring System

This document describes how to test the cloud API endpoint for vehicle event ingestion and reporting. For full project and cloud architecture, see [../README.md](../README.md) and [../cloud/README.md](../cloud/README.md).

## API Endpoint

`POST https://wvnlo3ttu6.execute-api.eu-north-1.amazonaws.com/vehicle/events`

## Test Script

- See `test_api.py` for a Python script to send a sample event to the API.
- The payload matches the expected event format for the cloud Lambda.

### Example Usage

```bash
python3 test_api.py
```

## Expected Output

- Status code: 200 (success)
- Response: "Event processed successfully" (from Lambda)

## Sample Payload

```json
{
  "device_id": "opla-01",
  "event_type": "hard_braking",
  "level": "alert",
  "timestamp": "2025-10-31T12:34:56Z",
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

## How to Integrate

- This test validates the end-to-end flow from device to cloud.
- For full integration, see:
  - [../README.md](../README.md) (root project)
  - [../cloud/README.md](../cloud/README.md) (cloud backend)

## Test Report

- Last test: [update with date/time]
- Status: [update with result, e.g. success/failure, response details]
- Notes: [add any issues, suggestions, or improvements]
