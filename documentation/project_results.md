# RescueMesh – Project Results and Performance Observations

## 1. Overview

RescueMesh is a decentralized wireless communication prototype developed using ESP32 microcontrollers and SX1278 LoRa modules.

The project demonstrates communication between multiple wireless nodes, emergency message transmission, message forwarding, duplicate filtering, and integration with a web-based monitoring dashboard.

The primary objective is to maintain communication between nodes even when conventional communication infrastructure is unavailable.

---

## 2. Hardware Implementation

The prototype consists of four ESP32-based communication nodes.

| Node | Hardware | Primary Function |
|------|----------|------------------|
| Node A | ESP32, SX1278, MPU6050 | Sensor-based emergency detection |
| Node B | ESP32, SX1278, SOS button, LEDs | Emergency communication and relay |
| Node C | ESP32, SX1278, SOS button, LED | Emergency communication and relay |
| Node D | ESP32, SX1278 | Gateway and dashboard integration |

Each node is designed to receive, process, and forward messages depending on the implemented firmware functionality.

---

## 3. Communication Results

### LoRa Communication

Wireless communication was successfully established between ESP32 nodes using SX1278 LoRa modules operating at 433 MHz.

The implemented communication configuration was:

| Parameter | Value |
|-----------|-------|
| Frequency | 433 MHz |
| Spreading Factor | SF7 |
| Bandwidth | 125 kHz |
| Coding Rate | 4/5 |
| Sync Word | 0x12 |
| CRC | Enabled |

Messages were transmitted and received successfully between nodes during prototype testing.

### Signal Strength Monitoring

Received Signal Strength Indicator (RSSI) values were monitored during wireless communication.

Observed RSSI values:

| Observation | RSSI |
|-------------|------|
| Strong signal | Approximately -29 dBm |
| Lower observed signal | Approximately -74 dBm |

The RSSI values varied depending on the distance and surrounding environment.

These measurements demonstrate the ability to monitor received signal strength during communication.

---

## 4. Emergency Communication

The prototype successfully demonstrated manual SOS message transmission using physical emergency buttons.

Emergency messages were transmitted using the RescueMesh packet format.

Example:

```text
RM_SK_7264|B-1|B|5|SOS:Manual emergency button


```
The receiving nodes identify emergency messages and activate their corresponding alert mechanisms.

The system supports:
- Manual emergency SOS transmission
- Emergency message reception
- Message identification
- Sender identification
- Emergency alert classification
- Message forwarding

## 5. Multi-Hop Communication
RescueMesh implements a message forwarding mechanism that allows nodes to relay received messages to neighboring nodes.

The forwarding mechanism includes:
- Message origin identification
- Unique message IDs
- TTL-based hop limitation
- Duplicate message filtering
- Random relay delay
The TTL value is reduced during forwarding to prevent messages from circulating indefinitely.

Multi-hop forwarding functionality is implemented in the node firmware.

Large-scale network coverage and multi-hop packet delivery performance require further experimental validation.

## 6. Duplicate Message Filtering
The prototype implements duplicate message filtering using the original sender identity and message ID.

Messages are identified using a combination of:
```
ORIGIN + MESSAGE_ID
```

Example:
```
A|A-130
```


When a previously processed message is received again, the node ignores the duplicate instead of processing or forwarding it repeatedly.

This mechanism helps reduce unnecessary network traffic.

## 7. Dashboard Integration
Node D acts as the gateway between the LoRa communication network and the web-based monitoring dashboard.

The gateway enables wireless emergency messages to be visualized through a dashboard interface.

During integration testing, emergency alert identifiers including:
```
A-130
A-131
```

were observed.

The dashboard integration demonstrates the potential for monitoring emergency communication events through a centralized interface without requiring every communication node to connect directly to the internet.

## 8. System Capabilities Demonstrated
|Feature|	Result|
|ESP32 initialization|	Demonstrated|
|LoRa wireless communication	|Demonstrated|
|Manual SOS transmission|	Demonstrated|
|Message reception|	Demonstrated|
|RSSI monitoring|	Demonstrated|
|Emergency message identification	|Implemented|
|Multi-hop forwarding	|Implemented|
|Duplicate filtering	|Implemented|
|TTL-based forwarding	|Implemented|
|Dashboard integration	|Demonstrated|
|Sensor-based impact detection|	Implemented|
|Node failure recovery	|Requires further testing|


## 9. Performance Limitations
The prototype has the following limitations:
- Communication range has not been systematically measured.
- Packet delivery ratio has not been experimentally calculated.
- Network latency has not been benchmarked.
- Large-scale node deployment has not been evaluated.
- Node failure recovery requires further controlled testing.
- Simultaneous transmissions may result in packet collisions.
- Sensor impact thresholds require calibration.
- The prototype does not currently provide end-to-end delivery acknowledgement.

## 10. Future Improvements
Potential improvements include:
- Implementing acknowledgement-based reliable communication.
- Improving relay coordination and collision avoidance.
- Testing alternative network topologies.
- Conducting communication range experiments.
- Measuring packet delivery ratio.
- Measuring end-to-end latency.
- Evaluating network behavior during node failure.
- Increasing the number of communication nodes.
- Improving sensor-based emergency detection.
- Enhancing the dashboard with real-time node status and network visualization.

## 11. Conclusion
RescueMesh demonstrates the feasibility of building a low-cost decentralized emergency communication prototype using ESP32 microcontrollers and LoRa technology.

The prototype successfully demonstrated wireless communication, SOS alert transmission, signal strength monitoring, and dashboard integration.

The implementation of multi-hop forwarding, duplicate filtering, and TTL-based message handling provides the foundation for a resilient communication network.

The project highlights the potential of decentralized wireless communication systems for emergency scenarios where conventional infrastructure may be unavailable.

