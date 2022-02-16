float num = 0.123456;
String val;
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial.println(num);
val = String(num, 3);
Serial.println(val);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
