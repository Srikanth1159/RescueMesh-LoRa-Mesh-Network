
#include <SPI.h>
#include <LoRa.h>

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

// ========== NETWORK SETTINGS ==========
const long LORA_FREQUENCY = 433E6;
const char* NETWORK_ID = "RM_SK_7264";
const char* NODE_ID = "B";

// ========== PROGRAM VARIABLES ==========
bool loraReady = false;
bool previousButtonState = HIGH;

unsigned long lastPressTime = 0;
unsigned long messageNumber = 0;

unsigned long ledTime = 0;
bool ledActive = false;

// Duplicate protection: remember recent origin + message IDs.
String seenMessages[20];
int seenIndex = 0;

// ========== DUPLICATE FILTER ==========
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

// ========== LED CONTROL ==========
void allLEDsOff() {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
}

void setStatusLED(String payload) {
  allLEDsOff();

  if (payload.indexOf("SOS") >= 0 ||
      payload.indexOf("HIGH_IMPACT") >= 0 ||
      payload.indexOf("SLIGHT_IMPACT") >= 0 ||
      payload.indexOf("IMPACT") >= 0 ||
      payload.indexOf("LOW_IMPACT") >= 0) {

    digitalWrite(RED_LED, HIGH);

  } else if (payload.indexOf("VIBRATION") >= 0) {

    digitalWrite(YELLOW_LED, HIGH);

  } else {

    digitalWrite(GREEN_LED, HIGH);
  }

  ledTime = millis();
  ledActive = true;
}

// ========== SEND MESSAGE ==========
void sendMessage(String payload) {
  if (!loraReady) {
    Serial.println("ERROR: LoRa is not ready.");
    return;
  }

  messageNumber++;

  String messageID = "B-" + String(messageNumber);

  String packet = String(NETWORK_ID) + "|" +
                  messageID + "|B|5|" + payload;

  // Remember this message so the echoed transmission is ignored.
  rememberMessage("B", messageID);

  Serial.println();
  Serial.println("========================================");
  Serial.println("        RESCUEMESH - NODE B");
  Serial.println("========================================");

  Serial.println("Sending emergency message.");
  Serial.print("Message ID: ");
  Serial.println(messageID);

  Serial.print("Details: ");
  Serial.println(payload);

  setStatusLED(payload);

  LoRa.beginPacket();
  LoRa.print(packet);

  int result = LoRa.endPacket();

  if (result == 1) {
    Serial.println("Message transmission completed.");
    Serial.println("This does not confirm remote delivery.");
  } else {
    Serial.println("ERROR: Message transmission failed.");
  }

  Serial.println("========================================");
}

// ========== RECEIVE AND RELAY ==========
void receiveMessage() {
  int packetSize = LoRa.parsePacket();

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

  // Ignore messages originally created by Node B.
  if (origin == NODE_ID) {
    return;
  }

  // Do not process or relay a message more than once.
  if (alreadySeen(origin, messageID)) {
    Serial.println("Duplicate message ignored.");
    return;
  }

  rememberMessage(origin, messageID);

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
  Serial.println(messageID);

  Serial.print("Details: ");
  Serial.println(payload);

  Serial.print("Signal strength: ");
  Serial.print(rssi);
  Serial.println(" dBm");

  setStatusLED(payload);

  // Forward the original message with a reduced TTL.
  if (ttl > 1) {

    String forwarded = String(NETWORK_ID) + "|" +
                       messageID + "|" +
                       origin + "|" +
                       String(ttl - 1) + "|" +
                       payload;

    // Brief random delay reduces the chance of simultaneous relays.
    delay(random(50, 200));

    Serial.println("Forwarding alert to nearby nodes...");

    LoRa.beginPacket();
    LoRa.print(forwarded);

    int result = LoRa.endPacket();

    if (result == 1) {
      Serial.println("Alert forwarding transmission completed.");
    } else {
      Serial.println("ERROR: Alert forwarding failed.");
    }

  } else {
    Serial.println("Message hop limit reached; not forwarding.");
  }

  Serial.println("======================================");
}

// ========== SOS BUTTON ==========
void checkButton() {
  bool currentState = digitalRead(SOS_BUTTON);

  if (previousButtonState == HIGH &&
      currentState == LOW &&
      millis() - lastPressTime > 300) {

    lastPressTime = millis();

    Serial.println("MANUAL SOS BUTTON PRESSED");

    sendMessage("SOS:Manual emergency button");
  }

  previousButtonState = currentState;
}

// ========== SETUP ==========
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
  Serial.println("        RESCUEMESH - NODE B");
  Serial.println("========================================");

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("ERROR: LoRa initialization failed.");
    Serial.println("Check LoRa wiring and power.");
    loraReady = false;
    return;
  }

  // Match the radio settings used by Node A.
  LoRa.setSyncWord(0x12);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  // Phase 1: CRC error checking.
  LoRa.enableCrc();

  loraReady = true;

  Serial.println("LoRa initialized successfully.");
  Serial.println("Packet CRC enabled.");
  Serial.println("Node B is ready.");
  Serial.println("Press the button to send SOS.");
  Serial.println("========================================");
}

// ========== MAIN LOOP ==========
void loop() {
  if (!loraReady) {
    delay(100);
    return;
  }

  checkButton();
  receiveMessage();

  // Return to idle green after the event indication.
  if (ledActive && millis() - ledTime >= 5000) {
    allLEDsOff();
    digitalWrite(GREEN_LED, HIGH);
    ledActive = false;
  }
}

