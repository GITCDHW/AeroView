#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

// ====== CONFIG ======
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASS "your_wifi_password"

#define DHTPIN 4          // GPIO where your DHT22 data pin is connected
#define DHTTYPE DHT22     // Sensor type
#define DEVICE_ID "envBot_01"

#define ENDPOINT_URL "https://your-vercel-endpoint.vercel.app/api/logData"  // Change this!

// ====== SETUP ======
DHT dht(DHTPIN, DHTTYPE);
unsigned long lastPost = 0;
const unsigned long postInterval = 10000; // 10 seconds between readings

void setup() {
  Serial.begin(115200);
  dht.begin();

  Serial.println("\n[WiFi] Connecting...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[WiFi] Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ====== LOOP ======
void loop() {
  if (millis() - lastPost >= postInterval) {
    lastPost = millis();

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    // Validate readings
    if (isnan(temp) || isnan(hum)) {
      Serial.println("[DHT] Failed to read sensor data!");
      return;
    }

    Serial.printf("[DHT] Temp: %.2f °C | Hum: %.2f %%\n", temp, hum);

    // Send to backend
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(ENDPOINT_URL);
      http.addHeader("Content-Type", "application/json");

      String payload = "{\"machineID\":\"" + String(DEVICE_ID) + 
                       "\",\"data\":{\"temp\":" + String(temp, 2) + 
                       ",\"hum\":" + String(hum, 2) + "}}";

      int code = http.POST(payload);

      if (code > 0) {
        Serial.printf("[HTTP] POST %d\n", code);
        String resp = http.getString();
        Serial.println(resp);
      } else {
        Serial.printf("[HTTP] Failed, error: %s\n", http.errorToString(code).c_str());
      }

      http.end();
    } else {
      Serial.println("[WiFi] Disconnected, retrying...");
      WiFi.reconnect();
    }
  }
}