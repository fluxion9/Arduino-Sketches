#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "DHT.h"
#include <EEPROM.h>

#define DHTPIN D5
#define DHTTYPE DHT22
#define LED_PIN D7
#define BTN_PIN D2
#define HUM_PWR_PIN D1

DHT dht(DHTPIN, DHTTYPE);

AsyncWebServer server(80);

// Settings storage
struct Settings {
  float targetHumidity;
  int mode;  // 0=off, 1=steady, 2=intermittent, 3=manual (steady), 4=manual (intermittent)
} settings;

bool spraying = false;

const char *ap_ssid = "Home Climate Control";
const char *ap_password = "HMC-2025";

// Save settings to EEPROM
void saveSettings() {
  EEPROM.begin(512);
  EEPROM.put(0, settings);
  EEPROM.commit();
  EEPROM.end();
  spraying = false;
}

// Load  from EEPROM
void loadSettings() {
  EEPROM.begin(512);
  EEPROM.get(0, settings);
  EEPROM.end();
  if (isnan(settings.targetHumidity)) {
    settings.targetHumidity = 60.0;
    settings.mode = 0;
    saveSettings();
  }
}

// Simulate button press
void toggleSpray(int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(BTN_PIN, HIGH);
    delay(200);
    digitalWrite(BTN_PIN, LOW);
    delay(500);
  }
}

const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Home Climate Control</title>
  <style>
    :root {
      --primary: #0078D7;
      --primary-dark: #005FA3;
      --bg: #f0f2f5;
      --card-bg: #fff;
      --text: #333;
      --muted: #666;
    }

    body {
      font-family: 'Segoe UI', Tahoma, sans-serif;
      background: var(--bg);
      margin: 0;
      padding: 20px;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
    }

    .card {
      background: var(--card-bg);
      padding: 25px;
      border-radius: 18px;
      box-shadow: 0 8px 16px rgba(0,0,0,0.12);
      width: 100%;
      max-width: 400px;
    }

    h1 {
      color: var(--primary);
      margin-bottom: 20px;
      font-size: 1.6em;
    }

    .reading {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 12px 15px;
      margin: 8px 0;
      border-radius: 12px;
      background: #f9f9f9;
      font-size: 1.1em;
      color: var(--text);
    }

    .reading span {
      font-weight: bold;
      color: var(--primary);
      font-size: 1.2em;
    }

    hr {
      margin: 24px 0;
      border: 0;
      border-top: 1px solid #ddd;
    }

    label {
      display: block;
      margin: 12px 0 6px;
      font-weight: 600;
      color: var(--muted);
      text-align: left;
    }

    input, select {
      width: 100%;
      padding: 10px;
      border-radius: 10px;
      border: 1px solid #ccc;
      font-size: 1em;
      box-sizing: border-box;
    }

    button {
      margin-top: 20px;
      padding: 12px;
      width: 100%;
      border: none;
      border-radius: 10px;
      background: var(--primary);
      color: white;
      font-size: 1.1em;
      cursor: pointer;
      font-weight: bold;
      transition: background 0.3s;
    }

    button:hover {
      background: var(--primary-dark);
    }

    footer {
      margin-top: 18px;
      font-size: 0.85em;
      color: var(--muted);
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>🌡 Home Climate Control</h1>

    <div class="reading">Temperature <span id="temp">--</span>°C</div>
    <div class="reading">Humidity <span id="hum">--</span>%</div>
    <div class="reading">Status <span id="status">--</span></div>

    <hr>

    <form onsubmit="saveSettings(); return false;">
      <label for="target">Target Humidity (%)</label>
      <input type="number" id="target" min="20" max="90">

      <label for="mode">Spray Method</label>
      <select id="mode">
        <option value="0">Off</option>
        <option value="1">Steady</option>
        <option value="2">Intermittent</option>
        <option value="3">Manual (Steady)</option>
        <option value="4">Manual (Intermittent)</option>
      </select>

      <button type="submit">💾 Save Settings</button>
    </form>

    <footer>Home Climate Controller</footer>
  </div>

<script>
  let firstLoad = true;

  function modeToText(mode){
    if(mode == 0) return "Off";
    if(mode == 1) return "Steady";
    if(mode == 2) return "Intermittent";
    if(mode == 3) return "Manual (Steady)";
    if(mode == 4) return "Manual (Intermittent)";
    return "Unknown";
  }

  async function fetchData(){
    let res = await fetch('/status');
    let data = await res.json();
    document.getElementById('temp').innerText = data.temp;
    document.getElementById('hum').innerText = data.hum;
    document.getElementById('status').innerText = modeToText(data.mode);

    if(firstLoad){
      document.getElementById('target').value = data.target;
      document.getElementById('mode').value = data.mode;
      firstLoad = false;
    }
  }

  setInterval(fetchData, 1500);
  fetchData();

  async function saveSettings(){
    let target = document.getElementById('target').value;
    let mode = document.getElementById('mode').value;
    await fetch(`/set?target=${target}&mode=${mode}`);
    alert("Settings Saved");
  }
</script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, OUTPUT);
  pinMode(HUM_PWR_PIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BTN_PIN, LOW);
  digitalWrite(HUM_PWR_PIN, LOW);

  dht.begin();

  loadSettings();

  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("AP Started");

  // HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", PAGE_INDEX);
  });

  // API: status
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    String json = "{\"hum\":" + String(isnan(h) ? 0 : h, 1) + ",\"temp\":" + String(isnan(t) ? 0 : t, 1) + ",\"target\":" + String(settings.targetHumidity, 1) + ",\"mode\":" + String(settings.mode) + "}";
    request->send(200, "application/json", json);
  });

  // API: set
  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("target")) settings.targetHumidity = request->getParam("target")->value().toFloat();
    if (request->hasParam("mode")) settings.mode = request->getParam("mode")->value().toInt();
    saveSettings();
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  float h = dht.readHumidity();
  if (isnan(h)) return;


  if (settings.mode == 0) {
    digitalWrite(HUM_PWR_PIN, LOW);
    spraying = false;
  } else if (settings.mode == 1) {
    if (h < settings.targetHumidity) {
      if (!spraying) {
        digitalWrite(HUM_PWR_PIN, HIGH);
        delay(1500);
        toggleSpray(1);
        spraying = true;
      }
    } else {
      digitalWrite(HUM_PWR_PIN, LOW);
      delay(1500);
      spraying = false;
    }
  } else if (settings.mode == 2) {
    if (h < settings.targetHumidity) {
      if (!spraying) {
        digitalWrite(HUM_PWR_PIN, HIGH);
        delay(1500);
        toggleSpray(2);
        spraying = true;
      }
    } else {
      digitalWrite(HUM_PWR_PIN, LOW);
      spraying = false;
    }
  } else if (settings.mode == 3 && !spraying) {
    digitalWrite(HUM_PWR_PIN, HIGH);
    delay(1500);
    toggleSpray(1);
    spraying = true;
  } else if (settings.mode == 4 && !spraying) {
    digitalWrite(HUM_PWR_PIN, HIGH);
    delay(1500);
    toggleSpray(2);
    spraying = true;
  }
}
