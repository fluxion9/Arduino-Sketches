#include <Wire.h>
#include <FreqMeasure.h>
String data = "";
struct FREQ_COUNT
{
  float frequency = 0;
  int count = 0;
  double sum = 0;

  void init(void)
  {
    FreqMeasure.begin();
    Wire.begin(0x40);
  }

  void run(void)
  {
    if (FreqMeasure.available()) {
      sum = sum + FreqMeasure.read();
      count = count + 1;
      if (count > 30) {
        frequency = FreqMeasure.countToFrequency(sum / count);
        frequency *= 2;
        data = String(frequency);
        data += ";";
        sum = 0;
        count = 0;
      }
    }
  }

} freq_count;

void requestEvent()
{
  Wire.print(data);
}
void setup()
{
  freq_count.init();
  Wire.onRequest(requestEvent);
}

void loop()
{
  freq_count.run();
}
