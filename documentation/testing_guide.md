# RescueMesh – Testing and Validation Guide

This document describes the testing procedures used to validate the RescueMesh decentralized LoRa communication network.

The testing focuses on communication reliability, emergency alert transmission, multi-hop forwarding, duplicate packet filtering, node behavior, and dashboard integration.

---

## 1. Testing Objectives

The primary objectives of testing RescueMesh are:

- Verify LoRa communication between ESP32 nodes.
- Validate emergency SOS message transmission.
- Test multi-hop message forwarding.
- Verify duplicate message suppression.
- Validate TTL-based packet forwarding.
- Test communication during node failure.
- Monitor signal strength using RSSI.
- Verify sensor-triggered emergency alerts.
- Validate dashboard integration through Node D.

---

## 2. Test Environment

### Hardware

| Component | Quantity |
|-----------|----------|
| ESP32 Development Boards | 4 |
| SX1278 LoRa Modules | 4 |
| MPU6050 Accelerometer/Gyroscope | 1 |
| SOS Push Buttons | As required |
| LEDs | As required |
| Antennas | 4 |
| Connecting Wires | As required |

### Software

| Software | Purpose |
|----------|---------|
| Arduino IDE | Firmware development |
| Arduino LoRa Library | LoRa communication |
| ESP32 Arduino Core | ESP32 programming |
| Serial Monitor | Debugging and communication monitoring |
| Web Dashboard | Emergency alert visualization |

---

## 3. LoRa Communication Configuration

| Parameter | Configuration |
|-----------|---------------|
| Frequency | 433 MHz |
| Spreading Factor | SF7 |
| Bandwidth | 125 kHz |
| Coding Rate | 4/5 |
| Sync Word | 0x12 |
| CRC | Enabled |
| Network ID | RM_SK_7264 |

---

## 4. Test Cases

### Test Case 1 – Node Initialization

**Objective:** Verify successful initialization of each ESP32-LoRa node.

**Procedure:**

1. Power on the ESP32 node.
2. Open the Serial Monitor.
3. Observe the initialization messages.
4. Verify successful LoRa module initialization.

**Expected Result:**

The node initializes successfully and enters the communication-ready state.

**Status:** Verified during individual node testing.

---

### Test Case 2 – Two-Node LoRa Communication

**Objective:** Verify direct wireless communication between two RescueMesh nodes.

**Procedure:**

1. Power on two ESP32-LoRa nodes.
2. Ensure both nodes use the same network ID and LoRa parameters.
3. Send a message from one node.
4. Observe the receiving node's Serial Monitor.
5. Record the received message and RSSI.

**Expected Result:**

The receiving node successfully receives and displays the transmitted message.

**Observed Result:**

Two-node LoRa communication was successfully demonstrated.

RSSI measurements observed during testing ranged approximately from -29 dBm to -74 dBm, depending on the distance and environmental conditions.

**Status:** Verified.

---

### Test Case 3 – Emergency SOS Transmission

**Objective:** Verify manual SOS alert transmission.

**Procedure:**

1. Power on the RescueMesh nodes.
2. Press the SOS button on a node.
3. Observe the transmitting node's Serial Monitor.
4. Verify reception by other nodes.
5. Check the received emergency alert.

**Expected Result:**

The transmitting node generates an SOS message, and nearby nodes receive and identify it as an emergency alert.

**Status:** Verified during communication testing.

---

### Test Case 4 – Multi-Hop Message Forwarding

**Objective:** Verify message forwarding between nodes beyond direct communication range.

**Procedure:**

1. Deploy multiple nodes in a communication chain.
2. Send a message from the originating node.
3. Verify reception by the intermediate node.
4. Verify that the intermediate node forwards the message.
5. Observe reception by the next node.

**Expected Result:**

Messages are forwarded through intermediate nodes, allowing communication beyond the direct transmission range of the originating node.

**Status:** Implemented in firmware. Multi-hop validation should be performed for each network topology.

---

### Test Case 5 – Duplicate Message Filtering

**Objective:** Prevent repeated processing and forwarding of duplicate messages.

**Procedure:**

1. Transmit a message with a unique message ID.
2. Allow the receiving node to process the message.
3. Transmit or receive the same message again.
4. Observe the Serial Monitor.

**Expected Result:**

The node identifies the repeated message using the origin and message ID and ignores the duplicate.

**Status:** Implemented in firmware.

---

### Test Case 6 – TTL-Based Forwarding

**Objective:** Prevent messages from circulating indefinitely within the network.

**Procedure:**

1. Transmit a packet with an initial TTL value.
2. Allow intermediate nodes to receive the packet.
3. Observe the TTL value in forwarded packets.
4. Verify that forwarding stops when TTL reaches its hop limit.

**Expected Result:**

Each forwarding node reduces the TTL value by one.

Messages with TTL less than or equal to one are not forwarded.

**Status:** Implemented in firmware.

---

### Test Case 7 – Node Failure and Alternate Path

**Objective:** Evaluate communication resilience when a relay node becomes unavailable.

**Procedure:**

1. Establish communication between multiple nodes.
2. Disable or power off one intermediate node.
3. Transmit an emergency message.
4. Observe whether another available route forwards the message.

**Expected Result:**

The network should maintain communication through an alternative available path when network topology permits.

**Status:** Requires validation under controlled node-failure conditions.

---

### Test Case 8 – RSSI Monitoring

**Objective:** Evaluate received signal strength during wireless communication.

**Procedure:**

1. Establish LoRa communication between nodes.
2. Transmit multiple messages.
3. Record RSSI values from the receiving node.
4. Repeat at different distances and environmental conditions.

**Expected Result:**

RSSI values are displayed for received packets and can be used to evaluate communication quality.

**Observed Result:**

RSSI values ranging approximately from -29 dBm to -74 dBm were observed during testing.

**Status:** Verified.

---

### Test Case 9 – MPU6050 Impact Detection

**Objective:** Verify sensor-based impact or vibration detection.

**Procedure:**

1. Power on Node A.
2. Ensure the MPU6050 sensor is initialized.
3. Monitor sensor readings.
4. Simulate vibration or impact conditions.
5. Verify that the node generates the corresponding alert.

**Expected Result:**

The sensor detects the configured impact or vibration condition and triggers an alert message.

**Status:** Implemented in firmware. Sensor threshold validation should be performed under controlled conditions.

---

### Test Case 10 – Dashboard Integration

**Objective:** Verify communication between the LoRa network and the web dashboard.

**Procedure:**

1. Power on Node D.
2. Connect a device to the RescueMesh Wi-Fi access point.
3. Open the dashboard.
4. Transmit an emergency message from another node.
5. Observe the dashboard for alert updates.

**Expected Result:**

The gateway receives LoRa messages and forwards the relevant information to the dashboard.

**Observed Result:**

Emergency alert identifiers including A-130 and A-131 were observed during dashboard integration testing.

**Status:** Verified during prototype demonstration.

---

## 5. Test Summary

| Test | Description | Status |
|------|-------------|--------|
| 1 | Node Initialization | Verified |
| 2 | Two-Node Communication | Verified |
| 3 | SOS Transmission | Verified |
| 4 | Multi-Hop Forwarding | Implemented |
| 5 | Duplicate Filtering | Implemented |
| 6 | TTL-Based Forwarding | Implemented |
| 7 | Node Failure Recovery | Requires validation |
| 8 | RSSI Monitoring | Verified |
| 9 | MPU6050 Impact Detection | Implemented |
| 10 | Dashboard Integration | Verified |

---

## 6. Limitations

- Communication range depends on environmental conditions, antenna placement, and obstacles.
- The prototype uses a limited number of nodes.
- Network performance may vary depending on node density and simultaneous transmissions.
- Random relay delays reduce the probability of packet collisions but do not guarantee collision-free communication.
- The prototype does not currently implement end-to-end delivery acknowledgements.
- Node failure recovery depends on the availability of alternative communication paths.
- Sensor thresholds require calibration for different environmental conditions.
- The network has not undergone large-scale deployment testing.

---

## 7. Conclusion

Testing demonstrated the feasibility of establishing decentralized communication using ESP32 and LoRa modules.

The prototype successfully demonstrated two-node communication, emergency message transmission, RSSI monitoring, and dashboard integration.

Additional testing is required to quantify multi-hop network reliability, node failure recovery, communication range, and performance under larger network conditions.

RescueMesh demonstrates the potential of low-cost wireless communication systems for emergency communication scenarios where conventional infrastructure may be unavailable.
