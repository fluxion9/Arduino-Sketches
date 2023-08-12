#include <TinyGPSPlus.h>
#include <Wire.h>
TinyGPSPlus gps;

String Lat = "", Lng = "";
String Location = "";

bool temp = false;

void setup()
{
  Serial.begin(9600);
  Wire.begin(35);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}

void loop()
{
  while (Serial.available() > 0) {
    gps.encode(Serial.read());
    if (gps.location.isUpdated()) {
      loadData();
    }
    else {
      if (!temp)
      {
        loadData();
      }
    }
  }
}

void loadData()
{
  Location = "";
  Location.concat("[");
  Location.concat(String(gps.location.lat(), 6));
  Location.concat(",");
  Location.concat(String(gps.location.lng(), 6));
  Location.concat("]");
  Location.concat(";");
}
void receiveEvent(int howMany) {

}

void requestEvent() {
  Wire.print(Location);
}
