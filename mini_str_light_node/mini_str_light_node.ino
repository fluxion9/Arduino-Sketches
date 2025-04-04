#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// #define nodeId "node1"
#define nodeId "node2"

const char *ssid = "xbee.net";
const char *password = "xbee-2025";

char ser_data[100];

void trimWhiteSpace(char *str) {
  if (str == NULL) {
    return;
  }
  int len = strlen(str);
  int start = 0;
  int end = len - 1;
  while (isspace(str[start]) && start < len) {
    start++;
  }
  while (end >= start && isspace(str[end])) {
    end--;
  }
  int shift = 0;
  for (int i = start; i <= end; i++) {
    str[shift] = str[i];
    shift++;
  }
  str[shift] = '\0';
}

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
  if (Serial.available()) {
    ser_data[0] = '\0';
    while (Serial.available()) {
      delay(3);
      char m = Serial.read();
      byte len = strlen(ser_data);
      if (m != '\n' || m != '\r') {
        ser_data[len] = m;
        ser_data[len + 1] = '\0';
      }
    }
    if (strlen(ser_data) > 0) {
      trimWhiteSpace(ser_data);
      ping_server(ser_data, strlen(ser_data));
      ser_data[0] = '\0';
    }
  }
}

int ping_server(const char data[], const int len) {
  HTTPClient http;
  WiFiClient client;
  char url[90];
  // Serial.print("Data to ping: ");
  // Serial.println(data);
  sprintf(url, "http://192.168.4.1/ping?name=%s&val=%s", nodeId, data);
  http.begin(client, url);
  int httpCode = http.GET();
  if(httpCode == 200)
  {
    Serial.println(http.getString());
  }
  http.end();
  return httpCode;
}
