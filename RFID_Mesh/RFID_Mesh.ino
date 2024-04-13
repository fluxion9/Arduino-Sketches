#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "RFID.net";
const char* password = "RFID-2023";

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, 1);
  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, 1);
    delay(150);
    digitalWrite(LED_BUILTIN, 0);
    delay(150);
  }
  digitalWrite(LED_BUILTIN, 0);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED_BUILTIN, 1);
      delay(150);
      digitalWrite(LED_BUILTIN, 0);
      delay(150);
    }
  }
  if (Serial.available()) {
    char data[65];
    data[0] = '\0';
    while (Serial.available()) {
      delay(3);
      char m = Serial.read();
      byte len = strlen(data);
      data[len] = m;
      data[len + 1] = '\0';
    }
    flash();
    ping_server(data);
  }
}

int ping_server(const char input[]) {
  HTTPClient http;
  WiFiClient client;
  char url[90];
  sprintf(url, "http://192.168.4.1/ping?id=%s", input);
  http.begin(client, url);
  int httpCode = http.GET();
  if(httpCode = 200)
  {
    String payload = http.getString();
    Serial.println(payload);
  }
  http.end();
  return httpCode;
}

void flash() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_BUILTIN, 1);
    delay(100);
    digitalWrite(LED_BUILTIN, 0);
    delay(100);
  }
}
