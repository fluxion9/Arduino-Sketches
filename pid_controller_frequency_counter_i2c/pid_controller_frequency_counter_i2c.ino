#include <Wire.h>
#include <FreqCount.h>

struct FREQ_COUNT
{
  unsigned long frequency = 0;;

  void init(void)
  {
    FreqCount.begin(1000);
    Wire.begin(0x40);
  }

  void run(void)
  {
    if(FreqCount.available())
    {
        frequency = FreqCount.read();
    }
  }

}freq_count;

void requestEvent()
{
    Wire.write(freq_count.frequency);
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
