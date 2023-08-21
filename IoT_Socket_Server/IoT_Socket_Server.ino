#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "Smart IoT Socket";
const char *password = "SIS-2023";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
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
        font-family: 'Courier New', Courier, monospace;
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

      .container {
        width: 100%;
        display: flex;
        justify-content: center;
        margin: 0;
      }

      .params {
        width: 33.33%;
        height: 150px;
        margin: 20px;
        display: flex;
        background-color: rgb(101, 185, 62);
        flex-direction: column;
        justify-content: center;
        align-items: center;
        box-shadow: 1px 1px 0px;
        border: 1px solid black;
        border-radius: 5%;
      }

      .param-value {
        width: 80px;
        text-align: center;
        margin-top: 60px;
        border: 1px solid;
        border-radius: 40px;
        font-weight: bolder;
        background-color: white;
        font-size: larger;
      }

      .off {
        background-color: green;
      }

      .on {
        background-color: rgb(226, 50, 19);
      }


      .btn {
        width: 150px;
        margin: 7px;
        padding: 10px;
        color: white;
        border-radius: 40px;
      }

      .energy {
        margin: 0;
        padding: 10px;
        border-radius: 40px;
        color: white;
        background-color: rgb(199, 103, 25);
      }

      @media only screen and (max-width: 780px) {
        main {
          padding: 0;
        }
        .param {
          width: 100%;
        }
      }

      body {
        background-color: rgb(51, 92, 182);
      }
    </style>
  </head>
  <body>
    <main>
      <h1>Smart IoT Socket Dashboard</h1>
      <p class="energy">Energy Consumed: <span id="enrg">0.00</span> kwhr</p>
      <div class="container">
        <div class="params">
          Voltage (V)
          <input disabled id="voltage" class="param-value" value="0.00" />
        </div>
        <div class="params">
          Current (A)
          <input disabled id="current" class="param-value" value="0.00" />
        </div>
        <div class="params">
          Power (W)
          <input disabled id="power" class="param-value" value="0.00" />
        </div>
      </div>
      <div class="container">
        <button onclick="activate()" id="on-off" class="btn off">
          POWER
        </button>
      </div>
    </main>

    <script lang="text/javascript">
      let v = document.getElementById('voltage');
      let i = document.getElementById('current');
      let p = document.getElementById('power');
      let e = document.getElementById('enrg');

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
            } else if (!state && btn.classList.contains('on')) {
                btn.classList.remove('on')
                btn.classList.add('off')
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
)rawliteral";

const char set_limit_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>set limit</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
    }
    header {
      background-color: #333;
      color: #fff;
      text-align: center;
      padding: 1rem;
    }
    h2 {
      color: goldenrod;
    }
    .container {
      max-width: 600px;
      margin: 0 auto;
      padding: 2rem;
    }
    .input-group {
      margin-bottom: 1rem;
    }
    label {
      display: block;
      margin-bottom: 0.5rem;
    }
    input[type="number"], select {
      width: 100%;
      padding: 0.5rem;
      border: 1px solid #ccc;
    }
    .increment-decrement {
      display: flex;
      align-items: center;
    }
    .increment-decrement button {
      background-color: #333;
      color: #fff;
      border: none;
      padding: 0.2rem 0.5rem;
      cursor: pointer;
    }
    button[type="submit"] {
      background-color: #333;
      color: #fff;
      border: none;
      padding: 0.5rem 1rem;
      cursor: pointer;
    }
  </style>
</head>
<body>
  <header>
    <h1>Set Current Consumption Limit</h1>
    <h2>
      Current Limit:
      <span><span id="lim">0.00</span>A</span>
    </h2>
  </header>
  <div class="container">
    <form id="input-formm">
      <div class="input-group">
        <label>Set Limit:</label>
        <input type="number" step="0.01" placeholder="Current Value" id="imax" min="0.0" max="30.0" value="10.0">
      </div>
      <button type="submit" id="submitBtn">Submit</button>
    </form>
  </div>
  <script>
  let limm = document.getElementById("lim");
  document.getElementById("submitBtn").addEventListener("click", function(event) {
      event.preventDefault();
      var ilimit = document.getElementById("imax").value;
      var url = "http://192.168.4.1/set-limit/?iLimit=" + ilimit;
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
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
      limm.innerText = payLoad.limt;
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
              
    server.on("/set-current-limit", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", set_limit_html); });

    server.on("/on-off", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    Serial.println("+on-off");
    request->send(200); });

    server.on("/set-limit", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    input = "";
    input.concat("[");
    input.concat("iLim,");
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
