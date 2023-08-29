#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "Smart IoT Socket";
const char *password = "SIS-2023";
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Iot Socket</title>
    <style>
      body {
        text-align: center;
        font-family: "Courier New", Courier, monospace;
      }
      * {
        box-sizing: border-box;
      }
      a {
        text-decoration: none;
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
      .timer {
        position: absolute;
        top: 1rem;
        left: 1rem;
        font-weight: 700;
        font-family: Arial, sans-serif;
        color: #000;
      }
      .limit {
        position: absolute;
        top: 1rem;
        right: 1rem;
        font-weight: 700;
        font-family: Arial, sans-serif;
        color: #000;
      }
      .container {
        width: 100%;
        display: flex;
        justify-content: center;
        margin: 0;
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
        border: 1px solid #000;
        padding: 0.5rem;
        border-radius: 6px;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: space-between;
        gap: 0.5rem;
      }
      .param-value {
        width: 100%;
        padding: 0.5rem;
        font-weight: 600;
        text-align: center;
      }
      .off {
        color: green;
      }
      .on {
        color: rgb(226, 50, 19);
      }
      .btn {
        width: 150px;
        margin: 7px;
        padding: 10px;
        background-color: #000;
        aspect-ratio: 1;
        border-radius: 100px;
        border: none;
        font-size: 20px;
        font-weight: 600;
        isolation: isolate;
        position: relative;
        z-index: 10;
      }
      .btn::before {
        content: "";
        background-color: #000;
        position: absolute;
        inset: 0;
        z-index: -5;
        border-radius: 100px;
      }
      .btn::after {
        content: "";
        position: absolute;
        inset: 0;
        border-radius: 100px;
        background-color: #000;
        transform: translate(-50%, -50%);
        z-index: -10;
        transform: scale(0);
        transition: all 250ms ease-in-out;
        animation: pulse 750ms linear 250ms infinite;
      }
      .energy {
        margin: 0;
        padding: 1rem 1.5rem;
        border-radius: 6px;
        color: #e2e2e2;
        background-color: #000;
      }
      #input-form {
        margin-top: 1rem;
        display: flex;
        gap: 1rem;
        flex-wrap: wrap;
        width: 100%;
        justify-content: center;
        align-items: center;
      }
      .input-group {
        display: flex;
        align-items: center;
        gap: 1rem;
      }
      #emax {
        height: 3rem;
        flex: 1;
        min-width: 10rem;
        padding-inline: 0.5rem;
        border-radius: 6px;
        border: 1px solid #000;
        background-color: #e2e2e2;
      }
      #submitBtn {
        background-color: #000;
        color: #fff;
        border: none;
        padding-inline: 2rem;
        border-radius: 6px;
        height: 3rem;
      }

      @keyframes pulse {
        0% {
          transform: scale(0.9);
          opacity: 1;
        }
        100% {
          transform: scale(1.3);
          opacity: 0;
        }
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
      @media only screen and (width < 426px) {
        #submitBtn {
          flex: 1;
        }
      }
      body {
        background-color: #e2e2e2;
      }
    </style>
  </head>
  <body>
    <main>
      <a href="http://192.168.4.1/timer""><span class="timer">TIMER</span></a>
      <h1>Smart IoT Socket Dashboard</h1>
      <p class="energy">Energy Consumed: <span id="enrg">0.00</span> kwhr</p>
      <form id="input-form">
        <div class="input-group">
          <label>Set Limit:</label>
          <input type="number" step="0.01" placeholder="Energy Limit Value" id="emax" min="0.0" value="10.0"/>
        </div>
        <button type="submit" id="submitBtn">Submit</button>
      </form>
      <div class="params-container">
        <div class="params">
          Voltage (V)
          <input title="voltage" disabled id="voltage" class="param-value" value="0.00"/>
        </div>
        <div class="params">
          Current (A)
          <input title="current" disabled id="current" class="param-value" value="0.00"/>
        </div>
        <div class="params">
          Power (W)
          <input title="power" disabled id="power" class="param-value" value="0.00"/>
        </div>
      </div>
      <div class="container">
        <button onclick="activate()" id="on-off" class="btn off">
          ACTIVATE
        </button>
      </div>
    </main>
    <script lang="text/javascript">
      let v = document.getElementById('voltage');
      let i = document.getElementById('current');
      let p = document.getElementById('power');
      let e = document.getElementById('enrg');
      document.getElementById("submitBtn").addEventListener("click", function (event) {
          event.preventDefault();
          var elimit = document.getElementById("emax").value;
          var url = "http://192.168.4.1/set-limit/?elimit=" + elimit;
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          xhttp.open("GET", url, true);
          xhttp.send();
        });
      let pwr = document.getElementById('on-off');
      function activate() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
        }
        };
        xhttp.open("GET", "/on-off", true);
        xhttp.send();
        }
      function flipSwitch(btn, state)  {
            if (state && btn.classList.contains('off')) {
              btn.classList.remove('off')
              btn.classList.add('on')
              btn.innerText = "DEACTIVATE"
              } else if (!state && btn.classList.contains('on')) {
                btn.classList.remove('on')
                btn.classList.add('off')
                btn.innerText = "ACTIVATE"
            }
      }
      function getPayLoad() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                let payLoad = JSON.parse(this.responseText);
                flipSwitch(pwr, payLoad.ps);
                e.innerText = payLoad.enrg;
                v.setAttribute('value', payLoad.volt);
                i.setAttribute('value', payLoad.curr);
                p.setAttribute('value', payLoad.powr);
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
)=====";

const char timer_html[] PROGMEM = R"=====(
)=====";

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
    
    server.on("/timer", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", timer_html); });

    server.on("/on-off", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    Serial.println("+on-off");
    request->send(200); });

    server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    Serial.println("+on");
    request->send(200); });

    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    Serial.println("+off");
    request->send(200); });
    server.on("/set-limit", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    input = "";
    input.concat("[");
    input.concat("eLim,");
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
