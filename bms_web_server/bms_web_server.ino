// {"v1":14.5,"v2":15.0,"v3":16.0,"ps":1,"bs":0}
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid     = "SwitchBMS";
const char* password = "SwitchBMS";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Document</title>

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

      .battery {
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
      }

      .battery-value {
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

      .charge-on {
        background-color: rgb(226, 147, 19);
      }

      .btn {
        width: 150px;
        margin: 7px;
        padding: 10px;
        color: white;
        border-radius: 40px;
      }

      .volt {
        margin: 0;
        padding: 10px;
        border-radius: 40px;
        color: white;
        background-color: rgb(199, 103, 25);
      }

      .charging {
        position: absolute;
        top: 40px; right: 20px;
        padding: 10px;
        border-radius: 50%;
      }

      @media only screen and (max-width: 780px) {
        main {
          padding: 0;
        }
        .battery {
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
      <span id="chargingStatus" class="charging"></span>
      <h1>Switch BMS</h1>
      <p class="volt">Battery Voltage: <span id="volt">50.1</span>V</p>
      <div class="container">
        <div class="battery">
          Battery 1
          <input disabled id="battery-1" class="battery-value" value="0.01" />
        </div>
        <div class="battery">
          Battery 2
          <input disabled id="battery-2" class="battery-value" value="0.01" />
        </div>
        <div class="battery">
          Battery 3
          <input disabled id="battery-3" class="battery-value" value="0.01" />
        </div>
      </div>
      <div class="container">
        <button onclick="start('Inverter')" id="power" class="btn off">
          POWER
        </button>
        <button onclick="start('Balancer')" id="balance" class="btn off">
          BALANCE
        </button>
      </div>
    </main>

    <script lang="text/javascript">
      let battery_1 = document.getElementById('battery-1');
      let battery_2 = document.getElementById('battery-2');
      let battery_3 = document.getElementById('battery-3');

      let battery_0 = document.getElementById('volt');

      let chargingStatus = document.getElementById('chargingStatus');

      let b = document.getElementById('balance');
      let p = document.getElementById('power');

      function start(Event) {
        if (Event == 'Inverter')
        {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
        };
          xhttp.open("GET", "/invert", true);
          xhttp.send();
        }else if (Event == 'Balancer') {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
        };
          xhttp.open("GET", "/balance", true);
          xhttp.send();
        }
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

      function checkChargingStatus(status) {
        if(status) {
          chargingStatus.classList.remove('off');
          chargingStatus.classList.add('charge-on');
        } else {
          chargingStatus.classList.remove('charge-on');
          chargingStatus.classList.add('off');
        }
      }

      function getPayLoad() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                let payLoad = JSON.parse(this.responseText);
                checkChargingStatus(payLoad.cs);
                flipSwitch(b, payLoad.bs);
                flipSwitch(p, payLoad.ps);
                battery_0.innerText = payLoad.vt;
                battery_1.setAttribute('value', payLoad.v1);
                battery_2.setAttribute('value', payLoad.v2);
                battery_3.setAttribute('value', payLoad.v3);
            }
          };
          xhttp.open("GET", "/get-payloads", true);
          xhttp.send();
        }
        getPayLoad();
        setInterval(getPayLoad, 3000);
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

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  // Serial.print("AP IP address: ");
  // Serial.println(IP);

  data_buffer.reserve(64);
  ser_buf.reserve(32);
  input.reserve(20);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
    });

   server.on("/balance", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+balanceBat");
    request->send(200);
    });

    server.on("/invert", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+power");
    request->send(200);
    });

    server.on("/lock/23568", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("!lock");
    request->send(200);
    });

    server.on("/unlock/45892", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("!unlock");
    request->send(200);
    });

    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("!reset");
    request->send(200);
    });

    server.on("/set-params", HTTP_GET, [](AsyncWebServerRequest * request) {
    input = "";
    input.concat("[");
    input.concat(request->getParam(0)->value());
    input.concat(",");
    input.concat(request->getParam(1)->value());
    input.concat(",");
    input.concat(request->getParam(2)->value());
    input.concat(",");
    input.concat(request->getParam(3)->value());
    input.concat(",");
    input.concat(request->getParam(4)->value());
    input.concat("]");
    Serial.println(input);
    request->send(200);
    });

    server.on("/get-payloads", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", data_buffer.c_str());
    });
    
   server.begin();
}

void loop() {
  statusLed.Update();
  if(millis() - last_millis >= 2000)
  {
    Serial.println("+read;");
    last_millis = millis();
  }
  while(Serial.available() > 0)
  {
    delay(3);
    char c = Serial.read();
    ser_buf += c;
  }
  if(ser_buf.length() > 0)
  {
    ser_buf.trim();
    data_buffer = ser_buf;
    ser_buf = "";
  }  
}
