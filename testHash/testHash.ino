#include <Crypto.h>
#include <SHA256.h>

#define HASH_SIZE 32

SHA256 sha256;

String Memo = "";

String printBuf = "";

void Hash(String msg)
{
    uint8_t value[HASH_SIZE];
    int len = msg.length();
    byte data[len];
    for(int i = 0; i < len; i++)
    {
      data[i] = msg[i];
    }
    sha256.reset();
    sha256.update(data, len);
    sha256.finalize(value, sizeof(value));
    byteArrayToHexStr(value, sizeof(value), &printBuf);
    Serial.println(printBuf);
}

String byteToHexStr(byte input)
{
  Memo = "";
  if (input < 16)
  {
    Memo.concat("0");
  }
  Memo.concat(String(input, HEX));
  return Memo;
}


void byteArrayToHexStr(byte* Array, int arraySize, String* memory)
{
  *memory = "";
  memory->concat("0x");
  for (int i = 0; i < arraySize; i++)
  {
    memory->concat(byteToHexStr(Array[i]));
  }
}

void setup()
{
  Serial.begin(9600);
  printBuf.reserve(90);
  Memo.reserve(64);
  Hash("Hello there! I'm inflating this hash function");
}

void loop()
{
  
}
