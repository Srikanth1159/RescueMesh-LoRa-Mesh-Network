
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <math.h>

// ========== LORA PINS ==========
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23

// ========== BUTTON AND LED PINS ==========
#define SOS_BUTTON 27
#define GREEN_LED  32
#define YELLOW_LED 33
#define RED_LED    25

// ========== ACCELEROMETER ==========
#define SENSOR_SDA  21
#define SENSOR_SCL  22
#define SENSOR_ADDR 0x68

// ========== NETWORK SETTINGS ==========
const long LORA_FREQUENCY = 433E6;
const char* NETWORK_ID = "RM_SK_7264";
const char* NODE_ID = "A";

// ========== SENSOR THRESHOLDS ==========
const float VIBRATION_THRESHOLD = 0.20;
const float HIGH_IMPACT_THRESHOLD = 1.00;

const unsigned long SENSOR_INTERVAL_MS = 10;
const unsigned long QUIET_TIME_MS = 500;
const unsigned long EVENT_COOLDOWN_MS = 1000;

// ========== SOS LED SETTINGS ==========
const unsigned long SOS_BLINK_DURATION_MS = 5000;
const unsigned long SOS_BLINK_INTERVAL_MS = 250;

// ========== PROGRAM VARIABLES ==========
bool loraReady = false;
bool sensorReady = false;
bool previousButtonState = HIGH;

unsigned long messageNumber = 0;
unsigned long lastPressTime = 0;
unsigned long lastSensorTime = 0;
unsigned long quietSince = 0;
unsigned long lastEventTime = 0;

bool eventArmed = true;

enum LedMode {
  IDLE_MODE,
  VIBRATION_MODE,
  HIGH_IMPACT_MODE
};

LedMode sensorMode = IDLE_MODE;
LedMode remoteMode = IDLE_MODE;

unsigned long remoteModeUntil = 0;
unsigned long sosBlinkUntil = 0;
unsigned long lastBlinkTime = 0;

bool redBlinkState = false;

// ---------- DUPLICATE MESSAGE FILTER ----------
String seenMessages[20];
int seenIndex = 0;

String makeMessageKey(String origin, String id) {
  return origin + "|" + id;
}

bool alreadySeen(String origin, String id) {
  String key = makeMessageKey(origin, id);

  for (int i = 0; i < 20; i++) {
    if (seenMessages[i] == key) {
      return true;
    }
  }

  return false;
}

void rememberMessage(String origin, String id) {
  seenMessages[seenIndex] = makeMessageKey(origin, id);
  seenIndex = (seenIndex + 1) % 20;
}

// ---------- SAFE RAW-PACKET DISPLAY ----------
// Non-printable bytes are displayed in hexadecimal
// so corrupted data does not make the Serial Monitor unreadable.
void printSafePacket(const String& packet) {
  Serial.print("Raw packet received: [");

  for (unsigned int i = 0; i < packet.length(); i++) {
    uint8_t c = (uint8_t)packet[i];

    if (c >= 32 && c <= 126) {
      Serial.write(c);
    } else {
      char hexBuffer[6];
      snprintf(hexBuffer, sizeof(hexBuffer), "<%02X>", c);
      Serial.print(hexBuffer);
    }
  }

  Serial.println("]");
}

// ---------- LED CONTROL ----------
void allLEDsOff() {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
}

void startSOSBlink() {
  sosBlinkUntil = millis() + SOS_BLINK_DURATION_MS;
  lastBlinkTime = 0;
  redBlinkState = false;
}

void updateLEDs() {
  unsigned long now = millis();

  // SOS: red LED blinks.
  if ((long)(now - sosBlinkUntil) < 0) {
    if (lastBlinkTime == 0 ||
        now - lastBlinkTime >= SOS_BLINK_INTERVAL_MS) {

      lastBlinkTime = now;
      redBlinkState = !redBlinkState;

      allLEDsOff();
      digitalWrite(RED_LED, redBlinkState ? HIGH : LOW);
    }
    return;
  }

  // Brief display for alerts received from other nodes.
  if ((long)(now - remoteModeUntil) < 0) {
    allLEDsOff();

    if (remoteMode == VIBRATION_MODE) {
      digitalWrite(YELLOW_LED, HIGH);
    } else if (remoteMode == HIGH_IMPACT_MODE) {
      digitalWrite(RED_LED, HIGH);
    } else {
      digitalWrite(GREEN_LED, HIGH);
    }
    return;
  }

  // Local sensor status.
  allLEDsOff();

  if (sensorMode == VIBRATION_MODE) {
    digitalWrite(YELLOW_LED, HIGH);
  } else if (sensorMode == HIGH_IMPACT_MODE) {
    digitalWrite(RED_LED, HIGH);
  } else {
    // Idle: green stays on.
    digitalWrite(GREEN_LED, HIGH);
  }
}

void showRemoteEvent(String payload) {
  if (payload.indexOf("SOS") >= 0) {
    startSOSBlink();
    return;
  }

  if (payload.indexOf("HIGH_IMPACT") >= 0 ||
      payload.indexOf("IMPACT") >= 0) {

    remoteMode = HIGH_IMPACT_MODE;
    remoteModeUntil = millis() + 3000;

  } else if (payload.indexOf("VIBRATION") >= 0) {

    remoteMode = VIBRATION_MODE;
    remoteModeUntil = millis() + 3000;
  }

  updateLEDs();
}

// ---------- SEND LORA MESSAGE ----------
void sendMessage(String payload) {
  if (!loraReady) {
    Serial.println("ERROR: LoRa is not ready.");
    return;
  }

  messageNumber++;

  String id = "A-" + String(messageNumber);

  String packet = String(NETWORK_ID) + "|" +
                  id + "|A|5|" + payload;

  rememberMessage("A", id);

  Serial.print("Sending alert: ");
  Serial.println(payload);

  LoRa.beginPacket();
  LoRa.print(packet);

  int result = LoRa.endPacket();

  if (result == 1) {
    Serial.println("Message transmission completed.");
  } else {
    Serial.println("Message transmission failed.");
  }
}

// ---------- SENSOR REGISTER HELPERS ----------
bool writeSensorRegister(byte reg, byte value) {
  Wire.beginTransmission(SENSOR_ADDR);
  Wire.write(reg);
  Wire.write(value);

  return Wire.endTransmission() == 0;
}

bool readSensorRegister(byte reg, byte &value) {
  Wire.beginTransmission(SENSOR_ADDR);
  Wire.write(reg);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(
        (uint8_t)SENSOR_ADDR, (uint8_t)1) != 1) {
    return false;
  }

  value = Wire.read();
  return true;
}

// ---------- INITIALIZE SENSOR ----------
bool initializeSensor() {
  Wire.begin(SENSOR_SDA, SENSOR_SCL);
  Wire.setClock(100000);

  Wire.beginTransmission(SENSOR_ADDR);

  if (Wire.endTransmission() != 0) {
    Serial.println("ERROR: Sensor not found at address 0x68.");
    return false;
  }

  byte identity = 0;

  if (readSensorRegister(0x75, identity)) {
    Serial.print("Sensor ID: 0x");
    Serial.println(identity, HEX);
  }

  // Wake the sensor.
  if (!writeSensorRegister(0x6B, 0x00)) {
    Serial.println("ERROR: Could not start sensor.");
    return false;
  }

  delay(100);

  // Configure for +/-4g, if supported by this sensor.
  if (!writeSensorRegister(0x1C, 0x08)) {
    Serial.println("ERROR: Sensor configuration failed.");
    return false;
  }

  byte testValue;

  if (!readSensorRegister(0x3B, testValue)) {
    Serial.println("ERROR: Sensor data unavailable.");
    return false;
  }

  Serial.println("Accelerometer registers are readable.");
  return true;
}

// ---------- READ ACCELERATION ----------
bool readAccelerationMagnitude(float &magnitudeG) {
  Wire.beginTransmission(SENSOR_ADDR);
  Wire.write(0x3B);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(
        (uint8_t)SENSOR_ADDR, (uint8_t)6) != 6) {
    return false;
  }

  int16_t rawX = (int16_t)(
    ((uint16_t)Wire.read() << 8) | Wire.read());

  int16_t rawY = (int16_t)(
    ((uint16_t)Wire.read() << 8) | Wire.read());

  int16_t rawZ = (int16_t)(
    ((uint16_t)Wire.read() << 8) | Wire.read());

  // Assumes compatible sensor registers and +/-4g range.
  float ax = rawX / 8192.0;
  float ay = rawY / 8192.0;
  float az = rawZ / 8192.0;

  magnitudeG = sqrt(ax * ax + ay * ay + az * az);
  return true;
}

// ---------- VIBRATION AND IMPACT DETECTION ----------
void checkImpact() {
  if (!sensorReady ||
      millis() - lastSensorTime < SENSOR_INTERVAL_MS) {
    return;
  }

  lastSensorTime = millis();

  float magnitudeG = 0;

  if (!readAccelerationMagnitude(magnitudeG)) {
    return;
  }

  // Approximate change from the resting 1g magnitude.
  float changeG = fabs(magnitudeG - 1.0);

  // Rearm only after the sensor is quiet.
  if (changeG < 0.12) {
    if (quietSince == 0) {
      quietSince = millis();
    }

    if (millis() - quietSince >= QUIET_TIME_MS) {
      sensorMode = IDLE_MODE;

      if (millis() - lastEventTime >= EVENT_COOLDOWN_MS) {
        eventArmed = true;
      }
    }
  } else {
    quietSince = 0;
  }

  if (changeG >= HIGH_IMPACT_THRESHOLD) {
    sensorMode = HIGH_IMPACT_MODE;
  } else if (changeG >= VIBRATION_THRESHOLD) {
    sensorMode = VIBRATION_MODE;
  }

  if (!eventArmed) {
    return;
  }

  String payload;

  if (changeG >= HIGH_IMPACT_THRESHOLD) {
    Serial.println("HIGH IMPACT DETECTED");
    payload = "HIGH_IMPACT: High impact detected";
  } else if (changeG >= VIBRATION_THRESHOLD) {
    Serial.println("VIBRATION DETECTED");
    payload = "VIBRATION: Moderate vibration detected";
  } else {
    return;
  }

  eventArmed = false;
  lastEventTime = millis();

  sendMessage(payload);
}

// ---------- RECEIVE AND RELAY ----------
void receiveAndRelay() {
  int packetSize = LoRa.parsePacket();

  if (!packetSize) {
    return;
  }

  String packet = "";

  while (LoRa.available()) {
    packet += (char)LoRa.read();
  }

  // Print the received packet only when it cannot be parsed.
  int p1 = packet.indexOf('|');
  int p2 = packet.indexOf('|', p1 + 1);
  int p3 = packet.indexOf('|', p2 + 1);
  int p4 = packet.indexOf('|', p3 + 1);

  if (p1 < 0 || p2 < 0 || p3 < 0 || p4 < 0) {
    Serial.println();
    Serial.println("WARNING: Invalid message format.");
    printSafePacket(packet);
    Serial.println("Check sender format and radio settings.");
    return;
  }

  String network = packet.substring(0, p1);
  String id = packet.substring(p1 + 1, p2);
  String origin = packet.substring(p2 + 1, p3);
  int ttl = packet.substring(p3 + 1, p4).toInt();
  String payload = packet.substring(p4 + 1);

  if (network != NETWORK_ID) {
    Serial.println("Message from another network ignored.");
    return;
  }

  if (origin == NODE_ID) {
    return;
  }

  if (alreadySeen(origin, id)) {
    Serial.println("Duplicate message ignored.");
    return;
  }

  rememberMessage(origin, id);

  Serial.println();
  Serial.println("========== MESSAGE RECEIVED ==========");

  if (payload.indexOf("SOS") >= 0) {
    Serial.println("EMERGENCY SOS RECEIVED!");
  } else if (payload.indexOf("HIGH_IMPACT") >= 0 ||
             payload.indexOf("IMPACT") >= 0) {
    Serial.println("HIGH IMPACT ALERT RECEIVED!");
  } else if (payload.indexOf("VIBRATION") >= 0) {
    Serial.println("VIBRATION ALERT RECEIVED!");
  } else {
    Serial.println("MESSAGE RECEIVED!");
  }

  Serial.print("Original sender: Node ");
  Serial.println(origin);

  Serial.print("Message ID: ");
  Serial.println(id);

  Serial.print("Details: ");
  Serial.println(payload);

  Serial.print("Signal strength: ");
  Serial.print(LoRa.packetRssi());
  Serial.println(" dBm");

  showRemoteEvent(payload);

  if (ttl > 1) {
    String forwarded = String(NETWORK_ID) + "|" +
                       id + "|" +
                       origin + "|" +
                       String(ttl - 1) + "|" +
                       payload;

    delay(random(50, 200));

    Serial.println("Forwarding alert to nearby nodes...");

    LoRa.beginPacket();
    LoRa.print(forwarded);

    if (LoRa.endPacket() == 1) {
      Serial.println("Alert forwarding transmission completed.");
    } else {
      Serial.println("Alert forwarding failed.");
    }
  }

  Serial.println("======================================");
}

// ---------- MANUAL SOS BUTTON ----------
void checkButton() {
  bool currentState = digitalRead(SOS_BUTTON);

  if (previousButtonState == HIGH &&
      currentState == LOW &&
      millis() - lastPressTime > 300) {

    lastPressTime = millis();

    Serial.println("MANUAL SOS BUTTON PRESSED");

    startSOSBlink();
    sendMessage("SOS:Manual emergency button");
  }

  previousButtonState = currentState;
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(SOS_BUTTON, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  allLEDsOff();
  digitalWrite(GREEN_LED, HIGH);

  randomSeed(micros());

  Serial.println();
  Serial.println("========================================");
  Serial.println("         RESCUEMESH NODE A");
  Serial.println("========================================");

  Serial.println("Starting accelerometer...");
  sensorReady = initializeSensor();

  if (sensorReady) {
    Serial.println("Vibration and impact monitoring enabled.");
  } else {
    Serial.println("Sensor unavailable; manual SOS remains enabled.");
  }

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("ERROR: LoRa initialization failed.");
    return;
  }

  LoRa.setSyncWord(0x12);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  // Phase 1: CRC error checking.
  LoRa.enableCrc();

  loraReady = true;

  Serial.println("LoRa initialized successfully.");
  Serial.println("Packet CRC enabled.");
  Serial.println("Node A is ready.");
  Serial.println("Green = idle");
  Serial.println("Yellow = vibration");
  Serial.println("Red = high impact");
  Serial.println("Blinking red = manual SOS");
  Serial.println("========================================");
}

// ---------- MAIN LOOP ----------
void loop() {
  checkButton();

  if (loraReady) {
    receiveAndRelay();
  }

  checkImpact();
  updateLEDs();
}

