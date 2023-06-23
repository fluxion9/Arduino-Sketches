// {"v1":14.5,"v2":15.0,"v3":16.0,"ps":1,"bs":0}
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid     = "Smart Dustbin";
const char* password = "MySmartDustbin";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>My Smart Dustbin</title>
    <style>
        body {
            background-color: #f5f5f5;
            font-family: Arial, sans-serif;
            text-align: center;
        }

        h1 {
            margin-top: 50px;
        }

        .battery-icon {
            position: relative;
            width: 200px;
            height: 300px;
            background-color: #f1f1f1;
            border-radius: 10px;
            display: flex;
            flex-direction: column;
            justify-content: flex-end;
            align-items: center;
            margin: 50px auto;
            overflow: hidden;
        }

        .battery-fill {
            width: 100%;
            height: var(--level);
            background-color: #f1f1f1;
            position: relative;
        }

        .battery-fill::before {
            content: '';
            position: absolute;
            bottom: 0;
            left: 0;
            width: 100%;
            height: calc(40% + var(--level));
            background-color: #4CAF50;
            transition: height 0s;
        }
    </style>
</head>
<body>
    <h1>My Smart Dustbin</h1>

    <div class="battery-icon">
        <div class="battery-fill" id="battery-fill"></div>
    </div>
    <div><p><span id="s1"></span></p></div>
    <script>
        const batteryFill = document.getElementById('battery-fill');
        function getPayLoad() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                let payLoad = JSON.parse(this.responseText);
                var level = payLoad.lvl + '%';
                document.getElementById('s1').innerText = level;
                batteryFill.style.setProperty('--level', level);
            }
          };
          xhttp.open("GET", "/get-payloads", true);
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
    data_buffer = "";
    data_buffer.concat("{\"lvl\":");
    data_buffer.concat(ser_buf);
    data_buffer.concat("}");
    ser_buf = "";
  }  
}
