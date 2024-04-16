#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "EEPROM.h"

#define EEPROM_SIZE 1024

// #define buzzer 2

const char *ssid = "RFID.net";
const char *password = "RFID-2023";

char sta_num[3] = "0";
char carddata[2048];
char payload[2048];

const char master[] = "6a026380";
bool progmode = false;

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RFID Project Dashboard</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=DM+Sans:ital,opsz,wght@0,9..40,100..1000;1,9..40,100..1000&display=swap');
        * {
            padding: 0;
            margin: 0;
            box-sizing: border-box;
        }
        body {
            display: flex;
            justify-content: space-between;
            width: 100vw;
            height: 100vh;
            font-family: DM Sans;
        }
        .sidebar {
            background-color: black;
            width: 20vw;
            padding: 3rem 1rem;
            .dashboard {
                background-color: white;
                padding: 0.7rem 1rem;
                border-radius: 10px;
                cursor: pointer;
                display: flex;
                align-items: center;
                gap: 0.8rem;
                .dash-icon {
                    width: 18px;
                    height: 18px;
                    object-fit: cover;
                    object-position: center;
                }
                a {
                    color: black;
                    text-decoration: none;
                    font-size: 1.1rem;
                    font-weight: 400;
                    width: 100%;
                    height: 100%;
                }
            }
        }

        .main {
            width: 80vw;
            padding: 1rem 2rem 0 2rem;
            background-color: lightgray;
            .header {
                background-color: white;
                padding: 1.4rem;
                border-radius: 10px;
                display: flex;
                justify-content: space-between;
                align-items: center;
                margin-bottom: 2rem;
                h1 {
                    font-size: 1.5rem;
                }
                .connect {
                    background-color: #E0E0E0;
                    border-radius: 5px;
                    padding: 0.75rem;
                }
            }
            .overview {
                margin-bottom: 2rem;
                h3 {
                    margin-bottom: 1rem;
                }
                .over {
                    display: flex;
                    gap: 4rem;
                    div {
                        display: flex;
                        flex-direction: column;
                        align-items: center;
                        p {
                            font-size: 0.7rem;
                        }
                        span {
                            font-size: 2rem;
                            font-weight: bold;
                        }
                    }
                }
            }
            .nodes {
                display: flex;
                align-items: center;
                gap: 1.5rem;
                margin-bottom: 2rem;
                div {
                    background-color: white;
                    padding: 0.75rem 1rem;
                    border-radius: 5px;
                    cursor: pointer;
                }
                div:hover {
                    box-shadow: 1px 1px 1px rgba(0, 0, 0, 0.7);
                    background-color: rgba(0,0,0);
                    color: white
                }
                .active {
                    background-color: black;
                    color: white;
                }
            }
            .table {
                overflow-y: auto;
                height: 300px;
                table {
                    width: 100%;
                    font-family: arial, sans-serif;
                    border-collapse: collapse;

                }
                .table-data-1,
                .table-data {
                    td, th {
                        border: 1px solid #dddddd;
                        text-align: left;
                        padding: 8px;
                        border-color: #232323;
                    }
                    tr:nth-child(odd) {
                        background-color: #333333;
                        color: white;
                    }
                }
            }
        }
    </style>
</head>
<body>
    <section class="sidebar">
        <div class="dashboard">
            <!--<img src="/dashboard.png" alt="dashboard" class="dash-icon">-->
            <a href="/main.html">Dashboard</a>
        </div>

    </section>
    <section class="main">
        <section class="header">
            <h1>RFID Project Dashboard</h1>

            <div class="connect">
                <p>Connected Devices: <span class="device"></span></p>
            </div>
        </section>

        <section class="overview">
            <h3>Overview</h3>
            <div class="over">
                <div>
                    <p>No of Saved Cards</p>
                    <span class="num"></span>
                </div>
            </div>
        </section>
        <section class="table">
            <table class="table-data-1">
                <tr>
                  <th>S/N</th>
                  <th>Registered UIDs</th>
                </tr>
                <table class="table-data">
                </table>
            </table>
        </section>
    </section>
    <script>
        var nodes = document.querySelectorAll(".reader");
        const tableData = document.querySelector(".table-data");
        const numUID = document.querySelector(".num");
        const devnum = document.querySelector(".device");

        function getpayloads() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    let payload = JSON.parse(this.responseText);
                    devnum.innerHTML = payload.stanum;
                    let nodeData = payload.cards;
                    let connected = nodeData.length;
                    numUID.innerHTML = connected;
                    generateTable(nodeData);
                }
            };
            xhttp.open("GET", "/get-payload", true);
            xhttp.send();
        }

        function generateTable(nodeData) {
            const tableBody = document.createElement("tbody");
            nodeData.map((value, index) => {
                const dataRow = document.createElement("tr");
                const dataCell1 = document.createElement("td");
                const dataCell2 = document.createElement("td");
                dataCell1.textContent = index + 1;
                dataCell2.textContent = value;
                dataRow.appendChild(dataCell1);
                dataRow.appendChild(dataCell2);
                tableBody.appendChild(dataRow);
            });
            tableData.innerHTML = "";
            tableData.appendChild(tableBody);
        }
        getpayloads();
        setInterval(getpayloads, 1500);
    </script>
</body>
</html>
)rawliteral";

unsigned long last_millis = 0;

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
};

Blinker statusLed(LED_BUILTIN, 5000, 300);

int findcard(const char cuid[], const int size) {
  int records = EEPROM.read(0);
  int slots[records];
  int slotsindex = 0, slotcount = 0;
  for (int i = 1; i < EEPROM_SIZE - 1; i += 10) {
    if (EEPROM.read(i) == 8) {
      slotcount++;
      slots[slotsindex++] = i;
    }
    if (slotcount >= records) {
      break;
    }
  }
  char card[size + 1];
  for (int i = 0; i < slotcount; i++) {
    for (int j = 0; j < 8; j++) {
      card[j] = EEPROM.read(slots[i] + j + 1);
    }
    card[8] = '\0';
    if (strcmp(card, cuid) == 0) {
      return slots[i];
    }
  }
  return -1;
}

int addcard(const char cuid[], const int size) {
  int slotpos = -1;
  for (int i = 1; i < EEPROM_SIZE - 1; i += 10) {
    if (EEPROM.read(i) == 0) {
      slotpos = i;
      break;
    }
  }
  uint8_t records = EEPROM.read(0);
  int maxrecords = (EEPROM_SIZE - 1) / 9;
  if (records >= maxrecords) {
    return -1;
  } else {
    EEPROM.write(slotpos, size);
    for (int i = 0; i < size; i++) {
      EEPROM.write(slotpos + 1 + i, (uint8_t)cuid[i]);
    }
    EEPROM.write(0, records + 1);
    EEPROM.commit();
    return 1;
  }
}

void deletecard(const int index) {
  uint8_t records = EEPROM.read(0);
  for (int i = 0; i < 8; i++) {
    EEPROM.write(index + 1 + i, 0);
  }
  EEPROM.write(index, 0);
  EEPROM.write(0, records - 1);
  EEPROM.commit();
}

int handleCard(char input[], int size) {
  if(size < 8)
  {
    return 8;
  }
  if (strcmp(input, master) == 0 && !progmode) {
    progmode = true;
    Serial.println("prog mode ON");
    return 2;
  } else if (strcmp(input, master) == 0 && progmode) {
    Serial.println("prog mode OFF");
    progmode = false;
    return 3;
  }
  uint8_t records = EEPROM.read(0);
  if (records == 0) {
    if (progmode) {
      if (addcard(input, size) == 1) {
        Serial.println("first card added!");
        return 4;
      } else {
        return 9;
      }
    } else {
      Serial.println("card not found!");
      return 0;
    }
  } else {
    int index = findcard(input, size);
    Serial.print("Index: ");
    Serial.println(index);
    if (index == -1) {
      if (progmode) {
        if (addcard(input, size) == 1) {
          Serial.println("card added!");
          return 4;
        } else {
          return 9;
        }
      } else {
        Serial.println("card not found!");
        return 0;
      }
    } else {
      if (progmode) {
        Serial.println("Card found, deleting...");
        deletecard(index);
        return 5;
      } else {
        Serial.println("Card found, you may pass.");
        return 1;
      }
    }
  }
}

void loadcarddata() {
  carddata[0] = '\0';
  strcat(carddata, "[");
  int records = EEPROM.read(0);
  int slots[records];
  int slotsindex = 0, slotcount = 0;
  for (int i = 1; i < EEPROM_SIZE - 1; i += 10) {
    if (EEPROM.read(i) == 8) {
      slotcount++;
      slots[slotsindex++] = i;
    }
    if (slotcount >= records) {
      break;
    }
  }
  char card[9];
  char buffer[13];
  for (int i = 0; i < slotcount; i++) {
    for (int j = 0; j < 8; j++) {
      card[j] = EEPROM.read(slots[i] + j + 1);
    }
    card[8] = '\0';
    sprintf(buffer, "\"%s\"", card);
    strcat(carddata, buffer);
    if(i < slotcount - 1)
    {
      strcat(carddata, ",");
    }
  }
  strcat(carddata, "]");
}

void loadjson()
{
  payload[0] = '\0';
  sprintf(payload, "{\"stanum\":%s,\"cards\":%s}", sta_num, carddata);
}

void setup() {
  pinMode(0, OUTPUT);
  Serial.begin(9600);

  EEPROM.begin(EEPROM_SIZE);

  delay(2000);

  WiFi.softAP(ssid, password, 1, 0, 8);
  digitalWrite(0, HIGH);

  IPAddress IP = WiFi.softAPIP();

  int records = EEPROM.read(0);
  Serial.print("Found ");
  Serial.print(records);
  Serial.println(" records in EEPROM.");

  carddata[0] = '\0';
  payload[0] = '\0';

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/sta-count", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", sta_num);
  });

  server.on("/get-payload", HTTP_GET, [](AsyncWebServerRequest *request) {
    loadcarddata();
    loadjson();
    request->send_P(200, "text/plain", payload);
  });

  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
    String id = request->getParam(0)->value();
    char buffer[16];
    strcpy(buffer, id.c_str());
    int res = handleCard(buffer, strlen(buffer));
    buffer[0] = '\0';
    itoa(res, buffer, 10);
    Serial.println(id);
    Serial.println(res);
    request->send_P(200, "text/plain", buffer);
  });

  server.begin();
}

void loop() {
  statusLed.Update();
  sprintf(sta_num, "%d", WiFi.softAPgetStationNum());
}
