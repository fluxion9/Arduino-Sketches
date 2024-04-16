#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char *ssid = "dbnet.net";
const char *password = "dbnet-2024";

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, 1);
  pinMode(0, 0);
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
  if (digitalRead(0)) {
    while (digitalRead(0)) {
      ping_server(1);
      delay(1000);
    }
    ping_server(0);
  }
}

int ping_server(int val) {
  HTTPClient http;
  WiFiClient client;
  char url[90];
  sprintf(url, "http://192.168.4.1/ping?name=%s&val=%d", "pirs", val);
  http.begin(client, url);
  int httpCode = http.GET();
  http.end();
  return httpCode;
}
