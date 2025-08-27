#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* AP_SSID = "Sewage Monitor";
const char* AP_PASS = "SM1-2025";   // 8+ chars or "" for open AP

const float MAX_HEIGHT_CM = 80.0;          // Distance from sensor to bottom
const float CRITICAL_CM_FROM_SENSOR = 5.0; // Alarm threshold

const uint8_t PIN_TRIG   = D2;
const uint8_t PIN_ECHO   = D8;
const uint8_t PIN_LED_RED  = D7;
const uint8_t PIN_LED_BLUE = D6; // power indicator
const uint8_t PIN_BUZZER = D5;

AsyncWebServer server(80);

float lastDistance = NAN;
float lastFill = NAN;
float lastPercent = NAN;
bool lastCritical = false;
unsigned long lastMeasureTime = 0;

float readDistanceCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_ECHO, HIGH, 25000UL); // timeout ~25ms
  if (duration == 0) return NAN;
  return duration / 58.0f;
}

void updateMeasurement() {
  float dist = readDistanceCm();
  if (isnan(dist)) {
    lastDistance = NAN;
    lastFill = NAN;
    lastPercent = 0;
    lastCritical = false;
    return;
  }

  lastDistance = dist;
  lastFill = MAX_HEIGHT_CM - dist;
  if (lastFill < 0) lastFill = 0;
  if (lastFill > MAX_HEIGHT_CM) lastFill = MAX_HEIGHT_CM;

  lastPercent = (lastFill / MAX_HEIGHT_CM) * 100.0f;
  if (lastPercent < 0) lastPercent = 0;
  if (lastPercent > 100) lastPercent = 100;

  lastCritical = (dist <= CRITICAL_CM_FROM_SENSOR);

  digitalWrite(PIN_LED_RED, lastCritical ? HIGH : LOW);
  digitalWrite(PIN_BUZZER,  lastCritical ? HIGH : LOW);
  digitalWrite(PIN_LED_BLUE, HIGH); // always on
}

String jsonStatus() {
  String s = "{";
  s += "\"distance_cm\":" + String(isnan(lastDistance) ? -1 : lastDistance, 1) + ",";
  s += "\"fill_cm\":" + String(isnan(lastFill) ? -1 : lastFill, 1) + ",";
  s += "\"percent\":" + String(lastPercent, 1) + ",";
  s += "\"critical\":" + String(lastCritical ? "true" : "false");
  s += "}";
  return s;
}

const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Sewage Monitor</title>
<style>
body { background:#0b1220; color:white; font-family:sans-serif; text-align:center; }
.water { width:200px; height:400px; margin:auto; background:#222; border-radius:12px; overflow:hidden; position:relative; }
.fill { position:absolute; bottom:0; left:0; width:100%; height:0; background:linear-gradient(#4da3ff,#2d6bff); transition:height 0.8s; }
.status { margin-top:20px; font-size:18px; }
</style>
</head>
<body>
  <h2>Sewage Level Monitor</h2>
  <div class="water"><div class="fill" id="fill"></div></div>
  <p>Distance: <span id="distance">--</span> cm</p>
  <p>Fill: <span id="level">--</span>%</p>
  <div class="status" id="status">Waiting...</div>
<script>
async function refresh() {
  try {
    const r = await fetch('/api/status');
    const j = await r.json();
    if (j.distance_cm >= 0) {
      document.getElementById('distance').textContent = j.distance_cm.toFixed(1);
      document.getElementById('level').textContent = j.percent.toFixed(1);
      document.getElementById('fill').style.height = j.percent + '%';
      document.getElementById('status').textContent = j.critical ? "CRITICAL LEVEL!" : "Normal";
      document.getElementById('status').style.color = j.critical ? "red" : "lime";
    }
  } catch(e) { console.error(e); }
}
setInterval(refresh, 1500);
refresh();
</script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_TRIG, LOW);
  digitalWrite(PIN_LED_BLUE, HIGH);
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", PAGE_INDEX);
  });

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", jsonStatus());
  });

  server.begin();
}

void loop() {
  // Measure every 1s
  if (millis() - lastMeasureTime >= 500) {
    updateMeasurement();
    lastMeasureTime = millis();
  }
}
