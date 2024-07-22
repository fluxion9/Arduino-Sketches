
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>


const char* ssid = "OGSSC-01";      //change this to your wifi username
const char* password = "OG-2024*";  //change this to your wifi password

char data[90];

void setupWiFI() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  WaitConnectWiFi();
  // Serial.println(WiFi.localIP());
}

void WaitConnectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
  digitalWrite(LED_BUILTIN, HIGH);
}


void setup() {
  Serial.begin(9600);

  setupWiFI();

  data[0] = '\0';
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WaitConnectWiFi();
  }
  if (Serial.available()) {
    data[0] = '\0';
    while (Serial.available()) {
      delay(3);
      char m = Serial.read();
      byte len = strlen(data);
      data[len] = m;
      data[len + 1] = '\0';
    }
    blink(2);
    dump(data);
  }
}

void blink(int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(150);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
  }
}

void dump(const char inp[]) {
  // Serial.println(inp);
  WiFiClient espclient;
  HTTPClient http;
  char url[120];
  sprintf(url, "http://meters.onegridenergies.com/update-meter");
  // Serial.println(url);
  http.begin(espclient, url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(inp);
  String payload = http.getString();
  payload.trim();
  Serial.println(payload);
  http.end();
}
