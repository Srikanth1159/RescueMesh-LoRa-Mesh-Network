# RescueMesh – Setup and Installation Guide

This guide explains how to configure the development environment,
upload firmware to RescueMesh nodes, and start the communication network.

## 1. Requirements

### Hardware
- ESP32 development boards
- SX1278 LoRa modules (433 MHz)
- MPU6050 sensor (Node A)
- Push buttons
- LEDs
- Jumper wires
- Breadboard

### Software
- Arduino IDE
- ESP32 Board Package
- LoRa library
- MPU6050 library (Node A)
- USB drivers for the ESP32 board

## 2. Arduino IDE Installation

1. Download and install Arduino IDE.
2. Open Arduino IDE.
3. Navigate to File → Preferences.
4. Add the ESP32 board package URL to Additional Boards Manager URLs.

ESP32 Board Package URL:

https://espressif.github.io/arduino-esp32/package_esp32_index.json

5. Open Tools → Board → Boards Manager.
6. Search for ESP32.
7. Install the ESP32 board package.

## 3. Required Libraries

Install the following libraries through Arduino IDE Library Manager:

- LoRa by Sandeep Mistry
- Adafruit MPU6050
- Adafruit Unified Sensor

Additional libraries may be required depending on the
individual node firmware.

## 4. Hardware Connections

Refer to:

hardware/pin_configuration.md

for the pin configuration of each node.

Ensure that the LoRa module is connected correctly before
powering the ESP32.

## 5. Firmware Upload

Each node has its own firmware.

Firmware locations:

- firmware/node_A/Node_A.ino
- firmware/node_B/Node_B.ino
- firmware/node_C/Node_C.ino
- firmware/node_D/Node_D.ino

### Upload Procedure

1. Connect the ESP32 board to your computer using USB.
2. Open the corresponding Arduino sketch.
3. Select the correct ESP32 board under Tools → Board.
4. Select the appropriate COM port.
5. Verify the firmware.
6. Upload the firmware.
7. Open Serial Monitor.
8. Set the baud rate according to the firmware configuration.

Repeat the procedure for each node.

## 6. LoRa Communication Configuration

The network uses the following communication parameters:

| Parameter | Configuration |
|---|---|
| Frequency | 433 MHz |
| Spreading Factor | SF7 |
| Bandwidth | 125 kHz |
| Coding Rate | 4/5 |
| Sync Word | 0x12 |
| CRC | Enabled |
| Network ID | RM_SK_7264 |

All nodes must use compatible LoRa communication settings.

## 7. Network Operation

Once all nodes are powered on:

1. Nodes initialize their LoRa modules.
2. Nodes enter receive mode.
3. An SOS message can be triggered using the configured button.
4. Nearby nodes receive the message.
5. Nodes relay eligible messages according to the TTL.
6. Duplicate messages are filtered.
7. Node D provides the dashboard gateway functionality.

## 8. Dashboard

Node D provides the Wi-Fi dashboard interface.

Connect to the configured Wi-Fi access point using the credentials
defined in the Node D firmware.

Open the dashboard using the gateway IP address.

Default gateway IP:

192.168.4.1

Verify the actual IP address from the Node D firmware or Serial Monitor.

## 9. Troubleshooting

### LoRa initialization failed

- Check the wiring.
- Verify SPI pins.
- Verify NSS, RESET and DIO0 pins.
- Check that the correct LoRa library is installed.
- Confirm that the module is compatible with the selected frequency.

### No messages received

- Check that both nodes use the same frequency.
- Verify the network configuration.
- Check antenna connections.
- Ensure both nodes are powered.
- Check Serial Monitor output.

### ESP32 not detected

- Install the appropriate USB driver.
- Try another USB cable.
- Try another USB port.
- Check the selected COM port.

### Dashboard not accessible

- Connect to the correct Wi-Fi access point.
- Verify Node D is powered.
- Check the gateway IP address.
- Confirm the dashboard server is running.

## 10. Safety and Usage

RescueMesh is an experimental prototype developed for educational
and demonstration purposes.

It is not a certified emergency communication system and should
not be relied upon for real-life emergency situations.

Communication performance depends on environmental conditions,
antenna configuration, interference and hardware limitations.
