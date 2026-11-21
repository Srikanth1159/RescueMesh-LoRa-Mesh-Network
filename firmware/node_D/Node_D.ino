
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <WebServer.h>

// ---------- NODE D LORA PINS ----------
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23

// ---------- SOS HARDWARE ----------
#define SOS_BUTTON 27
#define SOS_LED    25

// ---------- LORA SETTINGS ----------
const long LORA_FREQUENCY = 433E6;
const char* NETWORK_ID = "RM_SK_7264";

// ---------- WIFI SETTINGS ----------
const char* AP_SSID = "RescueMesh-D";
const char* AP_PASSWORD = "RescueMesh7264";

WebServer server(80);

bool loraReady = false;
bool previousButtonState = HIGH;

unsigned long lastPressTime = 0;
unsigned long messageNumber = 0;
unsigned long lastDiagnostic = 0;

// ---------- SOS LED ----------
bool sosBlinkActive = false;
bool redLEDState = false;
unsigned long blinkStarted = 0;
unsigned long lastBlinkChange = 0;

const unsigned long SOS_BLINK_DURATION = 5000;
const unsigned long BLINK_INTERVAL = 250;

// ---------- MESSAGE HISTORY ----------
const int MAX_MESSAGES = 30;

struct AlertMessage {
  String id;
  String origin;
  String payload;
  int rssi;
  unsigned long receivedAt;
};

AlertMessage messages[MAX_MESSAGES];
int messageCount = 0;
int nextMessageIndex = 0;

// ---------- DUPLICATE FILTER ----------
String seenMessages[40];
int seenIndex = 0;

String makeMessageKey(String origin, String id) {
  return origin + "|" + id;
}

bool alreadySeen(String origin, String id) {
  String key = makeMessageKey(origin, id);

  for (int i = 0; i < 40; i++) {
    if (seenMessages[i] == key) {
      return true;
    }
  }

  return false;
}

void rememberMessage(String origin, String id) {
  seenMessages[seenIndex] = makeMessageKey(origin, id);
  seenIndex = (seenIndex + 1) % 40;
}

// ---------- JSON ESCAPING ----------
String jsonEscape(const String& input) {
  String output = "";

  for (unsigned int i = 0; i < input.length(); i++) {
    char c = input[i];

    switch (c) {
      case '"':  output += "\\\""; break;
      case '\\': output += "\\\\"; break;
      case '\n': output += "\\n"; break;
      case '\r': output += "\\r"; break;
      case '\t': output += "\\t"; break;

      default:
        if ((uint8_t)c >= 0x20) {
          output += c;
        }
        break;
    }
  }

  return output;
}

// ---------- SAVE MESSAGE ----------
void saveMessage(String id, String origin,
                 String payload, int rssi) {
  messages[nextMessageIndex].id = id;
  messages[nextMessageIndex].origin = origin;
  messages[nextMessageIndex].payload = payload;
  messages[nextMessageIndex].rssi = rssi;
  messages[nextMessageIndex].receivedAt = millis();

  nextMessageIndex = (nextMessageIndex + 1) % MAX_MESSAGES;

  if (messageCount < MAX_MESSAGES) {
    messageCount++;
  }
}

// ---------- SOS LED ----------
void startSOSBlink() {
  sosBlinkActive = true;
  redLEDState = false;
  blinkStarted = millis();
  lastBlinkChange = 0;
  digitalWrite(SOS_LED, LOW);
}

void updateSOSLED() {
  if (!sosBlinkActive) {
    digitalWrite(SOS_LED, LOW);
    return;
  }

  if (millis() - blinkStarted >= SOS_BLINK_DURATION) {
    sosBlinkActive = false;
    redLEDState = false;
    digitalWrite(SOS_LED, LOW);
    return;
  }

  if (lastBlinkChange == 0 ||
      millis() - lastBlinkChange >= BLINK_INTERVAL) {
    lastBlinkChange = millis();
    redLEDState = !redLEDState;

    digitalWrite(SOS_LED, redLEDState ? HIGH : LOW);
  }
}

// ---------- SEND SOS FROM NODE D ----------
void sendSOS() {
  if (!loraReady) {
    Serial.println("ERROR: LoRa is not ready.");
    return;
  }

  messageNumber++;

  String id = "D-" + String(messageNumber);
  String packet = String(NETWORK_ID) + "|" +
                  id + "|D|5|SOS:Manual emergency button";

  Serial.println();
  Serial.println("========== NODE D SOS ==========");
  Serial.print("Packet: ");
  Serial.println(packet);

  startSOSBlink();

  LoRa.beginPacket();
  LoRa.print(packet);

  int result = LoRa.endPacket();

  if (result == 1) {
    Serial.println("SOS transmission completed.");
    Serial.println("Remote receipt is NOT confirmed.");
  } else {
    Serial.println("SOS transmission failed.");
  }

  // Explicitly return the radio to receive mode.
  LoRa.receive();

  Serial.println("Node D returned to receive mode.");
  Serial.println("================================");
}

// ---------- RECEIVE LORA MESSAGES ----------
void receiveMessage() {
  if (!loraReady) {
    return;
  }

  int packetSize = LoRa.parsePacket();

  if (packetSize <= 0) {
    return;
  }

  String packet = "";

  while (LoRa.available()) {
    packet += (char)LoRa.read();
  }

  int rssi = LoRa.packetRssi();

  Serial.println();
  Serial.println("========== RAW LORA PACKET ==========");
  Serial.print("Bytes: ");
  Serial.println(packetSize);
  Serial.print("RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.print("Packet: ");
  Serial.println(packet);
  Serial.println("=====================================");

  int p1 = packet.indexOf('|');

  if (p1 < 0) {
    Serial.println("Invalid packet: missing first separator.");
    return;
  }

  int p2 = packet.indexOf('|', p1 + 1);
  int p3 = packet.indexOf('|', p2 + 1);
  int p4 = packet.indexOf('|', p3 + 1);

  if (p2 < 0 || p3 < 0 || p4 < 0) {
    Serial.println("Invalid packet format.");
    Serial.println("Expected: NETWORK|ID|ORIGIN|TTL|MESSAGE");
    return;
  }

  String network = packet.substring(0, p1);
  String id = packet.substring(p1 + 1, p2);
  String origin = packet.substring(p2 + 1, p3);
  String ttl = packet.substring(p3 + 1, p4);
  String payload = packet.substring(p4 + 1);

  if (network != NETWORK_ID) {
    Serial.println("Packet ignored: different network ID.");
    return;
  }

  // Ignore D's own transmitted packet if it is received back.
  if (origin == "D") {
    Serial.println("Own packet ignored.");
    return;
  }

  if (alreadySeen(origin, id)) {
    Serial.println("Duplicate packet ignored.");
    return;
  }

  rememberMessage(origin, id);
  saveMessage(id, origin, payload, rssi);

  Serial.println();
  Serial.println("======= RESCUEMESH MESSAGE SAVED =======");

  if (payload.indexOf("SOS") >= 0) {
    Serial.println("EMERGENCY SOS RECEIVED!");
    startSOSBlink();
  } else if (payload.indexOf("IMPACT") >= 0 ||
             payload.indexOf("VIBRATION") >= 0) {
    Serial.println("SENSOR ALERT RECEIVED!");
    startSOSBlink();
  } else {
    Serial.println("MESSAGE RECEIVED AND SAVED");
  }

  Serial.print("Original sender: Node ");
  Serial.println(origin);

  Serial.print("Message ID: ");
  Serial.println(id);

  Serial.print("TTL: ");
  Serial.println(ttl);

  Serial.print("Details: ");
  Serial.println(payload);

  Serial.print("RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.println("========================================");
}

// ---------- SOS BUTTON ----------
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

// ---------- DASHBOARD API ----------
void handleMessagesAPI() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  String json = "[";
  int startIndex =
      (nextMessageIndex - messageCount + MAX_MESSAGES)
      % MAX_MESSAGES;

  for (int i = 0; i < messageCount; i++) {
    int index = (startIndex + i) % MAX_MESSAGES;

    if (i > 0) {
      json += ",";
    }

    json += "{\"id\":\"";
    json += jsonEscape(messages[index].id);
    json += "\",\"origin\":\"";
    json += jsonEscape(messages[index].origin);
    json += "\",\"message\":\"";
    json += jsonEscape(messages[index].payload);
    json += "\",\"rssi\":";
    json += String(messages[index].rssi);
    json += ",\"received_ms\":";
    json += String(messages[index].receivedAt);
    json += "}";
  }

  json += "]";
  server.send(200, "application/json", json);
}

void handleStatusAPI() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  String json = "{\"gateway\":\"D\",\"lora_ready\":";
  json += loraReady ? "true" : "false";
  json += ",\"wifi\":\"RescueMesh-D\"";
  json += ",\"ip\":\"192.168.4.1\"";
  json += ",\"message_count\":";
  json += String(messageCount);
  json += "}";

  server.send(200, "application/json", json);
}

// ---------- DASHBOARD PAGE ----------
const char DASHBOARD_PAGE[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>RescueMesh Dashboard</title>
<style>
body {
  font-family: Arial;
  background: #101827;
  color: #f3f4f6;
  padding: 20px;
  margin: 0;
}
main { max-width: 900px; margin: auto; }
.sub { color: #aebbd0; }
.card {
  background: #1d2b40;
  padding: 16px;
  border-radius: 10px;
  margin: 12px 0;
}
table { width: 100%; border-collapse: collapse; }
td, th {
  text-align: left;
  padding: 12px;
  border-bottom: 1px solid #344257;
  word-break: break-word;
}
.alert { color: #ff7777; font-weight: bold; }
</style>
</head>
<body>
<main>
<h1>RescueMesh Dashboard</h1>
<div class="sub">Node D — Emergency Monitoring</div>
<div id="status" class="card">Connecting...</div>
<h2>Received Messages</h2>
<div class="card">
<table>
<thead>
<tr><th>Sender</th><th>Details</th><th>Signal</th></tr>
</thead>
<tbody id="messages"></tbody>
</table>
</div>

<script>
async function refresh() {
  try {
    const sr = await fetch('/api/status');
    const status = await sr.json();

    const mr = await fetch('/api/messages');
    const list = await mr.json();

    document.getElementById('status').textContent =
      'Gateway ' + status.gateway +
      ' | LoRa: ' + (status.lora_ready ? 'Ready' : 'Not ready') +
      ' | Messages: ' + status.message_count;

    const body = document.getElementById('messages');
    body.replaceChildren();

    list.slice().reverse().forEach(m => {
      const row = document.createElement('tr');
      const sender = document.createElement('td');
      const detail = document.createElement('td');
      const signal = document.createElement('td');

      sender.textContent = 'Node ' + m.origin + ' (' + m.id + ')';
      detail.textContent = m.message;
      signal.textContent = m.rssi + ' dBm';

      if (m.message.includes('SOS') ||
          m.message.includes('IMPACT') ||
          m.message.includes('VIBRATION')) {
        detail.className = 'alert';
      }

      row.append(sender, detail, signal);
      body.append(row);
    });
  } catch (e) {
    document.getElementById('status').textContent =
      'Could not connect to Node D.';
  }
}

refresh();
setInterval(refresh, 2000);
</script>
</main>
</body>
</html>
)rawliteral";

void handleDashboard() {
  server.send_P(200, "text/html", DASHBOARD_PAGE);
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(SOS_BUTTON, INPUT_PULLUP);
  pinMode(SOS_LED, OUTPUT);
  digitalWrite(SOS_LED, LOW);

  Serial.println();
  Serial.println("================================");
  Serial.println("       RESCUEMESH NODE D");
  Serial.println("================================");

  // Start Wi-Fi dashboard access point.
  WiFi.mode(WIFI_AP);

  if (WiFi.softAP(AP_SSID, AP_PASSWORD)) {
    Serial.println("Wi-Fi access point started.");
    Serial.print("Wi-Fi name: ");
    Serial.println(AP_SSID);
    Serial.print("Wi-Fi password: ");
    Serial.println(AP_PASSWORD);
    Serial.print("Dashboard address: http://");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("ERROR: Wi-Fi access point failed.");
  }

  // Initialize LoRa SPI and pins.
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (LoRa.begin(LORA_FREQUENCY)) {
    LoRa.setSyncWord(0x12);
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.enableCrc();

    loraReady = true;

    // Start continuous receive mode.
    LoRa.receive();

    Serial.println("LoRa initialized successfully.");
    Serial.println("Frequency: 433 MHz");
    Serial.println("CRC enabled.");
    Serial.println("LoRa receive mode enabled.");
  } else {
    Serial.println("ERROR: LoRa initialization failed.");
    Serial.println("Check wiring and GPIO assignments.");
  }

  server.on("/", HTTP_GET, handleDashboard);
  server.on("/api/messages", HTTP_GET, handleMessagesAPI);
  server.on("/api/status", HTTP_GET, handleStatusAPI);

  server.begin();

  Serial.println("Dashboard server started.");
  Serial.println("Connect to RescueMesh-D Wi-Fi.");
  Serial.println("Open http://192.168.4.1");
  Serial.println("Press Node D button to send SOS.");
  Serial.println("================================");
}

// ---------- MAIN LOOP ----------
void loop() {
  server.handleClient();
  checkSOSButton();
  receiveMessage();
  updateSOSLED();

  // Diagnostic heartbeat: prints once per second,
  // without flooding Serial Monitor.
  if (millis() - lastDiagnostic >= 1000) {
    lastDiagnostic = millis();

    Serial.print("[DIAGNOSTIC] Loop running | LoRa: ");
    Serial.println(loraReady ? "READY" : "NOT READY");
  }
}

