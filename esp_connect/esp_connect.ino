#include <Arduino.h>
String data, payload;
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "Iphone"; // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "chiefbuydata";  // The password of the Wi-Fi network

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
    if (Serial.available())
      {
        char d = Serial.read();
        if (d == '?')
        {
          Serial.println("[wl_disconnected]");
        }
      }
  }
  digitalWrite(LED_BUILTIN, 0);
  data.reserve(400);
  payload.reserve(64);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED)
  {
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED_BUILTIN, 1);
      delay(150);
      digitalWrite(LED_BUILTIN, 0);
      delay(150);
      if (Serial.available())
      {
        char d = Serial.read();
        if (d == '?')
        {
          Serial.println("[wl_disconnected]");
        }
      }
    }
  }
  if (Serial.available())
  {
    data = "";
    while (Serial.available())
    {
      delay(3);
      char m = Serial.read();
      data += m;
    }
    data.trim();
    if (data == "?")
    {
      Serial.println("[wl_connected]");
    }
    else {
      dump(data);
    }
  }
}


  void dump(String msg)
  {
    HTTPClient http;
    WiFiClient client;
    String url = "http://api.thingspeak.com/update?";
    url.concat(msg);
    http.begin(client, url);
    int httpCode = http.GET();
    payload = http.getString();
    payload.trim();
    Serial.println(payload);
    http.end();
  }
