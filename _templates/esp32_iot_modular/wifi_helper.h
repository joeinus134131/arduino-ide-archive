#ifndef WIFI_HELPER_H
#define WIFI_HELPER_H

#include <WiFi.h>
#include "config.h"

class WiFiHelper {
private:
  const char* ssid;
  const char* password;
  unsigned long lastCheckTime;

public:
  WiFiHelper(const char* ssid, const char* password)
    : ssid(ssid), password(password), lastCheckTime(0) {}

  void begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print(F("Connecting to WiFi: "));
    Serial.println(ssid);
  }

  bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
  }

  void loop() {
    unsigned long now = millis();
    if (now - lastCheckTime >= WIFI_CHECK_INTERVAL) {
      lastCheckTime = now;
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("[WiFi] Reconnecting..."));
        WiFi.disconnect();
        WiFi.reconnect();
      }
    }
  }

  String getIP() {
    return WiFi.localIP().toString();
  }
};

#endif // WIFI_HELPER_H
