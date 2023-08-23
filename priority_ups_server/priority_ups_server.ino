#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "Smart Change-Over Switch";
const char *password = "SCOS-2023";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background-color: black;
            color: white;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
        }

        .nav-bar {
            background-color: goldenrod;
            width: 100%;
            padding: 40px 0;
            text-align: center;
        }

        .nav-bar h1 {
            margin: 0;
            color: black;
        }

        .dashboard {
            display: flex;
            flex-direction: column;
            align-items: center;
            padding: 20px;
            background-color: rgba(0, 0, 0, 0.7);
            border-radius: 10px;
            width: 80%;
            max-width: 700px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.3);
            margin-top: 20px;
        }

        .lamp-container {
            display: flex;
            justify-content: center;
            flex-wrap: wrap;
            margin-top: 20px;
        }

        .lamp {
            display: flex;
            flex-direction: column;
            align-items: center;
            margin: 10px;
        }

        .lamp-circle {
            width: 30px;
            height: 30px;
            background-color: grey;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            transition: background-color 0.3s;
        }

        .lamp-btn {
            background-color: transparent;
            border: 2px solid goldenrod;
            color: goldenrod;
            padding: 10px 20px;
            cursor: pointer;
            transition: background-color 0.3s, color 0.3s;
            margin-top: 10px;
        }

        .lamp-btn:hover {
            background-color: goldenrod;
            color: black;
        }

        #priority-select {
            background-color: black;
            color: white;
            border: none;
            padding: 5px;
            margin-top: 10px;
        }

            #disabled-btn {
            background-color: purple;
            border: none;
            color: black;
            width: 100px;
            height: 100px;
            font-size: 14px;
            border-radius: 50%;
            cursor: not-allowed;
            transition: transform 0.5s, color 0.3s;
            position: absolute;
            top: 70px;
            left: 50%;
            transform: translateX(-50%);
            animation: pulse 1.5s infinite;
        }

        @keyframes pulse {
            0% {
                transform: translateX(-50%) scale(1);
            }
            50% {
                transform: translateX(-50%) scale(1.1);
            }
            100% {
                transform: translateX(-50%) scale(1);
            }
        }
    </style>
    <title>Automatic Changeover System</title>
</head>
<body>
    <div class="nav-bar">
        <h1>Automatic Changeover System</h1>
    </div>
    <div class="dashboard">
        <div class="lamp-container">
            <div class="lamp">
                <div class="lamp-circle" id="lamp1"></div>
                <button class="lamp-btn">On/Off</button>
            </div>
            <div class="lamp">
                <div class="lamp-circle" id="lamp2"></div>
                <button class="lamp-btn">On/Off</button>
            </div>
            <div class="lamp">
                <div class="lamp-circle" id="lamp3"></div>
                <button class="lamp-btn">On/Off</button>
            </div>
            <div class="lamp">
                <div class="lamp-circle" id="lamp4"></div>
                <button class="lamp-btn">On/Off</button>
            </div>
            <div class="lamp">
                <div class="lamp-circle" id="lamp5"></div>
                <button class="lamp-btn">On/Off</button>
            </div>
        </div>
        <label for="priority-select">Set Priority:</label>
        <select id="priority-select">
            <option value="1">Lamp 1</option>
            <option value="2">Lamp 2</option>
            <option value="3">Lamp 3</option>
            <option value="4">Lamp 4</option>
            <option value="5">Lamp 5</option>
        </select>
    </div>
    <button id="disabled-btn" class="disabled">AC</button>
    <script>
        const lampCircles = document.querySelectorAll('.lamp-circle');
        const lampButtons = document.querySelectorAll('.lamp-btn');
        const prioritySelect = document.getElementById('priority-select');
        const disabledButton = document.getElementById('disabled-btn');

        lampButtons.forEach((button, index) => {
            button.addEventListener('click', () => {
                var url = "http://192.168.4.1/on-off/?id=" + (index+1);
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function () {
                    if (this.readyState == 4 && this.status == 200) {
                    }
                };
                xhttp.open("GET", url, true);
                xhttp.send();
            });
        });

        prioritySelect.addEventListener('change', (event) => {
            var id = event.target.value
            var url = "http://192.168.4.1/set-priority/?id=" + (id);
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                }
            };
            xhttp.open("GET", url, true);
            xhttp.send();
        });

        function getPayLoad() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                let payLoad = JSON.parse(this.responseText);
                lampCircles[0].style.backgroundColor = changeColor(payLoad.l1);
                lampCircles[1].style.backgroundColor = changeColor(payLoad.l2);
                lampCircles[2].style.backgroundColor = changeColor(payLoad.l3);
                lampCircles[3].style.backgroundColor = changeColor(payLoad.l4);
                lampCircles[4].style.backgroundColor = changeColor(payLoad.l5);
                disabledButton.style.backgroundColor = changeColor(payLoad.gatv);
            }
          };
          xhttp.open("GET", "/get-data", true);
          xhttp.send();
        }

        function changeColor(state) {
            if(state)
            {
                return "goldenrod";
            }else {
                return "grey";
            }
        }
        getPayLoad();
        setInterval(getPayLoad, 1500);
    </script>
</body>
</html>
)rawliteral";


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

    server.on("/on-off", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    input = "";
    input.concat("[");
    input.concat("onoff,");
    input.concat(request->getParam(0)->value());
    input.concat("]");
    Serial.println(input);
    request->send(200); });

    server.on("/set-priority", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    input = "";
    input.concat("[");
    input.concat("spr,");
    input.concat(request->getParam(0)->value());
    input.concat("]");
    Serial.println(input);
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
