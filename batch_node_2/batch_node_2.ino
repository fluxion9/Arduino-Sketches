#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char *ssid = "dbnet.net";
const char *password = "dbnet-2024";

char ser_data[65];

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
  ser_data[0] = '\0';
}

void loop() {
  if (WiFi.status() != WL_CONNECTED)
  {
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED_BUILTIN, 1);
      delay(150);
      digitalWrite(LED_BUILTIN, 0);
      delay(150);
    }
  }
  if (Serial.available()) {
    ser_data[0] = '\0';
    while (Serial.available()) {
      delay(3);
      char m = Serial.read();
      byte len = strlen(ser_data);
      ser_data[len] = m;
      ser_data[len + 1] = '\0';
    }
    ping_server(ser_data);
  }
}

int ping_server(const char val[])
{
  HTTPClient http;
  WiFiClient client;
  char url[90];
  sprintf(url, "http://192.168.4.1/ping?name=%s&val=%s", "dhtc", val);
  http.begin(client, url);
  int httpCode = http.GET();
  http.end();
  return httpCode;
}
