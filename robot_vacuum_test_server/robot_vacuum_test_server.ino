// {"v1":14.5,"v2":15.0,"v3":16.0,"ps":1,"bs":0}
#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid     = "RoboVacuum";
const char* password = "RBV-2023";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Remote Control</title>
  <style>
    /* Fullscreen layout */
    html, body {
      height: 100%;
      margin: 0;
    }

    /* Remote control container */
    .remote-control {
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      height: 100%;
    }

    /* Button container */
    .button-container {
      display: flex;
      justify-content: center;
      align-items: center;
      margin: 10px;
    }

    /* Buttons */
    .remote-control button {
      width: 150px;
      height: 150px;
      margin: 10px;
      font-size: 18px;
      border-radius: 50%;
      border: none;
    }

    /* Forward button */
    .forward {
      background-color: #ff5c5c;
    }

    /* Backward button */
    .backward {
      background-color: #5c5cff;
    }

    /* Stop button */
    .stop {
      background-color: #cccccc;
      color: #ffffff;
    }

    /* Turn left button */
    .left {
      background-color: #5cff5c;
    }

    /* Turn right button */
    .right {
      background-color: #ffff5c;
    }
  </style>
</head>
<body>
  <div class="remote-control">
    <div class="button-container">
      <button class="forward" onclick="forward()">Forward</button>
    </div>
    <div class="button-container">
      <button class="left" onclick="left()">Left</button>
      <button class="stop" onclick="stop()">Stop</button>
      <button class="right" onclick="right()">Right</button>
    </div>
    <div class="button-container">
      <button class="backward" onclick="backward()">Backward</button>
    </div>
  </div>
  <script lang="text/javascript">
  function forward()
  {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
      }
      };
      xhttp.open("GET", "/forward", true);
      xhttp.send();
  }
  function backward()
  {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
      }
      };
      xhttp.open("GET", "/backward", true);
      xhttp.send();
  }
  function left()
  {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
      }
      };
      xhttp.open("GET", "/turn-left", true);
      xhttp.send();
  }
  function right()
  {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
      }
      };
      xhttp.open("GET", "/turn-right", true);
      xhttp.send();
  }
  function stop()
  {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
      }
      };
      xhttp.open("GET", "/stop", true);
      xhttp.send();
  }
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

   server.on("/forward", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+fwd");
    request->send(200);
    });

    server.on("/backward", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+bwd");
    request->send(200);
    });

    server.on("/turn-right", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+tr");
    request->send(200);
    });

    server.on("/turn-left", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+tl");
    request->send(200);
    });

    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+stop");
    request->send(200);
    });

    server.on("/speed", HTTP_GET, [](AsyncWebServerRequest * request) {
    input = "";
    input.concat("[");
    input.concat(request->getParam(0)->value());
    input.concat("]");
    Serial.println(input);
    request->send(200);
    });

    server.on("/get-data", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", data_buffer.c_str());
    });
    
   server.begin();
}

void loop() {
  statusLed.Update();
  if(millis() - last_millis >= 1500)
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
