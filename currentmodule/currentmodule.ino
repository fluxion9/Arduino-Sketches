// #include <Wire.h>
#include "EmonLib.h"
EnergyMonitor emon1;

void setup()
{  
  emon1.current(1, 111.1);
}
void loop()
{
  double Irms = emon1.calcIrms(1480);
}
