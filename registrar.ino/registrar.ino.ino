#include <uECC.h>

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
  for (int i = 0; i < arraySize; i++)
  {
    memory->concat(byteToHexStr(Array[i]));
  }
}

String public_key = "", private_key = "", compressed_public_key = "";

byte pbk[64], pvk[32], cpbk[33];

void setup() {
  Serial.begin(115200);
  uECC_set_rng(&RNG);
  generatePrivAndPubKeyPairs(pbk, pvk, cpbk, 32, 64);
  byteArrayToHexStr(pbk, sizeof(pbk), &public_key);
  byteArrayToHexStr(pvk, sizeof(pvk), &private_key);
  byteArrayToHexStr(cpbk, sizeof(cpbk), &compressed_public_key);
  Serial.print("private key: ");
  Serial.println(private_key);
  Serial.println(private_key.length());
  Serial.print("public key: ");
  Serial.println(public_key);
  Serial.println(public_key.length());
  Serial.print("compressed public key: ");
  Serial.println(compressed_public_key);
  Serial.println(compressed_public_key.length());
}

void loop() {
}
