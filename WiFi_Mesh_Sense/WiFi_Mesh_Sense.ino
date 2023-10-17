#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "mnet.net";
const char* password = "mnet-2023";

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
  if (WiFi.status() != WL_CONNECTED)
  {
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED_BUILTIN, 1);
      delay(150);
      digitalWrite(LED_BUILTIN, 0);
      delay(150);
    }
  }
  if(digitalRead(0))
  {
    flash();
    ping_server();
    delay(500);
  }
}

int ping_server()
{
  HTTPClient http;
  WiFiClient client;
  http.begin(client, "http://192.168.4.1/ping!");
  int httpCode = http.GET();
  http.end();
  return httpCode;
}

void flash()
{
    for (int i = 0; i < 2; i++)
    {
        digitalWrite(LED_BUILTIN, 1);
        delay(100);
        digitalWrite(LED_BUILTIN, 0);
        delay(100);
    }
}
