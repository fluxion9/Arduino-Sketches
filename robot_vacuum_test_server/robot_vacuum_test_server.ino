#include <WiFi.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "Robot Vacuum";
const char *password = "";

AsyncWebServer server(80);

const char Auto[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Robot Vacuum Dashboard</title>
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
    <h1>Robot Vacuum Cleaner Dashboard</h1>
    <h2>
      Current Location: 
      <span id="currentLongitude">Longitude: --</span>
      <span id="currentLatitude">Latitude: --</span>
    </h2>
  </header>
  <div class="container">
    <form id="roboVacuumForm">
      <div class="input-group">
        <label>Start (Longitude, Latitude)</label>
        <input type="text" placeholder="Longitude" id="startLongitude">
        <input type="text" placeholder="Latitude" id="startLatitude">
      </div>
      <div class="input-group">
        <label>Stop (Longitude, Latitude)</label>
        <input type="text" placeholder="Longitude" id="stopLongitude">
        <input type="text" placeholder="Latitude" id="stopLatitude">
      </div>
      <div class="input-group">
        <label>Cleaning Speed</label>
        <input type="number" placeholder="Cleaning Speed" id="cleaningSpeed" min="0" max="10" value="3">
      </div>
      <div class="input-group">
        <label>Cleaning Type</label>
        <select id="cleaningType">
          <option value="none">--Select Cleaning Type--</option>
          <option value="scanv">Scan-V</option>
          <option value="scanh">Scan-H</option>
        </select>
      </div>
      <div class="input-group">
        <label>Repeat</label>
          <input type="number" id="repeatInput" min="0" value="0">
      </div>
      <button type="submit" id="submitBtn">Submit</button>
    </form>
  </div>
  <script>
    document.getElementById("submitBtn").addEventListener("click", function(event) {
      event.preventDefault();

      var startLongitude = document.getElementById("startLongitude").value;
      var startLatitude = document.getElementById("startLatitude").value;
      var stopLongitude = document.getElementById("stopLongitude").value;
      var stopLatitude = document.getElementById("stopLatitude").value;
      var cleaningSpeed = document.getElementById("cleaningSpeed").value;
      var cleaningType = Array.from(document.getElementById("cleaningType").selectedOptions, option => option.value);
      var repeat = document.getElementById("repeatInput").value;

      var url = "your-server-url?startLongitude=" + startLongitude +
                "&startLatitude=" + startLatitude +
                "&stopLongitude=" + stopLongitude +
                "&stopLatitude=" + stopLatitude +
                "&cleaningSpeed=" + cleaningSpeed +
                "&cleaningType=" + cleaningType.join(",") +
                "&repeat=" + repeat;
      console.log("URL:", url);
    });
  </script>
</body>
</html>
)rawliteral";

const char remote_html[] PROGMEM = R"rawliteral(
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

String data_buffer = "", ser_buf = "", input = "";
unsigned long last_millis = 0;


void setup()
{
  Serial.begin(9600);
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid); // no password
  IPAddress IP = WiFi.softAPIP();
  // Serial.print("AP IP address: ");
  // Serial.println(IP);

  data_buffer.reserve(64);
  ser_buf.reserve(32);
  input.reserve(20);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", "Hello!"); });

  server.on("/remote", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", remote_html); });

  server.on("/auto", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", Auto); });

  server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("+fwd;");
    request->send(200); });

  server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("+bwd;");
    request->send(200); });

  server.on("/turn-right", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("+tr;");
    request->send(200); });

  server.on("/turn-left", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("+tl;");
    request->send(200); });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    Serial.println("+stop;");
    request->send(200); });

  server.on("/cw", HTTP_GET, [](AsyncWebServerRequest *request)
            {
     input = "";
     input.concat("[");
     input.concat("cw,");
     input.concat(request->getParam(0)->value());
     input.concat("];");
     Serial.println(input);
     request->send(200); });

  server.on("/ccw", HTTP_GET, [](AsyncWebServerRequest *request)
            {
     input = "";
     input.concat("[");
     input.concat("ccw,");
     input.concat(request->getParam(0)->value());
     input.concat("];");
     Serial.println(input);
     request->send(200); });

  // server.on("/speed", HTTP_GET, [](AsyncWebServerRequest * request) {
  // input = "";
  // input.concat("[");
  // input.concat(request->getParam(0)->value());
  // input.concat("]");
  // Serial.println(input);
  // request->send(200);
  // });

  server.on("/get-data", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", data_buffer.c_str()); });

  server.begin();
}

void loop()
{
  if (millis() - last_millis >= 3000)
  {
    Serial.println("+read;");
    last_millis = millis();
  }
  while (Serial.available() > 0)
  {
    delay(3);
    char c = Serial.read();
    ser_buf += c;
  }
  if (ser_buf.length() > 0)
  {
    ser_buf.trim();
    data_buffer = ser_buf;
    ser_buf = "";
  }
}
