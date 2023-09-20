#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "index.h"

const char *ssid = "Grid-Tied Inverter";
const char *password = "GTISM-2023";
AsyncWebServer server(80);


String data_buffer = "", ser_buf = "", input = "";
unsigned long last_millis = 0;

class Blinker
{
    int ledPin;
    long onTime;
    long offTime;

    int ledState;
    unsigned long previousMillis;
    unsigned long currentMillis;

public:
    Blinker(int pin, long on, long off)
    {
        ledPin = pin;
        pinMode(ledPin, OUTPUT);

        onTime = on;
        offTime = off;
        ledState = LOW;
        previousMillis = 0;
    }
    void Update()
    {
        currentMillis = millis();
        if ((ledState == HIGH) && (currentMillis - previousMillis >= onTime))
        {
            ledState = LOW;
            previousMillis = currentMillis;
            digitalWrite(ledPin, ledState);
        }
        else if ((ledState == LOW) && (currentMillis - previousMillis >= offTime))
        {
            ledState = HIGH;
            previousMillis = currentMillis;
            digitalWrite(ledPin, ledState);
        }
    }
};

Blinker statusLed(LED_BUILTIN, 5000, 300);

void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();

    data_buffer.reserve(64);
    ser_buf.reserve(32);
    input.reserve(20);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });

    server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    Serial.println("+on");
    request->send(200); });

    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    Serial.println("+off");
    request->send(200); });

    server.on("/clamp", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    Serial.println("+cla");
    request->send(200); });

    server.on("/uclamp", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    Serial.println("+ucla");
    request->send(200); });

    server.on("/get-data", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/plain", data_buffer.c_str()); });
    server.begin();
}

void loop()
{
    statusLed.Update();
    if (Serial.available())
    {
        while (Serial.available() > 0)
        {
            delay(3);
            char c = Serial.read();
            ser_buf += c;
        }
    }
    if (ser_buf.length() > 0)
    {
        ser_buf.trim();
        data_buffer = ser_buf;
        ser_buf = "";
    }
}