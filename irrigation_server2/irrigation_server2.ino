#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "IoT Irrigation";
const char *password = "I2S-2023";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
 <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IoT Irrigation System</title>
    <style>
        body {
            text-align: center;
            font-family: Arial, sans-serif;
            background-color: #f2f2f2;
            margin: 0;
        }

        main {
            position: absolute;
            top: 0;
            right: 0;
            bottom: 0;
            left: 0;
            padding: 0 20%;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }

        h1 {
            color: #c46b18;
            margin-bottom: 1rem;
        }

        .params-container {
            display: flex;
            width: 100%;
            gap: 1rem;
            margin-block: 1rem;
            margin-bottom: 2rem;
            justify-content: center;
        }

        .params {
            border: 1px solid #ccc;
            padding: 1rem;
            border-radius: 6px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: space-between;
            gap: 1rem;
            background-color: #ffffff;
            box-shadow: 0px 2px 6px rgba(0, 0, 0, 0.1);
            transition: transform 0.2s ease-in-out;
        }

        .params:hover {
            transform: translateY(-5px);
        }

        .param-value {
            width: 100%;
            padding: 0.5rem;
            font-weight: 600;
            text-align: center;
            border: none;
            background-color: #ffffff;
            outline: none;
            color: #333;
        }

        @media only screen and (max-width: 780px) {
            main {
                padding: 0;
                padding: 1rem;
            }
            .param {
                width: 100%;
            }
        }
    </style>
</head>
<body>
    <main>
        <h1>IoT-Based Irrigation System</h1>
        <div class="params-container">
            <div class="params">
                Flow Rate (cm^3/S)
                <input
                    title="Flow Rate"
                    disabled
                    id="rate"
                    class="param-value"
                    value="0.00"
                />
            </div>
            <div class="params">
                Battery Voltage (V)
                <input
                    title="Battery Voltage"
                    disabled
                    id="vbatt"
                    class="param-value"
                    value="0.00"
                />
            </div>
            <div class="params">
                Volume (cm^3)
                <input
                    title="Volume"
                    disabled
                    id="volume"
                    class="param-value"
                    value="0.00"
                />
            </div>
            <div class="params">
                Water-Level
                <input
                    title="Water Level"
                    disabled
                    id="wlevel"
                    class="param-value"
                    value="LOW"
                />
            </div>
        </div>
    </main>

    <script lang="text/javascript">
        let flo = document.getElementById('rate');
        let vba = document.getElementById('vbatt');
        let vol = document.getElementById('volume');
        let lvl = document.getElementById('wlevel');

        function getPayLoad() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    let payLoad = JSON.parse(this.responseText);
                    flo.setAttribute('value', payLoad.flo);
                    vba.flosetAttribute('value', payLoad.vba);
                    vol.setAttribute('value', payLoad.vol);
                    lvl.setAttribute('value', payLoad.lvl);
                }
            };
            xhttp.open("GET", "/get-data", true);
            xhttp.send();
        }
        getPayLoad();
        setInterval(getPayLoad, 1500);
    </script>
</body>
</html>
)rawliteral";


String data_buffer = "", ser_buf = "";
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

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });
              
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
