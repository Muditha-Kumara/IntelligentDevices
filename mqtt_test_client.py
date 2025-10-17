#!/usr/bin/env python3
"""
MQTT Test Client for Vehicle Driving Quality Monitor
===================================================

This script helps test the Vehicle Driving Quality Monitor system by:
1. Subscribing to telemetry and event messages
2. Sending configuration updates
3. Logging all received data

Requirements:
    pip install paho-mqtt

Usage:
    python3 mqtt_test_client.py

Configuration:
    Update the MQTT_BROKER, DEVICE_ID variables below to match your setup.
"""

import paho.mqtt.client as mqtt
import json
import time
from datetime import datetime

# Configuration
MQTT_BROKER = "broker.hivemq.com"  # Free MQTT broker - change to your broker
MQTT_PORT = 1883
DEVICE_ID = "opla-01"  # Must match the device_id in Arduino code

# MQTT Topics
TELEMETRY_TOPIC = f"devices/{DEVICE_ID}/telemetry"
EVENTS_TOPIC = f"devices/{DEVICE_ID}/events"
CONFIG_TOPIC = f"devices/{DEVICE_ID}/config"

class VehicleMonitorClient:
    def __init__(self):
        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        
    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print(f"✅ Connected to MQTT broker: {MQTT_BROKER}")
            print(f"📡 Subscribing to topics for device: {DEVICE_ID}")
            
            # Subscribe to device topics
            client.subscribe(TELEMETRY_TOPIC)
            client.subscribe(EVENTS_TOPIC)
            
            print(f"   - {TELEMETRY_TOPIC}")
            print(f"   - {EVENTS_TOPIC}")
            print("\n🔍 Listening for messages... (Press Ctrl+C to exit)\n")
            
        else:
            print(f"❌ Failed to connect to MQTT broker. Return code: {rc}")
    
    def on_message(self, client, userdata, msg):
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        topic = msg.topic
        
        try:
            payload = json.loads(msg.payload.decode())
            
            if topic == TELEMETRY_TOPIC:
                self.handle_telemetry(payload, timestamp)
            elif topic == EVENTS_TOPIC:
                self.handle_event(payload, timestamp)
                
        except json.JSONDecodeError:
            print(f"[{timestamp}] ⚠️  Invalid JSON received on {topic}")
            print(f"Raw payload: {msg.payload.decode()}\n")
    
    def handle_telemetry(self, data, timestamp):
        print(f"[{timestamp}] 📊 TELEMETRY from {data.get('device_id', 'unknown')}")
        
        if 'values' in data:
            values = data['values']
            
            # Calculate acceleration magnitude
            accel_mag = (values.get('accel_x', 0)**2 + 
                        values.get('accel_y', 0)**2 + 
                        values.get('accel_z', 0)**2)**0.5
            
            print(f"   🚗 Acceleration: {accel_mag:.2f} m/s² "
                  f"(X:{values.get('accel_x', 0):.2f}, "
                  f"Y:{values.get('accel_y', 0):.2f}, "
                  f"Z:{values.get('accel_z', 0):.2f})")
            
            print(f"   🔄 Gyroscope: Z-axis {values.get('gyro_z', 0):.1f}°/s")
            
            print(f"   🌡️  Environment: {values.get('temperature', 0):.1f}°C, "
                  f"{values.get('humidity', 0):.1f}% RH, "
                  f"{values.get('pressure', 0):.1f} kPa")
        
        print()  # Empty line for readability
    
    def handle_event(self, data, timestamp):
        event_type = data.get('type', 'unknown')
        level = data.get('level', 'info')
        description = data.get('description', 'No description')
        
        # Use emoji based on event type and level
        if level == 'critical':
            icon = "🚨"
        elif level == 'alert':
            icon = "⚠️"
        else:
            icon = "ℹ️"
        
        print(f"[{timestamp}] {icon} EVENT DETECTED!")
        print(f"   Type: {event_type.upper()}")
        print(f"   Level: {level.upper()}")
        print(f"   Description: {description}")
        
        if 'values' in data:
            values = data['values']
            print(f"   Sensor data: Accel({values.get('accel_x', 0):.2f}, "
                  f"{values.get('accel_y', 0):.2f}, {values.get('accel_z', 0):.2f}), "
                  f"Gyro Z:{values.get('gyro_z', 0):.1f}°/s")
        
        print()  # Empty line for readability
    
    def send_config_update(self, config):
        """Send configuration update to the device"""
        config_json = json.dumps(config)
        result = self.client.publish(CONFIG_TOPIC, config_json)
        
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            print(f"✅ Configuration sent to device: {config_json}")
        else:
            print(f"❌ Failed to send configuration")
    
    def start(self):
        """Start the MQTT client"""
        try:
            self.client.connect(MQTT_BROKER, MQTT_PORT, 60)
            self.client.loop_forever()
        except KeyboardInterrupt:
            print("\n👋 Disconnecting from MQTT broker...")
            self.client.disconnect()
        except Exception as e:
            print(f"❌ Error: {e}")

def main():
    print("🚗 Vehicle Driving Quality Monitor - MQTT Test Client")
    print("=" * 55)
    print(f"Device ID: {DEVICE_ID}")
    print(f"MQTT Broker: {MQTT_BROKER}:{MQTT_PORT}")
    print()
    
    client = VehicleMonitorClient()
    
    # Example: Send configuration update after connecting
    # You can uncomment and modify this section to test configuration updates
    """
    def send_test_config():
        time.sleep(2)  # Wait for connection
        test_config = {
            "thresholds": {
                "alert_accel": 2.5,
                "critical_accel": 4.0,
                "hard_braking": 3.0,
                "sharp_turn": 120.0,
                "sampling_rate": 200,
                "telemetry_interval": 3000
            }
        }
        client.send_config_update(test_config)
    
    import threading
    threading.Timer(2.0, send_test_config).start()
    """
    
    client.start()

if __name__ == "__main__":
    main()
