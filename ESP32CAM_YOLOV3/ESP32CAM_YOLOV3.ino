#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <esp32cam.h>

#define interval 30

#define A1 14
#define A5 15

#define PIR 13

const char* ssid = "YOLO32-CAM";
const char* pswd = "YOLOV3-2024";

AsyncWebServer server(80);


static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);

unsigned long diff = 0, lastMillis = 0;

bool dark = false;

void setup() {
  Serial.begin(9600);
  pinMode(A1, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(PIR, INPUT);
  selectBrightness(2);
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);
    bool ok = Camera.begin(cfg);
    //Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }
  WiFi.softAP(ssid, pswd);
  IPAddress IP = WiFi.softAPIP();
  Serial.println();
  Serial.println(IP);

  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest* request) {
    auto frame = esp32cam::capture();
    if (frame == nullptr) {
      request->send(503);
    }
    else {
      AsyncWebServerResponse* response = request->beginResponse_P(200, "image/jpeg", frame->data(), frame->size());
      request->send(response);
    }
  });

  server.on("/person", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("Person(s) Detected!");
    lastMillis = millis();
    request->send(200);
  });

  server.on("/dark=1", HTTP_GET, [](AsyncWebServerRequest* request) {
    dark = true;
    Serial.println("Frame is Dark.");
    request->send(200);
  });

  server.on("/dark=0", HTTP_GET, [](AsyncWebServerRequest* request) {
    dark = false;
    request->send(200);
  });

  server.begin();
}

void selectBrightness(int val)
{
  switch (val)
  {
    case 0:
      digitalWrite(A1, LOW);
      digitalWrite(A5, LOW);
      break;
    case 1:
      digitalWrite(A1, HIGH);
      digitalWrite(A5, LOW);
      break;
    case 2:
      digitalWrite(A1, LOW);
      digitalWrite(A5, HIGH);
      break;
    case 3:
      digitalWrite(A1, HIGH);
      digitalWrite(A5, HIGH);
      break;
  }
}

void loop() {
  diff = millis() - lastMillis;
  if(diff >= interval * 1000UL)
  {
    if(diff <= interval*2000UL)
    {
      selectBrightness(1);
    }
    else {
      selectBrightness(0);
    }
  }
  else {
    selectBrightness(2);
  }
  if(dark && digitalRead(PIR))
  {
    lastMillis = millis();
  }
}