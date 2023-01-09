#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);


#define servo_pin 9
#define set_pot A0


struct PID
{
    unsigned long speed_rpm = 0, 
    last_millis = 0, 
    refresh_rate = 1000,
    pot_val = 0,
    servo_val = 0;
    
    void init(void)
    {
        pinMode(set_pot, 0);
        pinMode(tach_pin, 0);

        lcd.init();
        lcd.backlight();
        servo.attach(servo_pin);
        servo.write(180);
        delay(1000);
        servo.write(90);
        delay(3000);
    }
    void read_speed(void)
    {

    }
    void read_pot(void)
    {
        int adc = analogRead(set_pot);
        pot_val = map(adc, 0, 1023, 0, 2000);
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
                lcd.print("Speed: " + String(speed_rpm) + " RPM");
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
            servo_val += 1;
            servo_val = constrain(servo_val, 0, 180);
            servo.write(servo_val);
        }
        if(pot_val < speed_rpm)
        {
            servo_val -= 1;
            servo_val = constrain(servo_val, 0, 180);
            servo.write(servo_val);
        }
        
    }
}pid;

void setup()
{
    pid.init();
}

void loop()
{
    pid.run();
}
