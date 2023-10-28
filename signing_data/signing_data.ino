#include <uECC.h>
#include <Crypto.h>
#include <SHA256.h>

#define HASH_SIZE 32

SHA256 sha256;

#define noiseSource A0

String data = "{\"timestamp\":111222377,\"energy_spent_per_kw_hr\":22.55}";

String Memo = "";

String printBuf = "";

byte pbk[64], pvk[32], cpbk[33], sig[64];

bool Registered = false, DEBUG = false;


extern "C" {

  int RNGen(uint8_t *dest, unsigned size) {
    // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
    // random noise). This can take a long time to generate random data if the result of analogRead(0)
    // doesn't change very frequently.
    while (size) {
      uint8_t val = 0;
      for (unsigned i = 0; i < 8; ++i) {
        int init = analogRead(noiseSource);
        int count = 0;
        while (analogRead(noiseSource) == init) {
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

void generatePrivAndPubKeyPairs(byte* Public_Key, byte* Private_Key, byte* Compressed_Key, int privKeySize, int pubKeySize)
{
  const struct uECC_Curve_t * curve = uECC_secp256k1();
  uECC_make_key(Public_Key, Private_Key, curve);
  uECC_compress(Public_Key, Compressed_Key, curve);
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

int signMessage(String msg, byte* signature)
{
  const struct uECC_Curve_t * curve = uECC_secp256k1();
  uint8_t value[HASH_SIZE];
  int len = msg.length();
  byte data[len];
  for (int i = 0; i < len; i++)
  {
    data[i] = msg[i];
  }
  sha256.reset();
  sha256.update(data, len);
  sha256.finalize(value, sizeof(value));
  int stat = uECC_sign(pvk, value, len, signature, curve);
  return stat;
}

void setup() {
  Serial.begin(9600);

  uECC_set_rng(&RNGen);

  printBuf.reserve(90);
  Memo.reserve(64);

  generatePrivAndPubKeyPairs(pbk, pvk, cpbk, 32, 64);


  Serial.print("Private Key: ");
  byteArrayToHexStr(pvk, sizeof(pvk), &printBuf);
  Serial.println(printBuf);

  Serial.print("Public Key: ");
  byteArrayToHexStr(cpbk, sizeof(cpbk), &printBuf);
  Serial.println(printBuf);

  Serial.print("Data: ");
  Serial.println(data);

  Serial.print("Signature: ");
  signMessage(data, sig);
  byteArrayToHexStr(sig, sizeof(sig), &printBuf);
  Serial.println(printBuf);
}

void loop() {
  // put your main code here, to run repeatedly:
}
