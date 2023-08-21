#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "JohnPaul's RobotTrash";
const char* password = "JPRTS-2023";

AsyncWebServer server(80);

const char remote_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Remote Control</title>
  <style>

  button {
    user-select: none;
    -webkit-user-select: none; /* For Safari */
    -moz-user-select: none; /* For Firefox */
    -ms-user-select: none; /* For Internet Explorer */
    }
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
      <button class="forward" onmousedown="forward()" onmouseup="stop()" ontouchstart="forward()" ontouchend="stop()">Forward</button>
    </div>
    <div class="button-container">
      <button class="left" onmousedown="left()" onmouseup="stop()" ontouchstart="left()" ontouchend="stop()">Left</button>
      <button class="stop" onclick="stop()">Stop</button>
      <button class="right" onmousedown="right()" onmouseup="stop()" ontouchstart="right()" ontouchend="stop()">Right</button>
    </div>
    <div class="button-container">
      <button class="backward" onmousedown="backward()" onmouseup="stop()" ontouchstart="backward()" ontouchend="stop()">Backward</button>
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
      xhttp.open("GET", "http://192.168.4.1/forward", true);
      xhttp.send();
  }
  function backward()
  {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
      }
      };
      xhttp.open("GET", "http://192.168.4.1/backward", true);
      xhttp.send();
  }
  function left()
  {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
      }
      };
      xhttp.open("GET", "http://192.168.4.1/turn-left", true);
      xhttp.send();
  }
  function right()
  {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
      }
      };
      xhttp.open("GET", "http://192.168.4.1/turn-right", true);
      xhttp.send();
  }
  function stop()
  {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
      }
      };
      xhttp.open("GET", "http://192.168.4.1/stop", true);
      xhttp.send();
  }
  </script>
</body>
</html>
)rawliteral";

const char path_routing_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
<html>
<head>
    <title>Path Routing</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f2f2f2;
            margin: 0;
            padding: 0; /* Remove padding to make the canvas flush with the top */
        }

        .navbar {
            background-color: black;
            color: goldenrod; /* Changed text color to golden yellow */
            text-align: center;
            padding: 10px 0;
            margin: 0; /* Remove margin */
        }

        h1 {
            margin: 0;
        }

        .navbar span {
            margin: 0 10px; /* Add space between spans */
        }

        #canvasContainer {
            text-align: center;
            margin: 20px auto 0; /* Added margin to create space between navbar and canvas */
        }

        #canvas {
            border: 1px solid #333;
            background-color: #fff;
            cursor: crosshair;
            margin-top: 20px; /* Add margin to create space between bin button and canvas */
        }

        #resetButton, #binButton {
            display: block;
            margin: 20px auto 0; /* Center the button above the canvas */
            padding: 10px 20px;
            font-size: 16px;
            background-color: #333;
            color: #fff;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }

        #resetButton:hover, #binButton:hover {
            background-color: #555;
        }
    </style>
</head>
<body>
    <div class="navbar">
        <h1>John Paul's Autonomous Trash Can Path Routing</h1>
        <span>GPS status: <span id="gstat">Not Ready</span></span>
        <span>Longitude:<span id="lgd">--</span></span>
        <span>Latitude:<span id="ltd">--</span></span>
        <span>Compass:<span id="ang">--</span></span>
        <span>Trash Level:<span id="tlvl">0.00</span>cm</span>
        <span>Bin Status:<span id="bstat">Not Locked</span></span>
        <span>Battery Voltage:<span id="vbatt">0.00</span>V</span>
    </div>
    <div id="canvasContainer">
        <button id="binButton">Open/Close Bin</button>
        <canvas id="canvas" width="400" height="400"></canvas>
        <button id="resetButton">Reset</button>
    </div>

    <script>
        let gstat = document.getElementById("gstat");
        let tlvl = document.getElementById("tlvl");
        let bstat = document.getElementById("bstat");
        let lgd = document.getElementById("lgd");
        let ltd = document.getElementById("ltd");
        let ang = document.getElementById("ang");
        let vbatt = document.getElementById("vbatt");

        var gpsstat = "Not Ready";
        var lockstat = "Not Locked";


        var canvas = document.getElementById("canvas");
        var context = canvas.getContext("2d");
        var pathCoordinates_x = [];
        var pathCoordinates_y = [];
        var pathCoordinates = [];

        var isDrawing = false;

        canvas.addEventListener("mousedown", startDrawing);
        canvas.addEventListener("mousemove", drawPath);
        canvas.addEventListener("mouseup", endDrawing);

        var resetButton = document.getElementById("resetButton");
        resetButton.addEventListener("click", resetCanvas);

        var binButton = document.getElementById("binButton"); // Get the bin button
        binButton.addEventListener("click", toggleBin); // Add click event listener

        var gridStepSizeMM = 4; // 1 cm is equivalent to 10 mm, 1 cm = 10 mm / 2.5 px = 4 mm/px
        var gridStepSizeCM = 40; // 1 cm is equivalent to 40 px

        function startDrawing(event) {
            pathCoordinates = [];
            pathCoordinates_x = [];
            pathCoordinates_y = [];
            isDrawing = true;
            startPoint = getMousePosition(event);
            pathCoordinates_x.push(startPoint[0]);
            pathCoordinates_y.push(startPoint[1]);
            context.beginPath();
            context.moveTo(startPoint[0], startPoint[1]);
        }

        function drawPath(event) {
            if (!isDrawing) return;
            currentPoint = getMousePosition(event);
            pathCoordinates_x.push(currentPoint[0]);
            pathCoordinates_y.push(currentPoint[1]);
            drawLine(currentPoint);
        }

        function endDrawing() {
            isDrawing = false;
            pathCoordinates.push(pathCoordinates_x);
            pathCoordinates.push(pathCoordinates_y);
            var ploStr = JSON.stringify(pathCoordinates);
            if(gpsstat != "Ready")
            {
              alert("GPS Not Ready!");
            }else {
              console.log(ploStr);
            }
        }

        function resetCanvas() {
            context.clearRect(0, 0, canvas.width, canvas.height);
            drawGrid();
            pathCoordinates = [];
        }

        function toggleBin() {
            ControlBin();
            console.log("Toggle Bin button clicked");
        }

        function getMousePosition(event) {
            var rect = canvas.getBoundingClientRect();
            var x = event.clientX - rect.left;
            var y = event.clientY - rect.top;
            x = Math.round(x);
            y = Math.round(y);
            return [x, y];
        }

        function drawLine(position) {
            context.lineWidth = 10;
            context.strokeStyle = "black";
            context.lineTo(position[0], position[1]);
            context.stroke();
        }

        function drawGrid() {
            context.strokeStyle = "#ddd";
            context.lineWidth = 0.5;

            // Draw 1mm grid
            for (var x = 0; x <= canvas.width; x += gridStepSizeMM) {
                context.beginPath();
                context.moveTo(x, 0);
                context.lineTo(x, canvas.height);
                context.stroke();
            }
            for (var y = 0; y <= canvas.height; y += gridStepSizeMM) {
                context.beginPath();
                context.moveTo(0, y);
                context.lineTo(canvas.width, y);
                context.stroke();
            }

            // Draw 1cm grid
            context.strokeStyle = "#999";
            context.lineWidth = 1;
            for (var x = 0; x <= canvas.width; x += gridStepSizeCM) {
                context.beginPath();
                context.moveTo(x, 0);
                context.lineTo(x, canvas.height);
                context.stroke();
            }
            for (var y = 0; y <= canvas.height; y += gridStepSizeCM) {
                context.beginPath();
                context.moveTo(0, y);
                context.lineTo(canvas.width, y);
                context.stroke();
            }
        }

        function getPayLoad() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                let payLoad = JSON.parse(this.responseText);
                vbatt.innerText = payLoad.batt;
                lgd.innerText = payLoad.lng;
                ltd.innerText = payLoad.lat;
                tlvl.innerText = payLoad.top;
                ang.innerText = payLoad.azm;
                if(payLoad.gstat)
                {
                  gstat.innerText = "Ready";
                  gpsstat = "Ready";
                }else{
                  gstat.innerText = "Not Ready";
                  gpsstat = "Not Ready";
                }
                if(payLoad.bstat)
                {
                  bstat.innerText = "Locked";
                  lockstat = "Locked";
                }else{
                  bstat.innerText = "Not Locked";
                  lockstat = "Not Locked";
                }
            }
          };
          xhttp.open("GET", "/get-data", true);
          xhttp.send();
        }

        function ControlBin()
        {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            }
          };
          xhttp.open("GET", "/open-close", true);
          xhttp.send();
        }

        drawGrid();
        getPayLoad();
        setInterval(getPayLoad, 2000);
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
    request->send_P(200, "text/plain", "Hello!");
    });

  server.on("/remote", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", remote_html);
    });

  server.on("/path-routing", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", path_routing_html);
    });

   server.on("/forward", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+fwd;");
    request->send(200);
    });

    server.on("/backward", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+bwd;");
    request->send(200);
    });

    server.on("/turn-right", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+tr;");
    request->send(200);
    });

    server.on("/turn-left", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+tl;");
    request->send(200);
    });

    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+stop;");
    request->send(200);
    });

    server.on("/open-close", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("+opcl;");
    request->send(200);
    });

    server.on("/cw", HTTP_GET, [](AsyncWebServerRequest * request) {
     input = "";
     input.concat("[");
     input.concat("cw,");
     input.concat(request->getParam(0)->value());
     input.concat("];");
     Serial.println(input);
     request->send(200);
    });

    server.on("/ccw", HTTP_GET, [](AsyncWebServerRequest * request) {
     input = "";
     input.concat("[");
     input.concat("ccw,");
     input.concat(request->getParam(0)->value());
     input.concat("];");
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
