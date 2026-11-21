
#include <SPI.h>
#include <LoRa.h>

// ========== KEPLER NODE C PINS ==========
// Keep these matched to your friend's actual wiring.
#define LORA_SS     13
#define LORA_RST    14
#define LORA_DIO0   26
#define LORA_SCK    32
#define LORA_MISO   19
#define LORA_MOSI   23

// ========== SOS HARDWARE ==========
#define SOS_BUTTON  27
#define SOS_LED     25

// ========== NETWORK SETTINGS ==========
const long LORA_FREQUENCY = 433E6;
const char* NETWORK_ID = "RM_SK_7264";
const char* NODE_ID = "C";

// ========== PROGRAM VARIABLES ==========
bool loraReady = false;
bool previousButtonState = HIGH;

unsigned long lastPressTime = 0;
unsigned long messageNumber = 0;

// Non-blocking SOS LED blinking
bool blinkActive = false;
bool ledState = false;
unsigned long blinkStartTime = 0;
unsigned long lastBlinkTime = 0;

const unsigned long BLINK_INTERVAL_MS = 250;
const unsigned long BLINK_DURATION_MS = 5000;

// Remember recent messages.
// Include origin as IDs like A-1 and B-1 can overlap.
String seenMessages[30];
int seenIndex = 0;

// ========== DUPLICATE MESSAGE FILTER ==========
String makeMessageKey(String origin, String id) {
  return origin + "|" + id;
}

bool alreadySeen(String origin, String id) {
  String key = makeMessageKey(origin, id);

  for (int i = 0; i < 30; i++) {
    if (seenMessages[i] == key) {
      return true;
    }
  }

  return false;
}

void rememberMessage(String origin, String id) {
  seenMessages[seenIndex] = makeMessageKey(origin, id);
  seenIndex = (seenIndex + 1) % 30;
}

// ========== SOS LED ==========
void startSOSBlink() {
  blinkActive = true;
  ledState = false;
  blinkStartTime = millis();
  lastBlinkTime = 0;
  digitalWrite(SOS_LED, LOW);
}

void updateSOSLED() {
  if (!blinkActive) {
    digitalWrite(SOS_LED, LOW);
    return;
  }

  if (millis() - blinkStartTime >= BLINK_DURATION_MS) {
    blinkActive = false;
    ledState = false;
    digitalWrite(SOS_LED, LOW);
    return;
  }

  if (lastBlinkTime == 0 ||
      millis() - lastBlinkTime >= BLINK_INTERVAL_MS) {

    lastBlinkTime = millis();
    ledState = !ledState;

    digitalWrite(SOS_LED, ledState ? HIGH : LOW);
  }
}

// ========== SEND SOS ==========
void sendSOS() {
  if (!loraReady) {
    Serial.println("ERROR: LoRa is not ready.");
    return;
  }

  messageNumber++;

  String messageID = "C-" + String(messageNumber);

  String message = String(NETWORK_ID) + "|" +
                   messageID + "|C|5|" +
                   "SOS:Manual emergency button";

  // Remember own message so an echoed transmission is ignored.
  rememberMessage("C", messageID);

  Serial.println();
  Serial.println("========================================");
  Serial.println("        RESCUEMESH - KEPLER NODE C");
  Serial.println("========================================");
  Serial.println("MANUAL SOS BUTTON PRESSED");
  Serial.print("Message ID: ");
  Serial.println(messageID);

  Serial.println("Sending emergency SOS...");
  startSOSBlink();

  LoRa.beginPacket();
  LoRa.print(message);

  int result = LoRa.endPacket();

  if (result == 1) {
    Serial.println("SOS transmission completed.");
    Serial.println("Remote receipt is not yet confirmed.");
  } else {
    Serial.println("ERROR: SOS transmission failed.");
  }

  Serial.println("========================================");
}

// ========== RECEIVE AND RELAY ==========
void receiveAndRelay() {
  int packetSize = LoRa.parsePacket();

  // CRC-invalid packets may be discarded by the library.
  if (!packetSize) {
    return;
  }

  String packet = "";

  while (LoRa.available()) {
    packet += (char)LoRa.read();
  }

  int rssi = LoRa.packetRssi();

  int p1 = packet.indexOf('|');
  int p2 = packet.indexOf('|', p1 + 1);
  int p3 = packet.indexOf('|', p2 + 1);
  int p4 = packet.indexOf('|', p3 + 1);

  if (p1 < 0 || p2 < 0 || p3 < 0 || p4 < 0) {
    Serial.println("WARNING: Invalid message format.");
    return;
  }

  String network = packet.substring(0, p1);
  String messageID = packet.substring(p1 + 1, p2);
  String origin = packet.substring(p2 + 1, p3);
  int ttl = packet.substring(p3 + 1, p4).toInt();
  String payload = packet.substring(p4 + 1);

  // Accept only RescueMesh messages.
  if (network != NETWORK_ID) {
    return;
  }

  // Ignore this node's own messages.
  if (origin == NODE_ID) {
    return;
  }

  // Ignore already-processed messages.
  if (alreadySeen(origin, messageID)) {
    Serial.println("Duplicate message ignored.");
    return;
  }

  rememberMessage(origin, messageID);

  Serial.println();
  Serial.println("========== MESSAGE RECEIVED ==========");

  if (payload.indexOf("SOS") >= 0) {
    Serial.println("EMERGENCY SOS RECEIVED!");
    startSOSBlink();
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
  Serial.println(messageID);

  Serial.print("Details: ");
  Serial.println(payload);

  Serial.print("Signal strength: ");
  Serial.print(rssi);
  Serial.println(" dBm");

  // Forward the original sender and ID with a reduced TTL.
  if (ttl > 1) {
    String forwarded = String(NETWORK_ID) + "|" +
                       messageID + "|" +
                       origin + "|" +
                       String(ttl - 1) + "|" +
                       payload;

    // A short random delay can reduce some simultaneous relays.
    delay(random(50, 200));

    Serial.println("Forwarding message to nearby nodes...");

    LoRa.beginPacket();
    LoRa.print(forwarded);

    int result = LoRa.endPacket();

    if (result == 1) {
      Serial.println("Relay transmission completed.");
    } else {
      Serial.println("ERROR: Relay transmission failed.");
    }
  } else {
    Serial.println("Hop limit reached; not relaying.");
  }

  Serial.println("======================================");
}

// ========== SOS BUTTON ==========
void checkSOSButton() {
  bool currentState = digitalRead(SOS_BUTTON);

  if (previousButtonState == HIGH &&
      currentState == LOW &&
      millis() - lastPressTime > 300) {

    lastPressTime = millis();
    sendSOS();
  }

  previousButtonState = currentState;
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(SOS_BUTTON, INPUT_PULLUP);
  pinMode(SOS_LED, OUTPUT);
  digitalWrite(SOS_LED, LOW);

  randomSeed(micros());

  Serial.println();
  Serial.println("========================================");
  Serial.println("        RESCUEMESH - KEPLER NODE C");
  Serial.println("========================================");

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("ERROR: LoRa initialization failed.");
    Serial.println("Check wiring and the 3.3V supply.");

    loraReady = false;
    return;
  }

  // Match Nodes A and B.
  LoRa.setSyncWord(0x12);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  // Phase 1: enable packet CRC checking.
  LoRa.enableCrc();

  loraReady = true;

  Serial.println("LoRa initialized successfully.");
  Serial.println("Packet CRC enabled.");
  Serial.println("Node identity: C");
  Serial.println("Ready to receive, relay and send SOS.");
  Serial.println("Press the button to send an emergency SOS.");
  Serial.println("========================================");
}

// ========== MAIN LOOP ==========
void loop() {
  if (loraReady) {
    checkSOSButton();
    receiveAndRelay();
  }

  updateSOSLED();
}

