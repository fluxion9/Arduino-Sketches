#define relay_1 5
#define relay_2 6
#define relay_3 7

#define led_red
#define led_yellow
#define led_blue

#define AC_1 A5
#define AC_2 A4
#define AC_3 A3

struct AC_Selector
{
  float phase_voltages[3];

  byte relays[3] = {relay_1, relay_2, relay_3};

  uint32_t last_millis, refresh_rate = 1000;
  
  void init(void)
  {

  }
  void run(void)
  {

  }
  void display(byte slide)
  {

  }
  float measure_voltage_AC(byte pin)
  {

  }
  void read_phase_voltages()
  {
    
  }
};

void setup() {
}

void loop() {
}
