

#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "OGSSC-01";       //change this to your wifi username
const char* password = "OG-2024*";  //change this to your wifi password

const char* mqtt_server = "test.mosquitto.org";

WiFiClient espclient;
PubSubClient mqttclient(espclient);

char data[90];

void setupWiFI() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  WaitConnectWiFi();
  // Serial.println(WiFi.localIP());
}

void WaitConnectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
  digitalWrite(LED_BUILTIN, HIGH);
}


void setup() {
  Serial.begin(9600);

  setupWiFI();

  mqttclient.setServer(mqtt_server, 1883);
  mqttclient.setCallback(callback);

  data[0] = '\0';
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WaitConnectWiFi();
  }
  if (!mqttclient.connected()) {
    reconnect();
  }
  if (Serial.available()) {
    data[0] = '\0';
    while (Serial.available()) {
      delay(3);
      char m = Serial.read();
      byte len = strlen(data);
      data[len] = m;
      data[len + 1] = '\0';
    }
    mqttclient.publish("app/token-meter/up", data);
    blink(2);
  }
  mqttclient.loop();
}

void callback(char* topic, byte* message, unsigned int length) {
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");
  char msg[33];
  msg[0] = '\0';
  for (int i = 0; i < length; i++) {
    char c = (char)message[i];
    strncat(msg, &c, 1);
  }
  Serial.print(msg);
}

void blink(int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(150);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
  }
}

void reconnect() {
  while (!mqttclient.connected()) {
    // Serial.println("Attempting MQTT connection...");
    if (mqttclient.connect("meter-mqtt")) {
      blink(5);
      mqttclient.subscribe("app/token-meter/down");
      // Serial.println("connected!");
    } else {
      blink(3);
      // Serial.print("failed, rc=");
      // Serial.print(mqttclient.state());
      // Serial.println(" try again in 3 seconds");
      // Wait 3 seconds before retrying
      delay(3000);
    }
  }
}
