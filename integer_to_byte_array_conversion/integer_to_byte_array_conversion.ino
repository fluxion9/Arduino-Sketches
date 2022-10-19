uint8_t byteArray[4];
uint32_t val = 10968973;
void setup() {
  Serial.begin(115200);
  Serial.print("Num Raw: ");
  Serial.println(val);
  Serial.print("Num Hex: ");
  Serial.println(val, HEX);
  intToByteArray(val);
  for (uint8_t i = 0; i < 4; i++)
  {
    Serial.print(byteArray[i], HEX);
  }
  Serial.println();
  Serial.println(byteArrayToInt(byteArray, 4));
}

void loop() {
  // put your main code here, to run repeatedly:

}

void intToByteArray(uint32_t num)
{
  byteArray[3] = (num << (8 * 3)) >> (8 * 3);
  byteArray[2] = (num << (8 * 2)) >> (8 * 3);
  byteArray[1] = (num << (8 * 1)) >> (8 * 3);
  byteArray[0] = num >> (8 * 3);
}

uint32_t byteArrayToInt(uint8_t inp[], uint8_t s)
{
  uint32_t val = 0;
  uint32_t num[s];
  for (uint8_t i = 0; i < s; i++)
  {
    num[i] = inp[i];
  }
  for (uint8_t i = 0; i < s; i++)
  {
    val = val | (num[i] << (8*(s-1-i)));
  }
  //val = (num[0] << (8*3)) | (num[1] << (8*2)) | (num[2] << (8*1)) | (num[3] << (8*0))
  return val;
}
