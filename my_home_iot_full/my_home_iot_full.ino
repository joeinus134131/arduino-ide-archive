#include "DHT.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// --- Pengaturan Pin (ESP8266 NodeMCU Pin Aliases) ---
#define DHTPIN D4     // Pin D4 (GPIO 2). Ganti ke D2 jika kamu colok ke pin fisik D2!
#define GAS_PIN A0    // A0 (Analog) untuk Sensor Gas
#define PIR_PIN D6    // Pin D6 (GPIO 12) untuk Sensor PIR
#define BUZZER_PIN D5 // Pin D5 (GPIO 14) untuk Buzzer

#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- Batas Kritis (Threshold) ---
// Sesuaikan nilai ini sesuai hasil kalibrasimu
const float BATAS_SUHU_MAX = 35.0;       // Dalam Celcius
const float BATAS_KELEMBAPAN_MAX = 80.0; // Dalam Persen
const int BATAS_GAS = 400;               // Nilai analog ESP8266 (0-1023)

// ================= KONFIGURASI IOT & WIFI =================
const char *ssid = "Infinix NOTE 40 Pro 5G";
const char *password = "oktest123";
const char *serverUrl = "https://iot.nexflux.io/api/v1/iot/push";
const char *iotToken =
    "dvc_452451a5-d216-49e3-bb4f-a1161f5d6b3c"; // Ganti dengan token device
                                                // NexFlux kamu

// ================= VARIABEL GLOBAL IOT =================
unsigned long lastIotSend = 0;
const unsigned long iotInterval = 5000; // Kirim data tiap 5 detik

// Variabel global untuk menyimpan nilai sensor terakhir (untuk dikirim ke IoT)
float lastTemp = 0;
float lastHum = 0;
int lastGasLevel = 0;
bool lastPirState = false;

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n--- Sistem Pemantauan Lingkungan Rumah Aktif ---"));

  // Set Mode Pin
  pinMode(DHTPIN, INPUT_PULLUP);
  pinMode(GAS_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Pastikan buzzer mati saat booting
  digitalWrite(BUZZER_PIN, LOW);

  // Koneksi WiFi
  connectWiFi();

  // Inisialisasi Sensor DHT
  dht.begin();

  // Waktu kalibrasi untuk PIR dan pemanasan Sensor Gas
  Serial.print(F("Kalibrasi Sensor PIR & Gas... (tunggu 20 detik)"));
  for (int i = 0; i < 20; i++) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(F("Selesai! Sistem siap."));
}

void loop() {
  // 1. Baca Sensor Digital Respons Cepat (PIR)
  // Dibaca setiap iterasi loop agar tidak ketinggalan gerakan sekecil apapun
  bool adaGerakan = (digitalRead(PIR_PIN) == HIGH);
  lastPirState = adaGerakan;

  bool kondisiBahaya = false;
  String logPeringatan = "";

  // Jika PIR mendeteksi gerakan, langsung tandai bahaya
  if (adaGerakan) {
    kondisiBahaya = true;
    logPeringatan += "[ADA GERAKAN] ";
  }

  // 2. Baca Sensor Cuaca & Gas (Dengan Delay Non-Blocking)
  // Kita gunakan millis() agar pembacaan DHT/Gas tiap 2 detik tidak membuat PIR
  // nge-lag
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 3000) {
    lastUpdate = millis();

    // Baca Suhu & Kelembapan (force read agar membaca langsung dari pin)
    float h = dht.readHumidity(true);
    float t = dht.readTemperature(false, true);

    // Baca level Gas
    int gasLevel = analogRead(GAS_PIN);

    // Simpan ke variabel global untuk IoT
    if (!isnan(h) && !isnan(t)) {
      lastTemp = t;
      lastHum = h;
    }
    lastGasLevel = gasLevel;

    // Evaluasi Bahaya (Suhu & Gas)
    if (!isnan(h) && !isnan(t)) {
      if (t > BATAS_SUHU_MAX) {
        kondisiBahaya = true;
        logPeringatan += "[SUHU PANAS] ";
      }
      if (h > BATAS_KELEMBAPAN_MAX) {
        kondisiBahaya = true;
        logPeringatan += "[KELEMBAPAN TINGGI] ";
      }
    }

    if (gasLevel > BATAS_GAS) {
      kondisiBahaya = true;
      logPeringatan += "[AWAS GAS/ASAP] ";
    }

    // --- LOGGING KE SERIAL MONITOR ---
    Serial.print(F("Suhu: "));
    Serial.print(t);
    Serial.print(F("°C | "));
    Serial.print(F("Kelembapan: "));
    Serial.print(h);
    Serial.print(F("% | "));
    Serial.print(F("Gas: "));
    Serial.print(gasLevel);
    Serial.print(F(" | "));

    if (adaGerakan) {
      Serial.println("PIR: GERAKAN! >> " + logPeringatan);
    } else if (kondisiBahaya) {
      Serial.println("Status: >> " + logPeringatan);
    } else {
      Serial.println("Status: AMAN");
    }
  }

  // 3. Eksekusi Alarm Buzzer Terpusat
  if (kondisiBahaya) {
    // Pola bunyi cepat
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  } else {
    // Pastikan mati jika kondisi normal
    digitalWrite(BUZZER_PIN, LOW);
  }

  // 4. Kirim Data ke NexFlux IoT Platform Secara Berkala
  if (millis() - lastIotSend > iotInterval) {
    sendDataToIoT(lastTemp, lastHum, lastPirState, lastGasLevel);
    lastIotSend = millis();
  }
}

// ================= FUNGSI IOT & WIFI =================
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  int attempts = 0;
  // Coba konek maksimal 20 kali (sekitar 10 detik)
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Failed! Berjalan di mode offline.");
  }
}

void sendDataToIoT(float temperature, float humidity, bool pirState,
                   int gasLevel) {
  if (WiFi.status() != WL_CONNECTED) {
    // Coba reconnect jika terputus
    Serial.println("WiFi terputus, mencoba reconnect...");
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED)
      return;
  }

  WiFiClientSecure client;
  client.setInsecure(); // Skip SSL certificate verification (untuk development)

  HTTPClient http;
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-IoT-Token", iotToken);

  // Mapping Virtual Pin sesuai NexFlux Dashboard:
  // V0 = Temperature (double, 0-100 °C)
  // V1 = Humidity    (double, 0-100 %)
  // V2 = PIR Sensor  (double, 0-1)
  // V3 = MQ 2 Sensor (double, 0-1000)
  String payload = "{\"V0\":" + String(temperature, 2) +
                   ",\"V1\":" + String(humidity, 2) +
                   ",\"V2\":" + String(pirState ? 1 : 0) +
                   ",\"V3\":" + String(gasLevel) + "}";

  Serial.print("Mengirim ke NexFlux: ");
  Serial.println(payload);

  int httpCode = http.POST(payload);
  if (httpCode > 0) {
    Serial.printf("HTTP Response: %d\n", httpCode);
    if (httpCode == 200) {
      Serial.println("Data berhasil terkirim!");
    }
  } else {
    Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}