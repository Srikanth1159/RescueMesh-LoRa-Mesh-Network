# RescueMesh Communication Protocol

## 1. Overview

RescueMesh uses LoRa-based wireless communication to exchange messages between ESP32 nodes.

The protocol is designed to support decentralized communication, emergency message forwarding, and multi-hop transmission without relying on Wi-Fi or cellular infrastructure.

Each transmitted packet contains network identification, message identification, source node information, a forwarding limit, and the message payload.

## 2. Packet Format

RescueMesh uses the following packet structure:

NETWORK_ID|MESSAGE_ID|ORIGIN|TTL|PAYLOAD

### Example

RM_SK_7264|A-33|A|3|SOS:Rescue needed

## 3. Packet Fields

| Field | Description |
|---|---|
| NETWORK_ID | Identifies the RescueMesh network |
| MESSAGE_ID | Identifies an individual message |
| ORIGIN | Identifies the node that originally generated the message |
| TTL | Maximum forwarding limit |
| PAYLOAD | Contains message data or emergency information |

## 4. Network Identification

Each node uses a common Network ID to identify packets belonging to the RescueMesh network.

Packets containing an invalid Network ID are ignored.

Example:

RM_SK_7264

## 5. Message Identification

Each message contains a unique message identifier.

The identifier is used by receiving nodes to identify previously processed packets and prevent duplicate message handling.

Example:

A-33

## 6. Message Forwarding

When a node receives a valid message, it processes the packet and determines whether forwarding is permitted.

Forwarding allows messages to travel beyond the direct communication range of the original transmitter.

The receiving node decrements the TTL value before retransmitting the message.

## 7. Time-To-Live (TTL)

TTL controls the maximum forwarding limit of a message.

Each forwarding node decreases the TTL value.

Messages are not forwarded when the forwarding limit is reached.

This mechanism helps prevent unnecessary packet retransmissions.

## 8. Duplicate Message Filtering

Nodes maintain a record of previously received message identifiers.

When a packet is received:

1. The node extracts the message identifier.
2. The identifier is checked against previously processed messages.
3. Duplicate packets are discarded.
4. New packets are processed and may be forwarded.

Duplicate filtering helps reduce unnecessary network traffic.

## 9. Emergency Messages

RescueMesh supports emergency messages generated through:

- Manual SOS button activation
- Sensor-based vibration detection
- Threshold-based impact detection

Emergency messages are transmitted through the LoRa network and may be forwarded by other nodes.

Example:

RM_SK_7264|A-33|A|3|SOS:Rescue needed

## 10. Signal Strength Monitoring

Received Signal Strength Indicator (RSSI) values are monitored to evaluate the received signal strength of LoRa packets.

RSSI can be used to assess communication quality between nodes and assist in evaluating network performance.

## 11. Packet Error Checking

LoRa packet CRC is enabled to detect corrupted packets during wireless transmission.

Packets failing integrity checks are not treated as valid messages.

## 12. LoRa Configuration

| Parameter | Value |
|---|---|
| Frequency | 433 MHz |
| Spreading Factor | 7 |
| Bandwidth | 125 kHz |
| Coding Rate | 4/5 |
| Sync Word | 0x12 |
| CRC | Enabled |

## 13. Protocol Limitations

The current implementation is a prototype communication protocol.

Potential improvements include:

- Dynamic routing
- Improved message acknowledgement
- Collision avoidance
- Reliable message delivery
- Improved message ID generation
- Network congestion handling
- Node failure detection

---

**RescueMesh – Node failure must not become communication failure.**
