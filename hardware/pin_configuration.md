# RescueMesh – Hardware Pin Configuration

This document describes the GPIO pin assignments, hardware connections, and communication configuration used by all RescueMesh ESP32-LoRa nodes.

---

## Node A – Sensor and Emergency Node

### LoRa SX1278 Module

| LoRa Pin | ESP32 GPIO |
|-----------|------------|
| NSS / CS  | GPIO 5 |
| RESET     | GPIO 14 |
| DIO0      | GPIO 26 |
| SCK       | GPIO 18 |
| MISO      | GPIO 19 |
| MOSI      | GPIO 23 |
| VCC       | 3.3V |
| GND       | GND |

### MPU6050 Accelerometer and Gyroscope

| MPU6050 Pin | ESP32 GPIO |
|-------------|------------|
| SDA         | GPIO 21 |
| SCL         | GPIO 22 |
| VCC         | 3.3V |
| GND         | GND |

- I2C Address: `0x68`

### Emergency SOS Button

| Component | ESP32 GPIO |
|-----------|------------|
| SOS Button | GPIO 27 |

- Configured using `INPUT_PULLUP`.
- Button connects GPIO 27 to GND when pressed.

### Status LEDs

| LED | ESP32 GPIO |
|-----|------------|
| Green LED | GPIO 32 |
| Yellow LED | GPIO 33 |
| Red LED | GPIO 25 |

---

## Node B – Relay and Emergency Node

### LoRa SX1278 Module

| LoRa Pin | ESP32 GPIO |
|-----------|------------|
| NSS / CS  | GPIO 5 |
| RESET     | GPIO 14 |
| DIO0      | GPIO 26 |
| SCK       | GPIO 18 |
| MISO      | GPIO 19 |
| MOSI      | GPIO 23 |
| VCC       | 3.3V |
| GND       | GND |

### Emergency SOS Button

| Component | ESP32 GPIO |
|-----------|------------|
| SOS Button | GPIO 27 |

- Configured using `INPUT_PULLUP`.
- Button connects GPIO 27 to GND when pressed.

### Status LEDs

| LED | ESP32 GPIO |
|-----|------------|
| Green LED | GPIO 32 |
| Yellow LED | GPIO 33 |
| Red LED | GPIO 25 |

---

## Node C – Kepler Node

### LoRa SX1278 Module

| LoRa Pin | ESP32 GPIO |
|-----------|------------|
| NSS / CS  | GPIO 13 |
| RESET     | GPIO 14 |
| DIO0      | GPIO 26 |
| SCK       | GPIO 32 |
| MISO      | GPIO 19 |
| MOSI      | GPIO 23 |
| VCC       | 3.3V |
| GND       | GND |

### Emergency SOS Button

| Component | ESP32 GPIO |
|-----------|------------|
| SOS Button | GPIO 27 |

- Configured using `INPUT_PULLUP`.
- Button connects GPIO 27 to GND when pressed.

### SOS Indicator LED

| Component | ESP32 GPIO |
|-----------|------------|
| SOS LED | GPIO 25 |

- Blinks during SOS transmission.
- Blinks when an SOS message is received.
- Blink interval: 250 ms.
- Blink duration: 5 seconds.

---

## Node D – Dashboard Gateway

### LoRa SX1278 Module

| LoRa Pin | ESP32 GPIO |
|-----------|------------|
| NSS / CS  | GPIO 5 |
| RESET     | GPIO 14 |
| DIO0      | GPIO 26 |
| SCK       | GPIO 18 |
| MISO      | GPIO 19 |
| MOSI      | GPIO 23 |
| VCC       | 3.3V |
| GND       | GND |

### SOS Hardware

| Component | ESP32 GPIO |
|-----------|------------|
| SOS Button | GPIO 27 |
| SOS LED | GPIO 25 |

### Wi-Fi Configuration

| Parameter | Value |
|-----------|-------|
| Wi-Fi Mode | Access Point |
| SSID | `RescueMesh-D` |
| Password | `RescueMesh7264` |
| Dashboard IP | `192.168.4.1` |

### Node D Functionality

Node D acts as the gateway between the LoRa mesh network and the web-based monitoring dashboard.

It receives messages from the LoRa network and provides a Wi-Fi access point for dashboard connectivity and emergency alert visualization.

---

## Common LoRa Communication Configuration

| Parameter | Value |
|-----------|-------|
| Frequency | 433 MHz |
| Spreading Factor | SF7 |
| Bandwidth | 125 kHz |
| Coding Rate | 4/5 |
| Sync Word | `0x12` |
| CRC | Enabled |
| Network ID | `RM_SK_7264` |

---

## Hardware Notes

- ESP32 GPIO pins operate at 3.3V logic levels.
- SX1278 LoRa modules require a suitable 3.3V power supply.
- Use an appropriate 433 MHz antenna for the LoRa module.
- LEDs should be connected through current-limiting resistors.
- SOS buttons use the internal pull-up resistor.
- GPIO assignments are specific to the firmware and physical wiring of each node.
- Node C uses a different SPI pin configuration compared to Nodes A, B, and D.

---

## Node Summary

| Node | LoRa | Sensors | SOS Button | Indicators | Main Function |
|------|------|---------|------------|------------|---------------|
| A | SX1278 | MPU6050 | GPIO 27 | 3 LEDs | Sensor and emergency detection |
| B | SX1278 | None | GPIO 27 | 3 LEDs | Relay and emergency node |
| C | SX1278 | None | GPIO 27 | 1 LED | Kepler emergency relay node |
| D | SX1278 | None | GPIO 27 | 1 LED | Dashboard gateway |
