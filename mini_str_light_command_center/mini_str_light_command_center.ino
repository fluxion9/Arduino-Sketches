#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "xbee.net";
const char *password = "xbee-2025";

char sta_num[3] = "0";

char node1_data[32] = "{}", node2_data[32] = "{}", ser_data[33], payload[200];
bool nodeStates[2] = { 1, 1 };

unsigned long lastSendTime = 0;

AsyncWebServer server(80);


void handlePing(const char name[], const int n_len, const char value[], const int v_len) {
  if (strcmp(name, "node1") == 0) {
    node1_data[0] = '\0';
    strcpy(node1_data, value);
  } else if (strcmp(name, "node2") == 0) {
    node2_data[0] = '\0';
    strcpy(node2_data, value);
  }
}

void loadjson() {
  payload[0] = '\0';
  sprintf(payload, "{\"node1\":%s,\"node2\":%s}", node1_data, node2_data);
}

void setup() {
  Serial.begin(9600);
  pinMode(0, 0);
  WiFi.softAP(ssid, password, 1, 0, 8);
  IPAddress IP = WiFi.softAPIP();

  payload[0] = '\0';

  ser_data[0] = '\0';

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200);
  });

  server.on("/sta-count", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", sta_num);
  });

  server.on("/get-all", HTTP_GET, [](AsyncWebServerRequest *request) {
    loadjson();
    request->send_P(200, "text/plain", payload);
  });

  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
    String name = request->getParam(0)->value();
    String value = request->getParam(1)->value();
    char n_buffer[65], v_buffer[65];
    strcpy(n_buffer, name.c_str());
    strcpy(v_buffer, value.c_str());
    handlePing(n_buffer, strlen(n_buffer), v_buffer, strlen(v_buffer));
    if (name == "node1") {
      request->send_P(200, "text/plain", String(nodeStates[0]).c_str());
    } else if (name == "node2") {
      request->send_P(200, "text/plain", String(nodeStates[1]).c_str());
    }
  });

  server.begin();
}

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

void loop() {
  sprintf(sta_num, "%d", WiFi.softAPgetStationNum());
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
      // Serial.println(ser_data);
      if (strcmp(ser_data, "atv1") == 0) {
        // Serial.println("OK");
        nodeStates[0] = 1;
      } else if (strcmp(ser_data, "dtv1") == 0) {
        // Serial.println("OK");
        nodeStates[0] = 0;
      } else if (strcmp(ser_data, "atv2") == 0) {
        // Serial.println("OK");
        nodeStates[1] = 1;
      } else if (strcmp(ser_data, "dtv2") == 0) {
        // Serial.println("OK");
        nodeStates[1] = 0;
      } else {
        ser_data[0] = '\0';
      }
    }
  }
  if (millis() - lastSendTime >= 1000) {
    loadjson();
    Serial.println(payload);
    lastSendTime = millis();
  }
}
