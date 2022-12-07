#include <Stepper.h>

#define led_red 11
#define home_switch 4
#define end_switch 6
#define stepper_pin_A 7
#define stepper_pin_B 8
#define stepper_pin_C 9
#define stepper_pin_D 10

#define steps_per_rev 200

#define home 100
#define end 200
#define midway 150

Stepper stepper (steps_per_rev, stepper_pin_A, stepper_pin_B, stepper_pin_C, stepper_pin_D);

struct Wiper
{
  int check_position(void)
  {
    if (!digitalRead(home_switch))
    {
      return home;
    }
    else if (!digitalRead(end_switch))
    {
      return end;
    }
    else
    {
      return midway;
    }
  }
  void blink(byte pin, int times)
  {
    for (int i = 0; i < times; ++i)
    {
      digitalWrite(pin, 1);
      delay(300);
      digitalWrite(pin, 0);
      delay(300);
    }
  }
  void init()
  {
    pinMode(home_switch, 2);
    pinMode(end_switch, 2);
    pinMode(led_red, 1);


    blink(led_red, 3);
    stepper.setSpeed(20);

    if(check_position() != home)
    {
      return_home();
    }
  }
  void return_home(void)
  {
    while (digitalRead(home_switch))
    {
      stepper.step(-steps_per_rev);
    }
  }
  void goto_end(void)
  {
    while (digitalRead(end_switch))
    {
      stepper.step(steps_per_rev);
    }
  }
  void do_routine()
  {
    blink(led_red, 3);
    goto_end();
    delay(500);
    blink(led_red, 3);
    return_home();
  }
}wiper;

void setup() {
  wiper.init();
  wiper.do_routine();
}

void loop() {
}
