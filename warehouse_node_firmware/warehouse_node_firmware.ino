/*
  Warehouse Stock Locator — Firmware Node ESP32
  ------------------------------------------------
  Fungsi:
  - Subscribe MQTT topic "highlight" -> nyalakan LED WS2812B pada slot tertentu
  - Tombol fisik -> konfirmasi pengambilan barang (ACK) -> publish balik ke
  backend
  - Buzzer sebagai feedback audio saat ACK

  Library yang wajib diinstall (Arduino IDE Library Manager):
  - PubSubClient      (Nick O'Leary)
  - ArduinoJson        (Benoit Blanchon, v6/v7)
  - Adafruit NeoPixel

  Sebelum upload, WAJIB isi:
  - WIFI_SSID, WIFI_PASSWORD
  - MQTT_BROKER, MQTT_PORT
  - WAREHOUSE_ID, RACK_ID  (unik per node!)
  - NUM_LEDS              (sesuai jumlah kolom di rak ini)
*/

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

// ============ KONFIGURASI — WAJIB DIISI PER NODE ============
const char *WIFI_SSID = "Infinix NOTE 40 Pro 5G";
const char *WIFI_PASSWORD = "oktest123";

const char *MQTT_BROKER = "10.157.33.39"; // IP/host broker Mosquitto
const int MQTT_PORT = 1883;

const char *WAREHOUSE_ID = "wh01";
const char *RACK_ID = "rack01"; // GANTI UNIK TIAP NODE, misal rack01..rack10

#define NUM_LEDS 3 // jumlah CHIP WS2811 (bukan LED fisik!)
    // WS2811 12V = 3 LED fisik per chip
    // 12 LED fisik = 4 chip = NUM_LEDS 4
#define LED_DATA_PIN 4
#define BUTTON_PIN 5
#define BUZZER_PIN 18

// ============ OBJEK GLOBAL ============
WiFiClient espClient;
PubSubClient mqttClient(espClient);
Adafruit_NeoPixel strip(NUM_LEDS, LED_DATA_PIN, NEO_RGB + NEO_KHZ800);

String topicHighlight;
String topicAck;
String topicStatus;

// State highlight aktif
bool activeHighlight = false;
int activeSlot = -1;
bool blinkEnabled = false;
uint32_t activeColor = 0;
unsigned long lastBlinkToggle = 0;
bool blinkState = true;
const unsigned long BLINK_INTERVAL_MS = 400;

// Debounce tombol
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_MS = 300;
int lastButtonState = HIGH;

// Heartbeat
unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL_MS = 30000;

// Forward declaration
void beep(int durationMs = 150);

// ============ SETUP ============
void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Self-test Buzzer saat boot: beep singkat 100ms
  beep(100);

  strip.begin();
  strip.setBrightness(255);
  strip.clear();

  // Self-test LED saat boot: nyalakan semua slot terang sebentar untuk tes hardware
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 255));
  }
  strip.show();
  delay(500);
  strip.clear();
  strip.show();

  topicHighlight =
      "wh/" + String(WAREHOUSE_ID) + "/rack/" + String(RACK_ID) + "/highlight";
  topicAck = "wh/" + String(WAREHOUSE_ID) + "/rack/" + String(RACK_ID) + "/ack";
  topicStatus =
      "wh/" + String(WAREHOUSE_ID) + "/rack/" + String(RACK_ID) + "/status";

  // Inisialisasi WiFi bersih dari awal
  WiFi.persistent(false); // jangan simpan config ke flash (hindari konflik)
  WiFi.setAutoReconnect(false); // kita handle reconnect sendiri di loop()
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(200);

  connectWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
}

// ============ LOOP UTAMA ============
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  handleButton();
  handleBlink();
  handleHeartbeat();
}

// ============ WIFI ============
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED)
    return;

  Serial.printf("\nConnecting WiFi to \"%s\"", WIFI_SSID);
  WiFi.disconnect(true); // putus & bersihkan state lama
  delay(100);            // beri waktu driver WiFi reset
  WiFi.mode(WIFI_STA);   // pastikan mode Station
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" connected, IP: " + WiFi.localIP().toString());
  } else {
    Serial.printf(" gagal (status=%d). Retry di loop berikutnya\n",
                  WiFi.status());
  }
}

// ============ MQTT ============
void connectMQTT() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  Serial.print("Connecting MQTT broker...");
  String clientId = "esp32-" + String(RACK_ID);

  if (mqttClient.connect(clientId.c_str())) {
    Serial.println(" connected");
    mqttClient.subscribe(topicHighlight.c_str());
    Serial.println("Subscribed: " + topicHighlight);
  } else {
    Serial.print(" gagal, rc=");
    Serial.println(mqttClient.state());
    delay(2000); // backoff sederhana sebelum retry berikutnya
  }
}

// Callback saat pesan MQTT masuk
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) {
    Serial.println("JSON parse error: " + String(err.c_str()));
    return;
  }

  int slot = doc["slot"] | -1;
  String color = doc["color"] | "green";
  bool blink = doc["blink"] | false;

  if (slot < 0 || slot >= NUM_LEDS) {
    Serial.println("Slot index tidak valid: " + String(slot));
    return;
  }

  color.toLowerCase();

  // Handle perintah mematikan LED
  if (color == "off" || color == "none" || color == "black") {
    strip.setPixelColor(slot, 0);
    strip.show();

    if (activeSlot == slot) {
      activeHighlight = false;
      activeSlot = -1;
      blinkEnabled = false;
    }
    Serial.println("Highlight slot " + String(slot) + " color=off (cleared)");
    return;
  }

  activeSlot = slot;
  activeHighlight = true;
  blinkEnabled = blink;
  blinkState = true;
  activeColor = colorFromName(color);

  strip.clear();
  strip.setPixelColor(activeSlot, activeColor);
  strip.show();

  // Buzzer berbunyi saat ada highlight baru masuk
  beep(200);

  Serial.println("Highlight slot " + String(slot) + " color=" + color);
}

uint32_t colorFromName(String name) {
  if (name == "off" || name == "none" || name == "black")
    return strip.Color(0, 0, 0);
  if (name == "red")
    return strip.Color(255, 0, 0);
  if (name == "blue")
    return strip.Color(0, 0, 255);
  if (name == "amber" || name == "yellow" || name == "orange")
    return strip.Color(255, 140, 0);
  if (name == "white")
    return strip.Color(255, 255, 255);
  return strip.Color(0, 200, 0); // default green
}

// ============ BLINK NON-BLOCKING ============
void handleBlink() {
  if (!activeHighlight || !blinkEnabled)
    return;

  if (millis() - lastBlinkToggle >= BLINK_INTERVAL_MS) {
    lastBlinkToggle = millis();
    blinkState = !blinkState;

    strip.clear();
    if (blinkState) {
      strip.setPixelColor(activeSlot, activeColor);
    }
    strip.show();
  }
}

// ============ TOMBOL ACK ============
void handleButton() {
  int currentButtonState = digitalRead(BUTTON_PIN);

  // Deteksi transisi falling-edge (hanya saat tombol baru DITEKAN dari lepas)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    if (millis() - lastButtonPress >= DEBOUNCE_MS) {
      lastButtonPress = millis();

      if (activeHighlight && activeSlot >= 0) {
        publishAck(activeSlot);
        beep();

        strip.clear();
        strip.show();

        activeHighlight = false;
        activeSlot = -1;
        blinkEnabled = false;
      }
    }
  }
  lastButtonState = currentButtonState;
}

void publishAck(int slot) {
  StaticJsonDocument<128> doc;
  doc["slot"] = slot;
  doc["rack_id"] = RACK_ID;
  doc["ts"] = millis();

  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  mqttClient.publish(topicAck.c_str(), buffer, n);

  Serial.println("ACK published slot " + String(slot));
}

void beep(int durationMs) {
  tone(BUZZER_PIN, 2000, durationMs); // Menghasilkan frekuensi 2kHz untuk buzzer pasif
  digitalWrite(BUZZER_PIN, HIGH);     // Trigger HIGH untuk buzzer aktif
  delay(durationMs);
  digitalWrite(BUZZER_PIN, LOW);
  noTone(BUZZER_PIN);
}

// ============ HEARTBEAT / STATUS (opsional, monitoring kesehatan node)
// ============
void handleHeartbeat() {
  if (millis() - lastHeartbeat < HEARTBEAT_INTERVAL_MS)
    return;
  lastHeartbeat = millis();

  if (!mqttClient.connected())
    return;

  StaticJsonDocument<128> doc;
  doc["online"] = true;
  doc["uptime"] = millis() / 1000;
  doc["rssi"] = WiFi.RSSI();

  char buffer[128];
  size_t n = serializeJson(doc, buffer);
  mqttClient.publish(topicStatus.c_str(), buffer, n);
}
