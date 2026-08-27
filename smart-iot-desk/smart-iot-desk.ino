#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ================= KONFIGURASI HARDWARE =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
#define SDA_PIN 8
#define SCL_PIN 9
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define TOUCH_PIN 3
#define BUZZER_PIN 4

// ================= KONFIGURASI IOT & WIFI =================
const char* ssid = "Infinix NOTE 40 Pro 5G";
const char* password = "oktest123";
const char* serverUrl = "https://iot.nexflux.io/api/v1/iot/push";
const char* iotToken = "dvc_fddfc7e0-e7f0-4f0b-beb3-1a8f629b66dc";

// ================= VARIABEL GLOBAL =================
float temp = 0;
float hum = 0;

// Timer untuk IoT
unsigned long lastIotSend = 0;
const unsigned long iotInterval = 5000;

// Variabel untuk Deteksi Multi-Tap
int lastTouchState = LOW;
int tapCount = 0;
unsigned long lastTapTime = 0;
const unsigned long tapTimeout = 400;

// Variabel Kontrol Emosi
unsigned long emotionTimer = 0;
bool isShowingEmotion = false;
const unsigned long emotionDuration = 3000;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  
  pinMode(TOUCH_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  dht.begin();

  // Inisialisasi OLED
  Wire.begin(SDA_PIN, SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED gagal diakses!"));
    for(;;);
  }

  // Animasi Booting
  drawHappyFace();
  tone(BUZZER_PIN, 800, 150);
  delay(200);
  tone(BUZZER_PIN, 1200, 200);
  delay(2000);

  // Sapaan Baru: "Welcome, Made!"
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2); // Font lebih besar
  display.setCursor(15, 15);
  display.print("Welcome,");
  display.setCursor(35, 35);
  display.print("Made!");
  display.display();
  
  // Connect WiFi sembari sapaan tampil
  connectWiFi();
  delay(2000);

  // Baca suhu awal
  temp = dht.readTemperature();
  hum = dht.readHumidity();
}

// ================= LOOP UTAMA =================
void loop() {
  int currentTouchState = digitalRead(TOUCH_PIN);

  // Logika Deteksi Sentuhan
  if (currentTouchState == HIGH && lastTouchState == LOW) {
    tapCount++;
    lastTapTime = millis();
  }
  lastTouchState = currentTouchState;

  // Evaluasi Sentuhan
  if (tapCount > 0 && (millis() - lastTapTime > tapTimeout)) {
    isShowingEmotion = true;
    emotionTimer = millis(); 

    if (tapCount == 1) {
      drawHappyFace();
      playHappySound();
    } 
    else if (tapCount == 2) {
      drawSurprisedFace();
      playSurprisedSound();
    } 
    else if (tapCount >= 3) {
      drawSleepyFace();
      playSleepySound();
    }
    tapCount = 0; 
  }

  // Kembali ke Mode Normal jika durasi emosi habis
  if (isShowingEmotion && (millis() - emotionTimer > emotionDuration)) {
    isShowingEmotion = false;
  }

  // Update Layar Normal (Suhu Full Screen)
  if (!isShowingEmotion) {
    drawNormalState(temp, hum);
  }

  // Kirim Data IoT Secara Berkala
  if (millis() - lastIotSend > iotInterval) {
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();
    
    if (!isnan(newTemp) && !isnan(newHum)) {
      temp = newTemp;
      hum = newHum;
      Serial.println("-----------------------");
      Serial.printf("Temperature : %.2f °C\n", temp);
      Serial.printf("Humidity    : %.2f %%\n", hum);
      
      sendData(temp, hum);
    }
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
  } else {
    Serial.println("\nWiFi Failed to Connect! Berjalan di mode offline.");
  }
}

void sendData(float temperature, float humidity) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-IoT-Token", iotToken);

  String payload = "{\"V0\":" + String(temperature, 2) + ",\"V1\":" + String(humidity, 2) + "}";
  
  int httpCode = http.POST(payload);
  if (httpCode > 0) {
    Serial.printf("HTTP Response: %d\n", httpCode);
  }
  http.end();
}

// ================= FUNGSI SUARA =================
void playHappySound() {
  tone(BUZZER_PIN, 1000, 100); delay(100); tone(BUZZER_PIN, 1500, 150);
}
void playSurprisedSound() {
  tone(BUZZER_PIN, 2000, 50); delay(60); tone(BUZZER_PIN, 2500, 50); delay(60); tone(BUZZER_PIN, 2000, 50);
}
void playSleepySound() {
  tone(BUZZER_PIN, 800, 200); delay(200); tone(BUZZER_PIN, 600, 300);
}

// ================= FUNGSI GRAFIS UI =================
void drawNormalState(float t, float h) {
  display.clearDisplay();
  
  // Teks Label kecil
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Temperature:");
  
  display.setCursor(0, 32);
  display.print("Humidity:");

  // Indikator WiFi kecil di pojok kanan atas
  if(WiFi.status() == WL_CONNECTED) {
    display.setCursor(114, 0); 
    display.print("Wi");
  }

  // Nilai Data (Font Lebih Besar)
  display.setTextSize(2);
  
  // Posisi Suhu
  display.setCursor(0, 12); 
  display.print(t, 1); 
  display.print(" C");

  // Posisi Kelembapan
  display.setCursor(0, 44); 
  display.print(h, 1); 
  display.print(" %");

  display.display();
}

void drawHappyFace() {
  display.clearDisplay();
  display.fillCircle(40, 30, 12, SSD1306_WHITE);
  display.fillCircle(88, 30, 12, SSD1306_WHITE);
  display.fillRect(25, 30, 30, 15, SSD1306_BLACK); 
  display.fillRect(73, 30, 30, 15, SSD1306_BLACK); 
  display.fillCircle(64, 45, 6, SSD1306_WHITE);
  display.fillRect(55, 35, 20, 10, SSD1306_BLACK);
  display.display();
}

void drawSurprisedFace() {
  display.clearDisplay();
  display.fillCircle(40, 30, 10, SSD1306_WHITE); 
  display.fillCircle(88, 30, 10, SSD1306_WHITE);
  display.drawCircle(64, 48, 5, SSD1306_WHITE);  
  display.display();
}

void drawSleepyFace() {
  display.clearDisplay();
  display.drawLine(28, 30, 52, 30, SSD1306_WHITE); 
  display.drawLine(76, 30, 100, 30, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(60, 40);
  display.print("Zzz");
  display.display();
}