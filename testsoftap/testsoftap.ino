/*
 * This Code will configure ESP8266 in SoftAP mode and allow different devices (Laptop, Mobile, PCs) connect to it
 */
#include <ESP8266WiFi.h> 
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
const char* ap_ssid = "ESP8266"; //Access Point SSID
const char* ap_password= "psw1234"; //Access Point Password
uint8_t max_connections=8;//Maximum Connection Limit for AP
int current_stations=0, new_stations=0;
AsyncWebServer server(80);
void setup()
{
  delay(2000);
  Serial.begin(115200);
  if(WiFi.softAP(ap_ssid,ap_password,1,false,max_connections)==true)
  {
    Serial.println(WiFi.softAPIP());
  }
  else
  {
    Serial.println("could not connect. ");
  }
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", "hello!");
  });
  server.begin();
}
 
void loop()
{
}
