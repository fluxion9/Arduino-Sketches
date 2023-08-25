#include <WiFi.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Incubator Controller";
const char* password = "";

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
)rawliteral";

String data_buffer = "", ser_buf = "", input = "";

unsigned long last_millis = 0;

struct SIncubator
{
    void init(void)
    {
        Serial.begin(9600);
        WiFi.softAP(ssid);
        IPAddress IP = WiFi.softAPIP();
        data_buffer.reserve(64);
        ser_buf.reserve(32);
        input.reserve(20);
        
        server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
        request->send_P(200, "text/html", index_html);
        });

        server.on("/set-params", HTTP_GET, [](AsyncWebServerRequest * request) {
        input = "";
        input.concat("[");
        input.concat(request->getParam(0)->value());
        input.concat("]");
        Serial.println(input);
        request->send(200);
        });

        server.on("/get-data", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", data_buffer.c_str()); 
        });
        
        server.begin();
    }

    void run(void)
    {
      if(Serial.available())
      {
        while(Serial.available() > 0)
        {
          delay(3);
          char c = Serial.read();
          ser_buf += c;
        }
      }
      if(ser_buf.length() > 0)
      {
        ser_buf.trim();
        data_buffer = ser_buf;
        ser_buf = "";
      }  
    }

}sIncubator;

void setup()
{
  sIncubator.init();
}

void loop()
{
  sIncubator.run();
}
