#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* AP_SSID = "Sewage Monitor";
const char* AP_PASS = "SM2-2025";   // 8+ chars or "" for open AP

const float MAX_HEIGHT_CM = 80.0;          // Distance from sensor to bottom
const float CRITICAL_CM_FROM_SENSOR = 5.0; // Alarm threshold

const uint8_t PIN_TRIG   = D2;
const uint8_t PIN_ECHO   = D8;
const uint8_t PIN_LED_RED  = D6;
const uint8_t PIN_LED_BLUE = D5; // power indicator
const uint8_t PIN_BUZZER = D7;

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
  body { 
    background:#081c1f; 
    color:#e0fbe6; 
    font-family:'Segoe UI', Tahoma, sans-serif; 
    text-align:center; 
    margin:0; 
    padding:20px;
  }
  h2 {
    color:#00ffaa; 
    text-shadow:0 0 8px #00ffaa;
  }
  .container {
    display:flex; 
    justify-content:center; 
    margin-top:25px;
  }
  .water {
    width:180px; 
    height:360px; 
    background:#102a2d; 
    border-radius:20px; 
    overflow:hidden; 
    position:relative; 
    box-shadow:0 0 15px rgba(0,255,170,0.3);
  }
  .fill {
    position:absolute; 
    bottom:0; 
    left:0; 
    width:100%; 
    height:0; 
    background:linear-gradient(to top, rgba(0,255,170,0.3), rgba(0,255,170,0.6));
    transition:height 0.8s;
  }
  /* wave container is stuck to top of .fill */
  .wave-container {
    position:absolute;
    top:0;
    left:0;
    width:100%;
    height:40px; /* visible crest height */
    overflow:hidden;
  }
  .wave {
    position:absolute;
    top:-50%; /* shift up so crest aligns exactly at fill surface */
    left:0;
    width:200%;
    height:200%;
    background:rgba(0,255,170,0.7);
    border-radius:40%;
    animation: waveMove 6s linear infinite, waveBob 3s ease-in-out infinite;
    opacity:0.7;
  }
  .wave:nth-child(2) {
    background:rgba(0,180,120,0.8);
    animation: waveMove 10s linear infinite reverse, waveBob 4s ease-in-out infinite;
  }
  @keyframes waveMove {
    0% { transform:translateX(0) rotate(0deg); }
    100% { transform:translateX(-50%) rotate(-360deg); }
  }
  @keyframes waveBob {
    0%,100% { transform:translateY(-2px); }
    50% { transform:translateY(2px); }
  }

  p {
    font-size:18px; 
    margin:8px 0;
  }
  .status {
    margin-top:20px; 
    font-size:20px; 
    font-weight:bold; 
    padding:10px; 
    border-radius:8px; 
    display:inline-block;
    min-width:160px;
  }
  .normal {
    background:#003d2f; 
    color:#00ffaa; 
    box-shadow:0 0 10px #00ffaa;
  }
  .critical {
    background:#330000; 
    color:#ff4444; 
    box-shadow:0 0 10px #ff4444;
  }
</style>
</head>
<body>
  <h2>Sewage Level Monitor</h2>
  <div class="container">
    <div class="water">
      <div class="fill" id="fill">
        <div class="wave-container">
          <div class="wave"></div>
          <div class="wave"></div>
        </div>
      </div>
    </div>
  </div>
  <p>Distance: <span id="distance">--</span> cm</p>
  <p>Fill: <span id="level">--</span>%</p>
  <div class="status normal" id="status">Waiting...</div>

<script>
async function refresh() {
  try {
    const r = await fetch('/api/status');
    const j = await r.json();
    if (j.distance_cm >= 0) {
      document.getElementById('distance').textContent = j.distance_cm.toFixed(1);
      document.getElementById('level').textContent = j.percent.toFixed(1);
      document.getElementById('fill').style.height = j.percent + '%';

      const status = document.getElementById('status');
      if (j.critical) {
        status.textContent = "CRITICAL LEVEL!";
        status.className = "status critical";
      } else {
        status.textContent = "Normal";
        status.className = "status normal";
      }
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
