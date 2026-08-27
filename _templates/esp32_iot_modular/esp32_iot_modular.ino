// ==============================================================================
//  Template: ESP32 Modular IoT Sketch
//  Target: ESP32 Dev Module (esp32:esp32:esp32da or esp32:esp32:esp32)
// ==============================================================================

#include "config.h"
#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #include "secrets.example.h"
#endif
#include "wifi_helper.h"

WiFiHelper wifiHelper(WIFI_SSID, WIFI_PASSWORD);
unsigned long lastTelemetryTime = 0;

void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(STATUS_LED_PIN, OUTPUT);

  Serial.println(F("\n========================================"));
  Serial.println(F("     ESP32 Modular IoT Node Starting    "));
  Serial.println(F("========================================"));

  wifiHelper.begin();
}

void loop() {
  // 1. Maintain WiFi Connection non-blocking
  wifiHelper.loop();

  // 2. Periodic Telemetry Task
  unsigned long now = millis();
  if (now - lastTelemetryTime >= TELEMETRY_INTERVAL) {
    lastTelemetryTime = now;

    if (wifiHelper.isConnected()) {
      digitalWrite(STATUS_LED_PIN, HIGH);
      Serial.print(F("[IoT Telemetry] IP: "));
      Serial.print(wifiHelper.getIP());
      Serial.println(F(" -> Status: OK"));
      digitalWrite(STATUS_LED_PIN, LOW);
    } else {
      Serial.println(F("[IoT Telemetry] Waiting for WiFi connection..."));
    }
  }
}
