# RescueMesh – Bill of Materials (BOM)

This document lists the hardware components used in the
RescueMesh decentralized LoRa communication network prototype.

## Hardware Components

| Component | Quantity | Purpose |
|---|---:|---|
| ESP32 Development Board | 4 | Main processing and wireless network control |
| SX1278 LoRa Module (433 MHz) | 4 | Long-range wireless communication |
| MPU6050 IMU Sensor | 1 | Motion and vibration detection |
| Push Buttons | 4 | Manual emergency SOS triggering |
| LEDs | Multiple | Visual node status indication |
| Resistors | As required | LED current limiting |
| Jumper Wires | As required | Hardware interconnections |
| Breadboards | As required | Prototype circuit assembly |
| USB Cables | 4 | Programming and power supply |
| LoRa Antennas | 4 | Wireless signal transmission and reception |

## Hardware Description

### ESP32 Development Board

The ESP32 serves as the main controller for each network node.
It handles LoRa communication, message processing, emergency
trigger detection, and node status monitoring.

### SX1278 LoRa Module

The SX1278 module provides long-range, low-power wireless
communication between RescueMesh nodes using the 433 MHz band.

### MPU6050 Sensor

The MPU6050 is integrated with Node A to detect motion,
vibration, or impact events that can be used to trigger emergency alerts.

### Emergency Push Buttons

Push buttons allow users to manually trigger SOS messages
through the network.

### Status LEDs

LED indicators provide visual feedback regarding node status
and emergency conditions.

## Prototype Configuration

The prototype consists of four ESP32-based nodes.

- Node A: Sensor-enabled emergency node
- Node B: Communication and relay node
- Node C: Communication and relay node
- Node D: Gateway and dashboard interface

## Notes

The quantities listed represent the intended prototype configuration.
Actual component quantities may vary depending on the final
hardware assembly.
