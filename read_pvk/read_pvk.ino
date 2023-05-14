#include <uECC.h>
#include <EEPROM.h>

#define key_addr 0

extern "C" {

  int RNG(uint8_t *dest, unsigned size) {
    // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
    // random noise). This can take a long time to generate random data if the result of analogRead(0)
    // doesn't change very frequently.
    while (size) {
      uint8_t val = 0;
      for (unsigned i = 0; i < 8; ++i) {
        int init = analogRead(0);
        int count = 0;
        while (analogRead(0) == init) {
          ++count;
        }

        if (count == 0) {
          val = (val << 1) | (init & 0x01);
        } else {
          val = (val << 1) | (count & 0x01);
        }
      }
      *dest = val;
      ++dest;
      --size;
    }
    // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
    return 1;
  }
}  // extern "C"

String byteToHexStr(byte input)
{
  String output = "";
  if (input < 16)
  {
    output.concat("0");
  }
  output.concat(String(input, HEX));
  return output;
}

void generatePrivAndPubKeyPairs(byte* Public_Key, byte* Private_Key, byte* Compressed_Key, int privKeySize, int pubKeySize)
{
  const struct uECC_Curve_t * curve = uECC_secp256k1();
  uECC_make_key(Public_Key, Private_Key, curve);
  uECC_compress(Public_Key, Compressed_Key, curve);
}

void byteArrayToHexStr(byte* Array, int arraySize, String* memory)
{
  *memory = "";
  memory->concat("0x");
  for (int i = 1; i < arraySize; i++)
  {
    memory->concat(byteToHexStr(Array[i]));
  }
}

void pvkByteArrayToHexStr(byte* Array, int arraySize, String* memory)
{
  *memory = "";
  memory->concat("0x");
  for (int i = 0; i < arraySize; i++)
  {
    memory->concat(byteToHexStr(Array[i]));
  }
}

bool storePrivateKey(byte* key, int keySize, int address)
{
  if ((keySize + 1) <= (EEPROM.length() - 1 - address))
  {
    EEPROM.write(address, keySize);
    for (int i = 0; i < keySize; i++)
    {
      EEPROM.write(address + 1 + i, key[i]);
    }
    return 1;
  }
  else {
    return 0;
  }
}

void readPrivateKey(int address, byte* keyArray, int keyArraySize)
{
  int keySize = EEPROM.read(address);
  for (int i = 0; i < keySize; i++)
  {
    keyArray[i] = EEPROM.read(address + 1 + i);
  }
}

String public_key = "", private_key = "", compressed_public_key = "";

byte pbk[64], pvk[32], cpbk[33];


String buf = "";

void setup() {
  Serial.begin(9600);
  readPrivateKey(key_addr, pvk, sizeof(pvk));
  pvkByteArrayToHexStr(pvk, sizeof(pvk), &private_key);
  Serial.println(private_key);
}

void loop() {
}
