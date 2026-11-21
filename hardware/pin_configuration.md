# RescueMesh Hardware Pin Configuration

## 1. Overview

RescueMesh uses ESP32 microcontrollers interfaced with LoRa SX1278 communication modules and peripheral components.

The GPIO configuration varies between nodes depending on their hardware connections and functionalities.

This document describes the hardware pin configuration used in the RescueMesh prototype.

---

## 2. Node A Configuration

### ESP32 – LoRa SX1278

| LoRa Module Pin | ESP32 GPIO |
|---|---|
| NSS / SS | GPIO 5 |
| RESET | GPIO 14 |
| DIO0 | GPIO 26 |
| SCK | GPIO 18 |
| MISO | GPIO 19 |
| MOSI | GPIO 23 |

### MPU6050 – I2C Interface

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

## 3. Node B Configuration

### SOS Button and LEDs

| Component | ESP32 GPIO |
|---|---|
| SOS Button | GPIO 27 |
| Green LED | GPIO 32 |
| Yellow LED | GPIO 33 |
| Red LED | GPIO 25 |

The LoRa pin configuration should be verified against the Node B firmware.

---

## 4. Node C Configuration

### ESP32 – LoRa SX1278

| LoRa Module Pin | ESP32 GPIO |
|---|---|
| NSS / SS | GPIO 13 |
| RESET | GPIO 14 |
| DIO0 | GPIO 26 |
| SCK | GPIO 32 |
| MISO | GPIO 19 |
| MOSI | GPIO 23 |

### Additional Components

| Component | ESP32 GPIO |
|---|---|
| SOS Button | GPIO 27 |
| LED | GPIO 25 |

---

## 5. Node D Configuration

Node D is connected to the monitoring dashboard in the current implementation.

Refer to the Node D firmware for the specific GPIO assignments and hardware connections.

---

## 6. LoRa Communication Configuration

All LoRa modules must use compatible communication parameters to exchange packets.

| Parameter | Value |
|---|---|
| Frequency | 433 MHz |
| Spreading Factor | 7 |
| Bandwidth | 125 kHz |
| Coding Rate | 4/5 |
| Sync Word | 0x12 |
| CRC | Enabled |

---

## 7. Hardware Notes

- Ensure the LoRa module and ESP32 share a common ground.
- Verify the SPI connections before powering the modules.
- Use the appropriate power supply for the ESP32 and LoRa module.
- Connect the LoRa antenna before transmitting.
- Verify GPIO assignments against the corresponding firmware.
- Avoid short circuits during hardware assembly.

---

**RescueMesh – Node failure must not become communication failure.**
