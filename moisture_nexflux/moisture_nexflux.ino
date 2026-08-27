#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHTPIN 4          // Pin DATA DHT11
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Infinix NOTE 40 Pro 5G";
const char* password = "oktest123";
const char* serverUrl = "https://iot.nexflux.io/api/v1/iot/push";

// Use your Device Token from the Token & API tab
const char* iotToken = "dvc_fddfc7e0-e7f0-4f0b-beb3-1a8f629b66dc";

void connectWiFi() {
  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void sendData(float temperature, float humidity) {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected");
    return;
  }

  HTTPClient http;

  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-IoT-Token", iotToken);

  String payload =
      "{"
      "\"V0\":" + String(temperature, 2) + ","
      "\"V1\":" + String(humidity, 2) +
      "}";

  Serial.println(payload);

  int httpCode = http.POST(payload);

  Serial.print("HTTP Response: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    Serial.println(http.getString());
  }

  http.end();
}

void setup() {

  Serial.begin(115200);

  dht.begin();

  connectWiFi();
}

void loop() {

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Celsius

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read DHT11!");
    delay(2000);
    return;
  }

  Serial.println("-----------------------");
  Serial.print("Temperature : ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity    : ");
  Serial.print(humidity);
  Serial.println(" %");

  sendData(temperature, humidity);

  delay(5000);
}