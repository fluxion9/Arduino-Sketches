#include <WiFi.h>

#include <ESPAsyncWebServer.h>

#define LED_PIN 5

AsyncWebServer server(80);

const char* ssid = "ES Diary";
const char* password = "123456789";

const String index_html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 LED Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #0f172a;
      color: #fff;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      height: 100vh;
      margin: 0;
    }
    .container {
      background: #1e293b;
      padding: 30px 40px;
      border-radius: 12px;
      box-shadow: 0 0 15px rgba(0,0,0,0.4);
      text-align: center;
    }
    h2 {
      margin-bottom: 20px;
    }
    button {
      padding: 12px 20px;
      margin: 10px;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      font-weight: bold;
      width: 100px;
      font-size: 16px;
    }
    .on { background: #16a34a; color: #fff; }
    .off { background: #dc2626; color: #fff; }
    .status {
      margin-top: 15px;
      font-size: 0.9em;
      color: #94a3b8;
    }
  </style>
  </head>
  <body>
    <div class="container">
      <h2>ESP32 LED Control</h2>
      <div>
        <button class="on" onclick="turnOnLED()">ON</button>
        <button class="off" onclick="turnOffLED()">OFF</button>
      </div>
      <div class="status" id="status"></div>
    </div>

  <script>
  const statusEl = document.getElementById('status');

  function turnOnLED() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        statusEl.textContent = `LED turned ON`;
      }
    };
    xhttp.open("GET", "/on", true);
    xhttp.send();
  }

  function turnOffLED() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        statusEl.textContent = `LED turned OFF`;
      }
    };
    xhttp.open("GET", "/off", true);
    xhttp.send();
  }
  </script>
  </body>
  </html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  Serial.print("\n\n\n");

  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-FI");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  IPAddress STA_IP = WiFi.localIP();

  Serial.println("Connected. ");

  Serial.print("IP address: ");

  Serial.println(STA_IP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED Turned ON");
    request->send(200);
  });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED Turned OFF");
    request->send(200);
  });

  server.begin();
}

void loop() {
}
