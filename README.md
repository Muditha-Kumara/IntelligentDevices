# 🏠 Smart Door Lock System with ESP32

A comprehensive IoT-based remote door lock system for secure access control using ESP32 microcontrollers, AWS IoT, and web/mobile interfaces.

## 📋 Table of Contents

- [Overview](#overview)
- [Problem Statement](#problem-statement)
- [Project Team](#project-team)
- [System Architecture](#system-architecture)
- [Hardware Components](#hardware-components)
- [Features](#features)
- [Technical Specifications](#technical-specifications)
- [Installation & Setup](#installation--setup)
- [Usage](#usage)
- [API Documentation](#api-documentation)
- [Security Features](#security-features)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## 🎯 Overview

This project demonstrates a smart door lock system built for IT BSc degree demonstration. The system addresses a real-world problem in Finnish apartment buildings and enables remote door control through multiple communication channels including WiFi, ESP-NOW, and AWS IoT Cloud services.

## 🏢 Problem Statement

### Current Issue in Finnish Apartment Buildings

In Finland, apartment buildings typically have a two-tier door system:

1. **Main Building Door**: Often automated with digital access codes or key fobs
2. **Individual Unit Doors**: Traditional mechanical locks requiring physical keys

This creates several challenges for residents:

- **Service Access**: Couriers, food delivery personnel, and cleaning services cannot enter individual units when residents are away
- **Family Access**: Children or elderly family members may lose keys or forget them
- **Emergency Situations**: Family members, caregivers, or emergency contacts cannot access the unit remotely
- **Guest Access**: Visitors require physical key handover, which is inconvenient for short-term stays
- **Security Concerns**: Spare keys hidden outside pose security risks

### Our Solution

Our smart door lock system specifically targets **individual apartment unit doors**, providing:

✅ **Remote Access Control**: Grant access to delivery personnel, family members, or guests without physical keys  
✅ **Temporary Permissions**: Set time-limited access for specific services or visitors  
✅ **Real-time Monitoring**: Know when your door is accessed and by whom  
✅ **Emergency Access**: Family members can unlock the door remotely in emergencies  
✅ **No Infrastructure Changes**: Works with existing apartment door systems  

This solution bridges the gap between the convenience of automated building entrances and the security needs of individual apartment units.

## 🎯 Use Cases

### Real-World Scenarios

1. **Food Delivery** 🍕
   - Grant 15-minute access to delivery person
   - Monitor delivery completion remotely
   - Automatic access revocation after time limit

2. **Child Care** 👶
   - Allow babysitter access during work hours
   - Monitor entry/exit times
   - Emergency access for family members

3. **Elderly Care** 👴👵
   - Caregiver access during scheduled visits
   - Remote monitoring of care visits
   - Emergency access for medical situations

4. **Cleaning Services** 🧹
   - Weekly scheduled access for cleaning personnel
   - Verify service completion
   - Temporary access without key duplication

5. **Emergency Situations** 🚨
   - Remote access for family during emergencies
   - Landlord access for maintenance issues
   - Guest access when resident is traveling

## 🎓 Academic & Practical Value

### Learning Objectives

This project demonstrates proficiency in multiple IT domains:

**Hardware & Embedded Systems**
- ESP32 microcontroller programming
- Sensor integration and actuator control
- Power management and battery optimization
- PCB design considerations

**Networking & Communications**
- WiFi protocol implementation (STA/AP modes)
- ESP-NOW peer-to-peer communication
- MQTT protocol for IoT messaging
- RESTful API development

**Cloud Computing & IoT**
- AWS IoT Core integration
- Cloud-based data storage and retrieval
- Device authentication and security
- Scalable IoT architecture design

**Software Development**
- Full-stack web development
- Mobile application development
- Real-time system programming
- Database design and management

**Security & Privacy**
- End-to-end encryption implementation
- Access control and authentication
- Secure communication protocols
- Privacy-by-design principles

### Market Relevance

The smart home market is projected to reach €537 billion globally by 2030, with access control systems being a key segment. This project addresses:

- **Growing Urbanization**: More people living in apartment buildings
- **Digital Transformation**: Traditional security systems becoming smart
- **Convenience Demand**: Need for contactless and remote solutions
- **Security Enhancement**: Advanced access logging and monitoring

### Technical Innovation

- **Multi-protocol Communication**: Combines WiFi, ESP-NOW, and MQTT
- **Energy Efficiency**: Deep sleep implementation for battery longevity
- **Failsafe Design**: Multiple access methods for reliability
- **Scalable Architecture**: Can expand to multiple doors/devices

### Key Highlights
- ✅ Remote door lock control via web interface and mobile app
- ✅ Real-time door status monitoring
- ✅ Temporary access permissions for third parties
- ✅ Battery-optimized design with deep sleep functionality
- ✅ Multiple connectivity options (WiFi, AP, ESP-NOW)
- ✅ Cloud-based settings storage with AWS MQTT

## 🏗️ System Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Mobile App    │    │   Web Client    │    │  3rd Party      │
│                 │    │                 │    │  Access         │
└─────────┬───────┘    └─────────┬───────┘    └─────────┬───────┘
          │                      │                      │
          └──────────────────────┼──────────────────────┘
                                 │
                    ┌─────────────▼───────────────┐
                    │      AWS IoT Cloud          │
                    │      MQTT Broker            │
                    └─────────────┬───────────────┘
                                  │
                    ┌─────────────▼───────────────┐
                    │     ESP32 Main Device       │
                    │   (Home WiFi Connected)     │
                    │                             │
                    │ • Web Server Hosting        │
                    │ • AWS MQTT Client           │
                    │ • WiFi AP Mode              │
                    │ • ESP-NOW Communication     │
                    └─────────────┬───────────────┘
                                  │ ESP-NOW
                                  │
                    ┌─────────────▼───────────────┐
                    │    ESP32 Door Device        │
                    │    (Battery Powered)        │
                    │                             │
                    │ • Deep Sleep Mode           │
                    │ • Door Lock Control         │
                    │ • Status Monitoring         │
                    │ • Touch Sensor              │
                    └─────────────────────────────┘
```

## 🔧 Hardware Components

### Main Device (ESP32 #1)
- **ESP32 Development Board**
- **Power Supply**: Home power system (5V/3.3V)
- **Connectivity**: WiFi (STA + AP modes), ESP-NOW
- **Location**: Indoor, permanently powered

### Door Device (ESP32 #2)
- **ESP32 Development Board**
- **Power Supply**: Battery powered (Li-ion recommended)
- **Sensors & Actuators**:
  - Door lock actuator (servo/solenoid)
  - Door status sensor (magnetic reed switch)
  - Touch sensor for manual access
- **Location**: Door-mounted, battery operated

## ✨ Features

### 🔐 Core Functionality
1. **Remote Door Control**
   - Open door lock remotely
   - Real-time door status monitoring
   - Manual touch sensor override

2. **Access Management**
   - Grant temporary access permissions
   - Set time-limited access for third parties
   - Revoke access permissions instantly

3. **Multiple Interface Options**
   - Web-based control panel
   - Mobile application
   - Direct AP connection for emergency access

4. **Cloud Integration**
   - AWS IoT MQTT for reliable communication
   - Settings and permissions stored in cloud
   - Real-time status synchronization

### 🔋 Power Management
- Deep sleep mode for battery conservation
- Wake-on-touch functionality
- Low battery alerts

### 🌐 Connectivity Options
- **WiFi STA**: Connect to home router
- **WiFi AP**: Create hotspot for direct access
- **ESP-NOW**: Low-power device-to-device communication
- **AWS MQTT**: Cloud-based remote access

## 📱 Technical Specifications

### Communication Protocols
- **WiFi**: 802.11 b/g/n
- **ESP-NOW**: Encrypted peer-to-peer
- **MQTT**: AWS IoT Core
- **HTTP/HTTPS**: Web interface

### Security Features
- TLS/SSL encryption for cloud communication
- Device authentication tokens
- Time-based access control
- Secure ESP-NOW pairing

### Power Consumption
- **Active Mode**: ~160mA
- **Deep Sleep**: <10μA
- **Battery Life**: 6+ months (with proper usage patterns)

## 🚀 Installation & Setup

### Prerequisites
- ESP IDE with ESP32 board support
- AWS Account with IoT Core access
- Basic understanding of electronics and programming

### Hardware Setup
1. **Main Device Wiring**
   ```
   ESP32 Pin    | Component
   GPIO2        | Status LED
   3.3V/GND     | Power supply
   ```

2. **Door Device Wiring**
   ```
   ESP32 Pin    | Component
   GPIO12       | Door lock actuator
   GPIO14       | Door status sensor
   GPIO27       | Touch sensor
   GPIO2        | Status LED
   3.3V/GND     | Battery power
   ```

### Software Setup

1. **Clone Repository**
   ```bash
   git clone https://github.com/Muditha-Kumara/IntelligentDevices.git
   cd IntelligentDevices
   ```

2. **Install Dependencies**
   - ESP32 Board Package
   - AWS IoT Device SDK
   - WiFi and WebServer libraries
   - ESP-NOW library

3. **Configure AWS IoT**
   - Create IoT Thing in AWS Console
   - Download certificates
   - Update configuration files

4. **Upload Code**
   - Flash main device code to ESP32 #1
   - Flash door device code to ESP32 #2

## 📖 Usage

### Web Interface
Access the web interface by connecting to your home WiFi and navigating to the ESP32's IP address.

**Features:**
- Real-time door status
- Lock/unlock controls
- Access permission management
- System settings

### Mobile App
Download and install the companion mobile app for remote access.

**Features:**
- Push notifications
- User management
- Activity logs

### Emergency Access
In case of network failure, connect directly to the ESP32's AP:
- Network: `SmartDoor_AP`
- Password: `[configured_password]`
- Access: `192.168.4.1`

## 🔌 API Documentation

### REST Endpoints

#### Door Control
```http
POST /api/door/unlock
GET /api/door/status
```

#### Access Management
```http
POST /api/access/grant
DELETE /api/access/revoke
GET /api/access/list
```

#### System Status
```http
GET /api/system/status
GET /api/system/battery
```

### MQTT Topics

#### Commands
- `smartdoor/command/unlock`
- `smartdoor/command/status`

#### Status Updates
- `smartdoor/status/door`
- `smartdoor/status/battery`
- `smartdoor/status/connection`

## 🤝 Contributing

We welcome contributions! Please follow these steps:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## � Project Team

This project is developed as a group effort for IT BSc degree demonstration by:

- **Muditha** - Project Lead & System Architecture
- **Chuck** - Hardware Integration & ESP32 Programming  
- **Nikolai** - Cloud Services & Mobile App Development

## �🙏 Acknowledgments

- ESP32 community for excellent documentation
- AWS IoT team for robust cloud services
- Open-source contributors for various libraries used
- Our university faculty for project guidance and support

---

**Project Status**: 🚧 In Development for IT BSc Degree Demonstration

**Team Members**: Muditha, Chuck, Nikolai

**Contact**: muditha.kumara@centria.fi 

**Last Updated**: October 2025