#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Getting single-ended readings from AIN0..3");
  ads.begin();
}

void loop(void) 
{
  int16_t adc0, adc1, adc2, adc3;

  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);
  Serial.print("AIN0: "); 
  Serial.println(adc0);
  Serial.print("AIN1: "); 
  Serial.println(adc1);
  Serial.print("AIN2: "); 
  Serial.println(adc2);
  Serial.print("AIN3: "); 
  Serial.println(adc3);
  Serial.println(" ");
  
  delay(1000);
}