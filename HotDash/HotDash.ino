#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "Smart IoT Socket";
const char *password = "SIS-2023";

char sta_ssid[32] = "OGSSC-01";
char sta_pswd[32] = "OG-2024*";

unsigned long t_stablink = 0, t_reconnect = 0;

bool s_stablink = false;

char inp[90];

AsyncWebServer server(80);


void reconnect() {
  if (millis() - t_reconnect >= 2000) {
    // Serial.print("Reconnect ");
    // Serial.print(sta_ssid);
    // Serial.print(" ");
    // Serial.println(sta_pswd);
    // WiFi.begin(sta_ssid, sta_pswd);
    t_reconnect = millis();
  }
}

void stablink() {
  int w_stat = WiFi.status();
  if (w_stat == WL_CONNECTED) {
    if (millis() - t_stablink >= 500) {
      if (s_stablink) {
        digitalWrite(LED_BUILTIN, LOW);
        s_stablink = !s_stablink;
      } else {
        digitalWrite(LED_BUILTIN, HIGH);
        s_stablink = !s_stablink;
      }
      t_stablink = millis();
    }
  } else {
    if (millis() - t_stablink >= 100) {
      if (s_stablink) {
        digitalWrite(LED_BUILTIN, LOW);
        s_stablink = !s_stablink;
      } else {
        digitalWrite(LED_BUILTIN, HIGH);
        s_stablink = !s_stablink;
      }
      t_stablink = millis();
    }
    reconnect();
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP(ssid, password);
  // Serial.println(WiFi.softAPIP());

  delay(2000);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", "Hello!");
  });

  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request) {
    sta_ssid[0] = '\0';
    strcpy(sta_ssid, request->getParam(0)->value().c_str());
    sta_pswd[0] = '\0';
    strcpy(sta_pswd, request->getParam(1)->value().c_str());
    Serial.print("Set Connect: ");
    Serial.print(sta_ssid);
    Serial.print(" ");
    Serial.println(sta_pswd);
    WiFi.begin(sta_ssid, sta_pswd);
    request->send(200);
  });

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (WiFi.status() == WL_CONNECTED) {
      inp[0] = '\0';
      strcat(inp, "{");
      strcat(inp, "\"conn\":1,");
      strcat(inp, "\"rssi\":");
      char temp[16];
      itoa(WiFi.RSSI(), temp, 10);
      strcat(inp, temp);
      strcat(inp, ",\"ip\":");
      strcat(inp, WiFi.localIP().toString().c_str());
      strcat(inp, "}");
      Serial.println(inp);
      request->send_P(200, "text/plain", inp);
    } else {
      request->send_P(200, "text/plain", "{\"conn\":0}");
    }
  });

  server.begin();

  Serial.print("Connect: ");
  Serial.print(sta_ssid);
  Serial.print(" ");
  Serial.println(sta_pswd);

  WiFi.begin(sta_ssid, sta_pswd);
}

void loop() {
  stablink();
}
