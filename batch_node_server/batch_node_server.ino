#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "dbnet.net";
const char *password = "dbnet-2024";

char sta_num[3] = "0";

char pirs_data[5] = "0", dhtc_data[33] = "{\"tem\":0,\"fsd\":0}", ldrc_data[33] = "{\"lux\":0,\"mot\":0}", ser_data[33], payload[100];

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IoT Data Board</title>
    <style>
        body {
            text-align: center;
            font-family: Arial, sans-serif;
            background-color: #f2f2f2;
            margin: 0;
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

        h1 {
            color: #c46b18;
            margin-bottom: 1rem;
        }

        .params-container {
            display: flex;
            width: 100%;
            gap: 1rem;
            margin-block: 1rem;
            margin-bottom: 2rem;
            justify-content: center;
        }

        .params {
            border: 1px solid #ccc;
            padding: 1rem;
            border-radius: 6px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: space-between;
            gap: 1rem;
            background-color: #ffffff;
            box-shadow: 0px 2px 6px rgba(0, 0, 0, 0.1);
            transition: transform 0.2s ease-in-out;
        }

        .params:hover {
            transform: translateY(-5px);
        }

        .param-value {
            width: 100%;
            padding: 0.5rem;
            font-weight: 600;
            text-align: center;
            border: none;
            background-color: #ffffff;
            outline: none;
            color: #333;
        }

        @media only screen and (max-width: 780px) {
            main {
                padding: 0;
                padding: 1rem;
            }
            .param {
                width: 100%;
            }
        }
    </style>
</head>
<body>
    <main>
        <h1>Data Board</h1>
        <div class="params-container">
            <div class="params">
                Temperature
                <input
                    title="Temperature"
                    disabled
                    id="temp"
                    class="param-value"
                    value="0"
                />
            </div>
            <div class="params">
                Fan Speed
                <input
                    title="Fan Speed"
                    disabled
                    id="fs"
                    class="param-value"
                    value="0"
                />
            </div>
            <div class="params">
                Light Intensity
                <input
                    title="Light Intensity"
                    disabled
                    id="lux"
                    class="param-value"
                    value="0"
                />
            </div>
            <div class="params">
                Motion 1
                <input
                    title="Motion Sensor"
                    disabled
                    id="mot1"
                    class="param-value"
                    value="No Motion"
                />
            </div>
            <div class="params">
                Motion 2
                <input
                    title="Motion Sensor"
                    disabled
                    id="mot2"
                    class="param-value"
                    value="No Motion"
                />
            </div>
        </div>
    </main>

    <script lang="text/javascript">
        let tem = document.getElementById('temp');
        let fsd = document.getElementById('fs');
        let lux = document.getElementById('lux');
        let mot1 = document.getElementById('mot1');
        let mot2 = document.getElementById('mot2');

        function processmotion(x)
        {
          if(x==1)
          {
            return "Motion Detected!";
          }
          else if(x==0)
          {
            return "No Motion";
          }
        }
        function getPayLoad() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    let payLoad = JSON.parse(this.responseText);
                    tem.setAttribute('value', payLoad.dhtc.tem);
                    fsd.setAttribute('value', payLoad.dhtc.fsd);
                    lux.setAttribute('value', payLoad.ldrc.lux);
                    mot1.setAttribute('value', processmotion(payLoad.ldrc.mot));
                    mot2.setAttribute('value', processmotion(payLoad.pirs));
                }
            };
            xhttp.open("GET", "/get-all", true);
            xhttp.send();
        }
        getPayLoad();
        setInterval(getPayLoad, 1500);
    </script>
</body>
</html>

)rawliteral";


class Blinker {
  int ledPin;
  long onTime;
  long offTime;

  int ledState;
  unsigned long previousMillis;
  unsigned long currentMillis;

public:
  Blinker(int pin, long on, long off) {
    ledPin = pin;
    pinMode(ledPin, OUTPUT);

    onTime = on;
    offTime = off;
    ledState = LOW;
    previousMillis = 0;
  }
  void Update() {
    currentMillis = millis();
    if ((ledState == HIGH) && (currentMillis - previousMillis >= onTime)) {
      ledState = LOW;
      previousMillis = currentMillis;
      digitalWrite(ledPin, ledState);
    } else if ((ledState == LOW) && (currentMillis - previousMillis >= offTime)) {
      ledState = HIGH;
      previousMillis = currentMillis;
      digitalWrite(ledPin, ledState);
    }
  }
} statusLed(LED_BUILTIN, 5000, 300);
;

void handlePing(const char name[], const int n_len, const char value[], const int v_len) {
  if (strcmp(name, "dhtc") == 0) {
    dhtc_data[0] = '\0';
    strcpy(dhtc_data, value);
  } else if (strcmp(name, "pirs") == 0) {
    pirs_data[0] = '\0';
    strcpy(pirs_data, value);
  }
}

void loadjson() {
  payload[0] = '\0';
  sprintf(payload, "{\"sta\":%s,\"pirs\":%s,\"dhtc\":%s,\"ldrc\":%s}", sta_num, pirs_data, dhtc_data, ldrc_data);
}

void flash() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, 1);
    delay(200);
    digitalWrite(LED_BUILTIN, 0);
    delay(200);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, 0);
  WiFi.softAP(ssid, password, 1, 0, 8);
  IPAddress IP = WiFi.softAPIP();
  digitalWrite(LED_BUILTIN, 1);

  payload[0] = '\0';

  ser_data[0] = '\0';

  // pirs_data[0] = '\0';
  // ldrc_data[0] = '\0';
  // dhtc_data[0] = '\0';

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    flash();
    request->send_P(200, "text/html", index_html);
  });

  server.on("/sta-count", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", sta_num);
  });

  server.on("/get-all", HTTP_GET, [](AsyncWebServerRequest *request) {
    loadjson();
    request->send_P(200, "text/plain", payload);
  });

  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
    String name = request->getParam(0)->value();
    String value = request->getParam(1)->value();
    char n_buffer[65], v_buffer[65];
    strcpy(n_buffer, name.c_str());
    strcpy(v_buffer, value.c_str());
    handlePing(n_buffer, strlen(n_buffer), v_buffer, strlen(v_buffer));
    request->send(200);
  });

  server.begin();
}

void loop() {
  statusLed.Update();
  sprintf(sta_num, "%d", WiFi.softAPgetStationNum());
  if (Serial.available()) {
    ser_data[0] = '\0';
    while (Serial.available()) {
      delay(3);
      char m = Serial.read();
      byte len = strlen(ser_data);
      if (m != '\n' || m != '\r') {
        ser_data[len] = m;
        ser_data[len + 1] = '\0';
      }
    }
    strcpy(ldrc_data, ser_data);
  }
}
