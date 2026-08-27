#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- KONFIGURASI OLED ---
#define SCREEN_WIDTH 128 // Lebar OLED, dalam piksel
#define SCREEN_HEIGHT 64 // Tinggi OLED, dalam piksel

// Deklarasi untuk SSD1306 display terhubung ke I2C (SDA, SCL pins)
// Address 0x3C biasanya untuk 128x64, 0x3D untuk 128x32
#define OLED_RESET     -1 // Reset pin # (atau -1 jika berbagi pin reset Arduino)
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- KONFIGURASI DHT ---
#define DHTPIN 4     // Pin digital ESP32 yang terhubung ke sensor DHT
// Pilih tipe sensor yang Anda gunakan (uncomment salah satu)
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // Inisialisasi DHT
  dht.begin();

  // Inisialisasi OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Loop selamanya jika OLED gagal
  }

  // Bersihkan buffer layar
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  // Tampilkan pesan pembuka
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("Inisialisasi...");
  display.display();
  delay(2000);
}

void loop() {
  // Tunggu beberapa detik antar pengukuran (DHT sensor lambat)
  delay(2000);

  // Baca kelembaban
  float h = dht.readHumidity();
  // Baca suhu dalam Celsius
  float t = dht.readTemperature();

  // Cek jika pembacaan gagal
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Gagal membaca sensor DHT!"));
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println(F("Sensor Error!"));
    display.display();
    return;
  }

  // Tampilkan di Serial Monitor (untuk debugging)
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));

  // --- TAMPILKAN DI OLED ---
  display.clearDisplay();

  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Monitoring Suhu");

  // Garis pemisah
  display.drawLine(0, 10, 128, 10, WHITE);

  // Tampilkan Suhu
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Suhu: ");
  display.setTextSize(2); // Font lebih besar untuk angka
  display.setCursor(0, 32);
  display.print(t);
  display.setTextSize(1);
  display.print(" C");

  // Tampilkan Kelembaban
  display.setTextSize(1);
  display.setCursor(70, 20);
  display.print("Lembab: ");
  display.setTextSize(2); // Font lebih besar untuk angka
  display.setCursor(70, 32);
  display.print((int)h); // Cast ke int biar hemat tempat
  display.setTextSize(1);
  display.print(" %");

  display.display(); // Perintah wajib untuk update layar
}