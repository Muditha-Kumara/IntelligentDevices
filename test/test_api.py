import requests
import json

url = "https://uz42hteetd.execute-api.us-east-1.amazonaws.com/vehicle/events"

payload = {
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

headers = {"Content-Type": "application/json"}

response = requests.post(url, data=json.dumps(payload), headers=headers)

print("Status code:", response.status_code)
print("Response:", response.text)
