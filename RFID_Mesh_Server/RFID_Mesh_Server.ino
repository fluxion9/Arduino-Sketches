#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "EEPROM.h"

#define EEPROM_SIZE 1024

// #define buzzer 2

const char *ssid = "RFID.net";
const char *password = "RFID-2023";

char sta_num[3] = "0";

int error = 0;

const char master[] = "6a026380";
bool addCard = false;

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }

        .container {
            text-align: center;
        }
    </style>
    <title>RFID Server SPA</title>
</head>
<body>
    <div class="container">
        <h1>RFID Server</h1>
        <p>No. of Connected Devices: <span id="connectedDevices">0</span></p>
    </div>

    <script>
        function updateConnectedDevices() {
            const connectedDevicesElement = document.getElementById('connectedDevices');
             var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    connectedDevicesElement.textContent = this.responseText;
                }
            };
            xhttp.open("GET", "/sta-count", true);
            xhttp.send();
        }
        updateConnectedDevices();
        setInterval(updateConnectedDevices, 1500);
    </script>
</body>
</html>
)rawliteral";

unsigned long last_millis = 0;

class Blinker {
  int ledPin;
  long onTime;
  long offTime;

  int ledState;
  unsigned long previousMillis;
  unsigned long currentMillis;

public:
  Blinker(int pin, long on, long off) {
    ledPin = pin;
    pinMode(ledPin, OUTPUT);
    onTime = on;
    offTime = off;
    ledState = LOW;
    previousMillis = 0;
  }
  void Update() {
    currentMillis = millis();
    if ((ledState == HIGH) && (currentMillis - previousMillis >= onTime)) {
      ledState = LOW;
      previousMillis = currentMillis;
      digitalWrite(ledPin, ledState);
    } else if ((ledState == LOW) && (currentMillis - previousMillis >= offTime)) {
      ledState = HIGH;
      previousMillis = currentMillis;
      digitalWrite(ledPin, ledState);
    }
  }
};

Blinker statusLed(0, 5000, 300);

int handleCard(char input[], int size) {
  if (strcmp(input, master) == 0 && !addCard) {
    addCard = true;
    return 2;
  } else if (strcmp(input, master) == 0 && addCard) {
    addCard = false;
    return 3;
  }
  int readpos = 1;
  char card[9];
  uint8_t records = EEPROM.read(0);
  if (records == 0) {
    if (addCard) {
      EEPROM.write(1, size);
      for (int i = 0; i < size; i++) {
        EEPROM.write(i + 2, input[i]);
      }
      EEPROM.write(0, 1);
      EEPROM.commit();
      return 4;
    } else {
      return 0;
    }
  } else {
    for (int i = 0; i < records; i++) {
      int readlen = EEPROM.read(readpos);
      for (int j = 0; j < readlen; j++) {
        card[j] = EEPROM.read(readpos + j + 1);
      }
      card[8] = '\0';
      if (strcmp(card, input) == 0) {
        return 1;
      } else {
        if (addCard) {
          //add card
          int nextwrite = (9 * records) + 1;
          EEPROM.write(0, records + 1);
          EEPROM.write(nextwrite, size);
          nextwrite++;
          for (int j = 0; j < size; j++) {
            EEPROM.write(nextwrite++, (uint8_t)input[j]);
          }
          EEPROM.commit();
          Serial.println();
          return 4;
        }
      }
      readpos += 9;
    }
    return 0;
  }
}

void setup() {
  pinMode(0, OUTPUT);
  Serial.begin(9600);

  EEPROM.begin(EEPROM_SIZE);

  delay(2000);

  WiFi.softAP(ssid, password, 1, 0, 8);
  digitalWrite(0, HIGH);

  IPAddress IP = WiFi.softAPIP();

  int records = EEPROM.read(0);
  Serial.print("Found ");
  Serial.print(records);
  Serial.println(" records in EEPROM.");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/sta-count", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", sta_num);
  });

  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
    String id = request->getParam(0)->value();
    char buffer[9];
    strcpy(buffer, id.c_str());
    int res = handleCard(buffer, strlen(buffer));
    buffer[0] = '\0';
    itoa(res, buffer, 10);
    Serial.println(id);
    Serial.println(res);
    request->send_P(200, "text/plain", buffer);
  });

  server.begin();
}

void loop() {
  statusLed.Update();
  sprintf(sta_num, "%d", WiFi.softAPgetStationNum());
}
