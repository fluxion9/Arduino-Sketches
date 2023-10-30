//#include <Crypto.h>
#include <Ed25519.h>

uint8_t privateKey[32];
uint8_t publicKey[32];
uint8_t signature[64];

String printBuf = "";
String Memo = "";

//const char* data = "{\"timestamp\":111222377,\"energy_spent_per_kw_hr\":22.55}";

String data = "{\"timestamp\":111222377,\"energy_spent_per_kw_hr\":22.55}";

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

void setup() {
  Serial.begin(9600);
  
  Serial.print("Private Key: ");
  Ed25519::generatePrivateKey(privateKey);
  byteArrayToHexStr(privateKey, sizeof(privateKey), &printBuf);
  Serial.println(printBuf);

  Serial.print("Public Key: ");
  Ed25519::derivePublicKey(publicKey, privateKey);
  byteArrayToHexStr(publicKey, sizeof(publicKey), &printBuf);
  Serial.println(printBuf);

  Serial.print("Message: ");
  Serial.println(data);

//  Serial.print("Signature: ");
//  Ed25519::sign(signature, privateKey, publicKey, data, strlen(data));
//  byteArrayToHexStr(signature, sizeof(signature), &printBuf);
//  Serial.println(printBuf);

  Serial.print("Signature: ");
  int len = data.length();
  byte arr[len];
  for(int i = 0; i < len; i++)
  {
    arr[i] = data[i];
  }
  Ed25519::sign(signature, privateKey, publicKey, arr, sizeof(arr));
  byteArrayToHexStr(signature, sizeof(signature), &printBuf);
  Serial.println(printBuf);
}

void loop() {
}
