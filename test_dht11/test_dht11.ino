#include "DHT.h"

// Pin DHT11 di ESP8266 NodeMCU
#define DHTPIN D4     // Pin fisik D4 (GPIO 2)
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println(F("\n=========================================="));
  Serial.println(F("       TESTING SENSOR DHT11 DI PIN D4     "));
  Serial.println(F("=========================================="));
  
  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();
  
  Serial.println(F("Menunggu 2 detik untuk stabilisasi sensor..."));
  delay(2000);
}

void loop() {
  // DHT11 membutuhkan jeda minimal 2 detik antar pembacaan
  delay(2000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("[GAGAL] Tidak dapat membaca dari sensor DHT11!"));
    Serial.println(F("  -> Cek kabel VCC (ke 3.3V atau 5V)"));
    Serial.println(F("  -> Cek kabel GND"));
    Serial.println(F("  -> Cek kabel Data (ke D4)"));
  } else {
    Serial.print(F("[SUKSES] Suhu: "));
    Serial.print(t);
    Serial.print(F(" °C  |  Kelembapan: "));
    Serial.print(h);
    Serial.println(F(" %"));
  }
}
