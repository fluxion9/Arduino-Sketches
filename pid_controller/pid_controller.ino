#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <FreqCount.h>


Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);


#define servoPin 9
#define set_pot A0
#define tach_pin 5


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
        pinMode(tach_pin, 1);

        lcd.init();
        lcd.backlight();
        FreqCount.begin(1000);
    }
    void measure_speed(void)
    {
        if(FreqCount.available())
        {
            unsigned long count = FreqCount.read();
            speed_rpm = count / 60;
        }
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
        measure_speed();
        display(0);
        if(pot_val > speed_rpm)
        {
            servo_val += 1;
            servo_val = constrain(servo_val, 1000, 2000);
            servo.writeMicroseconds(servo_val);
        }
        if(pot_val < speed_rpm)
        {
            servo_val -= 1;
            servo_val = constrain(servo_val, 1000, 2000);
            servo.writeMicroseconds(servo_val);
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
