#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define buzzer 0

const char *ssid = "mnet.net";
const char *password = "mnet-2023";

String sta_num = "0";

int error = 0;

AsyncWebServer server(80);

String users_phone_number = "+234...";

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
    <title>Mnet Server SPA</title>
</head>
<body>
    <div class="container">
        <h1>Mnet Server</h1>
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
    pinMode(buzzer, 1);
    pinMode(LED_BUILTIN, OUTPUT);
    WiFi.softAP(ssid, password, 1, 0, 8);
    IPAddress IP = WiFi.softAPIP();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });

    server.on("/sta-count", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/plain", sta_num.c_str()); });
              
    server.on("/ping!", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                error = 1;
                request->send(200); 
                });

    server.begin();
}

void loop()
{
    statusLed.Update();
    sta_num = String(WiFi.softAPgetStationNum());
    if(error > 0)
    {
        digitalWrite(buzzer, 1);
        sendSMS(users_phone_number);
        error = 0;
        digitalWrite(buzzer, 0);
    }
}

void sendSMS(String phoneNumber)
  {
    Serial.println("AT");
    delay(1000);
    Serial.println("AT+CMGF=1");
    delay(1000);
    Serial.println("AT+CMGS=\"" + phoneNumber + "\"");
    delay(1000);
    Serial.print("Hi Boss, I'm sorry to say this but just so you know, its about to go down! :-)");
    delay(1000);
    Serial.write(26);
    delay(1000);
  }
