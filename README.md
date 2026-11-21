# RescueMesh – Decentralized LoRa Communication Network

A resilient, multi-hop wireless communication network designed to maintain connectivity during emergencies and infrastructure failures.

RescueMesh uses ESP32 microcontrollers and LoRa communication modules to enable nodes to exchange messages, relay emergency alerts, and extend communication coverage without relying on Wi-Fi or cellular infrastructure.

---

## Project Overview

RescueMesh is designed around the principle:

> **Node failure must not become communication failure.**

The network consists of multiple independent ESP32-based nodes communicating through LoRa. Each node can transmit, receive, and relay messages, allowing information to travel across the network through multiple hops.

The system supports emergency SOS alerts, vibration detection, impact detection, and remote node status indication.

---

## Features

- Decentralized node-to-node communication
- LoRa-based long-range wireless communication
- Multi-hop message forwarding
- Message Time-To-Live (TTL)
- Duplicate message filtering
- Manual SOS emergency button
- Accelerometer-based vibration detection
- Threshold-based impact detection
- LED-based alert indication
- Signal strength (RSSI) monitoring
- Packet CRC error checking
- Dashboard integration through Node D

---

## System Architecture

RescueMesh consists of multiple ESP32-based communication nodes.

Each node is capable of:

- Transmitting messages
- Receiving messages from nearby nodes
- Forwarding messages to extend network coverage
- Identifying message origins
- Filtering duplicate messages
- Processing emergency alerts

Node D is connected to the monitoring dashboard in the current implementation, providing a visual interface for network messages and alerts.

The LoRa communication network itself does not depend on a dedicated central gateway.

---

## Hardware Components

| Component | Description |
|---|---|
| ESP32 | Microcontroller for each communication node |
| LoRa SX1278 / RA-02 | Wireless communication module |
| MPU6050 | Accelerometer and gyroscope sensor |
| Push Button | Manual SOS trigger |
| LEDs | Node status and alert indication |
| OLED Display | Optional monitoring interface |
| Power Supply | Portable power source |

---

## Communication Protocol

Messages are transmitted using a structured packet format:

```text
NETWORK_ID|MESSAGE_ID|ORIGIN|TTL|PAYLOAD
```

### Example Packet

```text
RM_SK_7264|A-33|A|3|SOS:Rescue needed
```

### Packet Fields

| Field | Description |
|---|---|
| NETWORK_ID | Identifies the RescueMesh network |
| MESSAGE_ID | Unique message identifier |
| ORIGIN | Original transmitting node |
| TTL | Remaining forwarding limit |
| PAYLOAD | Message or emergency information |

Nodes verify the network ID and check for duplicate messages before processing and forwarding packets.

The TTL value is decremented during forwarding to limit unnecessary retransmissions.

---

## Node A Implementation

Node A currently implements:

- Manual SOS button
- MPU6050-based acceleration monitoring
- Vibration detection
- Threshold-based impact detection
- LoRa transmission and reception
- Multi-hop packet forwarding
- Duplicate message filtering
- Remote alert processing
- LED status indication
- RSSI reporting

### Alert Indication

| LED Indication | Status |
|---|---|
| Green | Idle |
| Yellow | Vibration detected |
| Red | High-impact detected |
| Blinking Red | SOS alert |

---

## Hardware Pin Configuration

### ESP32 – LoRa SX1278

| LoRa Pin | ESP32 GPIO |
|---|---|
| NSS / SS | GPIO 5 |
| RESET | GPIO 14 |
| DIO0 | GPIO 26 |
| SCK | GPIO 18 |
| MISO | GPIO 19 |
| MOSI | GPIO 23 |

### MPU6050 – I2C

| MPU6050 Pin | ESP32 GPIO |
|---|---|
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| Address | 0x68 |

### SOS Button and LEDs

| Component | ESP32 GPIO |
|---|---|
| SOS Button | GPIO 27 |
| Green LED | GPIO 32 |
| Yellow LED | GPIO 33 |
| Red LED | GPIO 25 |

---

## LoRa Configuration

| Parameter | Value |
|---|---|
| Frequency | 433 MHz |
| Spreading Factor | 7 |
| Bandwidth | 125 kHz |
| Coding Rate | 4/5 |
| Sync Word | 0x12 |
| CRC | Enabled |

---

## Software Requirements

- Arduino IDE
- ESP32 Arduino Core
- LoRa library by Sandeep Mistry
- Wire library
- SPI library

---

## Repository Structure

```text
RescueMesh-LoRa-Mesh-Network/
│
├── firmware/
│   ├── node_A/
│   │   └── Node_A.ino
│   │
│   ├── node_B/
│   │   └── Node_B.ino
│   │
│   ├── node_C/
│   │   └── Node_C.ino
│   │
│   └── node_D/
│       └── Node_D.ino
│
├── documentation/
│   ├── system_architecture.md
│   └── communication_protocol.md
│
├── hardware/
│   ├── pin_configuration.md
│   └── hardware_components.md
│
├── images/
│   ├── system_architecture.png
│   ├── hardware_setup.jpg
│   └── dashboard.png
│
├── README.md
└── LICENSE
```

---

## Getting Started

### 1. Install Arduino IDE

Download and install the Arduino IDE.

### 2. Install ESP32 Board Support

Install the ESP32 board package through the Arduino IDE Boards Manager.

### 3. Install Required Libraries

Install the following library:

- LoRa by Sandeep Mistry

The SPI and Wire libraries are included with the Arduino framework.

### 4. Open Firmware

Navigate to the `firmware` folder and open the `.ino` file corresponding to the node you want to program.

### 5. Configure Hardware

Connect the ESP32, LoRa module, sensors, LEDs, and SOS button according to the pin configuration.

### 6. Upload Firmware

Select the appropriate ESP32 board and COM port, then upload the firmware.

### 7. Test Communication

Power up the nodes and test message transmission, reception, relaying, and SOS alerts.

---

## Project Status

RescueMesh is currently a working prototype with ESP32-based LoRa communication nodes.

Implemented features include node-to-node communication, message forwarding, duplicate filtering, SOS alerts, sensor-based alerts, and dashboard integration.

Further improvements are planned for network routing, recovery mechanisms, and node monitoring.

---

## Applications

- Disaster response communication
- Emergency alert systems
- Remote monitoring
- Infrastructure-independent communication
- Search and rescue operations
- Temporary communication networks

---

## Future Improvements

- Dynamic route selection
- Improved network recovery mechanisms
- Extended node monitoring
- Enhanced dashboard visualization
- GPS-based node location
- Power optimization
- Additional environmental sensors

---

## Project Team

Developed as a collaborative engineering project focused on resilient IoT communication and decentralized wireless networking.

---

**RescueMesh – Node failure must not become communication failure.**
