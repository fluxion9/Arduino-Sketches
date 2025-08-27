#include "DHT.h"
#include "WiFi.h"
#include <ESPAsyncWebServer.h>
#include "myliteral.h"

#define dhtPin 5

#define led 12

#define dhtType DHT22

DHT dht(dhtPin, dhtType);


String data = "";

float temperature = 0.0, humidity = 0.0;

unsigned long lastReadTime = 0;


const char* ssid = "T-Sense";
const char* password = "Tense-2025";

AsyncWebServer server(80);

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

Blinker heartbeatLED(led, 300, 5000);


struct TempSense {
  unsigned long lastSendTime = 0;

  void init() {
    Serial.begin(9600);

    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();

    dht.begin();

    data.reserve(32);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      request->send_P(200, "text/html", index_html);
    });

    server.on("/get-data", HTTP_GET, [](AsyncWebServerRequest* request) {
      request->send_P(200, "text/plain", data.c_str());
    });

    server.begin();
  }

  void measureTemperatureAndHumidity() {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    if (isnan(humidity)) {
      humidity = 0;
    }
    if (isnan(temperature)) {
      temperature = 0;
    }
  }

  void load_data(void) {
    data = "";
    data.concat("{\"temp\":");
    data.concat(temperature);
    data.concat(",\"humd\":");
    data.concat(humidity);
    data.concat("}");
  }

  void run() {
    if (millis() - lastReadTime > 500) {
      measureTemperatureAndHumidity();
      load_data();
      Serial.println(data);
      lastReadTime = millis();
    }
    heartbeatLED.Update();
  }
} tempsense;

void setup() {
  tempsense.init();
}

void loop() {
  tempsense.run();
}
