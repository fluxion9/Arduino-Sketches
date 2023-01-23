#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

String buf = "";

#define servo_pin 9
#define set_pot A0


struct PID
{
    unsigned long speed_rpm = 0, 
    last_millis = 0,
    last_read = 0,
    refresh_rate = 500,
    pot_val = 0,
    freq = 0;
    int servo_val = 0;
    
    void init(void)
    {
        pinMode(set_pot, 0);
        Wire.begin();

        lcd.init();
        lcd.backlight();
        servo.attach(servo_pin);
        servo.write(0);
        delay(5000);
    }
    void read_speed(void)
    {
      if((millis() - last_read) >= 100)
      {
        buf = "";
        Wire.requestFrom(0x40, 10);
        while(Wire.available())
        {
          char c = Wire.read();
          buf.concat(c);
        }
        buf = buf.substring(0, buf.indexOf(';'));
        freq = buf.toInt();
        speed_rpm = freq;
        last_read = millis();
      }
    }
    void read_pot(void)
    {
        int adc = analogRead(set_pot);
        pot_val = map(adc, 0, 1023, 0, 180);
    }
    void display(byte slide)
    {
        if(millis() - last_millis >= refresh_rate)
        {
            if(slide == 0)
            {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Set Speed: " + String(pot_val));
                lcd.setCursor(0, 1);
                lcd.print("Speed: " + String(speed_rpm) + " RPS");
            }
            last_millis = millis();
        }
    }
    void run(void)
    {
        read_pot();
        read_speed();
        display(0);
        if(pot_val > speed_rpm)
        {
            servo_val++;
            servo_val = constrain(servo_val, 0, 150);
            servo.write(servo_val);
            delay(15);
        }
        if(pot_val < speed_rpm)
        {
            servo_val--;
            servo_val = constrain(servo_val, 0, 150);
            servo.write(servo_val);
            delay(15);
        }
    }

    void test(void)
    {
      read_pot();
      read_speed();
      display(0);
      servo.write(pot_val);
    }
    
}pid;

void setup()
{
    pid.init();
}

void loop()
{
  // pid.test();
  pid.run();
}
