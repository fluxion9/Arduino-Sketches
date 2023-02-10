String inString;

void setup() {
 Serial.begin(9600);
}

void loop() {
  Serial.print("Input: ");
  inString = input();
  Serial.println(inString);
}

String input() {
  String Input;
  while(!Serial.available());
  while(Serial.available() > 0)
  {
    delay(3);
    char c = Serial.read();
    Input += c;
  }
  if(Input.length() > 0)
  {
    Input.trim();
    return Input;
  }
  else {
    return "";
  }
}
