#include <EEPROM.h>

struct MeterParams {
  uint8_t usage[12];
  int pointer = -1;
  char Token[12][12];
  float energy = 0.00;
  uint8_t lastStamp = 0;
  uint8_t use = 0;
};

// struct MeterParams
// {
//   uint8_t usage[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
//   int pointer = -1;
//   char Token[12][12] = {
//     "012345678989",
//     "246289742379",
//     "237879802782",
//     "278337837683",
//     "412145234125",
//     "782766636626",
//     "238383833472",
//     "092937303098",
//     "144212454153",
//     "747326523567",
//     "293400238263",
//     "233836237273"
//   };
//   float energy = 0.00;
//   uint8_t lastStamp = 0;
//   uint8_t use = 0;
// }d;

void writeToEEPROM(int address, const MeterParams& data) {
  EEPROM.put(address, data);
}

void readFromEEPROM(int address, MeterParams& data) {
  EEPROM.get(address, data);
}

MeterParams mParams;

void setup() {
  Serial.begin(115200);
  if(EEPROM.read(0) == 0)
  {
    Serial.println("EEPROM empty, loading data...");
    // writeToEEPROM(1, d);
    // EEPROM.write(0, 1);
    Serial.println("Done!");
  }
  else {
    Serial.println("EEPROM not empty, reading data...");
    readFromEEPROM(1, mParams);
    Serial.println(mParams.Token[0]);
    Serial.println("Done!");
  }
}

void loop() {
}
