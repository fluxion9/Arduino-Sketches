#include <ESP8266WiFi.h> //Includes the ESP8266 WiFi Library

const char* ssid     = "JoshWireless"; //SSID of the WiFi
const char* password = "Josh2023"; //Password of the WiFi Connection

WiFiServer server(80);
void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  blynk(3);
  client.setTimeout(5000);
  String req = client.readStringUntil('\r');
  // Match the request
  int val;
  if (req.indexOf(F("/btn/1")) != -1) {
    Serial.println("A");
  }
  if (req.indexOf(F("/btn/2")) != -1) {
    Serial.println("a");
  }
  if (req.indexOf(F("/btn/3")) != -1) {
    Serial.println("B");
  }
  if (req.indexOf(F("/btn/4")) != -1) {
    Serial.println("b");
  }
  if (req.indexOf(F("/btn/5")) != -1) {
    Serial.println("C");
  }
  if (req.indexOf(F("/btn/6")) != -1) {
    Serial.println("c");
  }
  while (client.available()) {
    client.read();
  }
  client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n"));
  client.print(F("<body margin=\"0\"><div style = \"width: 500px; height: 800px; background-color: white; top: 50%; left: 50%; position: absolute; margin-left: -250px; margin-top: -400px; border-radius: 2%; border: 2px solid black;\">\n"));
  client.print(F("<h1 style = \"text-align: center;\"> WifiTooth Rev 1.1 </h1>\n"));
  client.print(F("<h3 style = \"text-align: center;\">ON   |  OFF</h3>\n"));
  client.print(F("<div style = \"border-radius: 2%; border: 2px solid black; width: 400px; height: 500px; margin-left: 50px; margin-top: 100px; position: absolute;\" >\n"));

  client.print(F("<p style = \"padding: 20px; text-align: center;\">"));
  client.print(F("<a href=\"http://"));
  client.print(WiFi.softAPIP());
  client.print(F("/btn/1\"><button style = \"width: 100px; \" >btn1</button></a> | "));
  client.print(F("<a href=\"http://"));
  client.print(WiFi.softAPIP());
  client.print(F("/btn/2\"><button style = \"width: 100px; \" >btn2</button></a></p>"));

  client.print(F("<p style = \"padding: 20px; text-align: center;\">"));
  client.print(F("<a href=\"http://"));
  client.print(WiFi.softAPIP());
  client.print(F("/btn/3\"><button style = \"width: 100px; \" >btn3</button></a> | "));
  client.print(F("<a href=\"http://"));
  client.print(WiFi.softAPIP());
  client.print(F("/btn/4\"><button style = \"width: 100px; \" >btn4</button></a></p>"));

  client.print(F("<p style = \"padding: 20px; text-align: center;\">"));
  client.print(F("<a href=\"http://"));
  client.print(WiFi.softAPIP());
  client.print(F("/btn/5\"><button style = \"width: 100px; \" >btn5</button></a> | "));
  client.print(F("<a href=\"http://"));
  client.print(WiFi.softAPIP());
  client.print(F("/btn/6\"><button style = \"width: 100px; \" >btn6</button></a></p>"));
  client.print(F("</div></div></body></html>"));
}

void blynk(byte val)
{
  for (byte i = 0; i < val; i++)
  {
    digitalWrite(LED_BUILTIN, 0);
    delay(100);
    digitalWrite(LED_BUILTIN, 1);
    delay(100);
  }
  digitalWrite(LED_BUILTIN, 0);
}
