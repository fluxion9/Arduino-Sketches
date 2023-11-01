#include <Ed25519.h>

byte pvk[32], pbk[32];

Ed25519 Ed25519;

void setup() {
  // put your setup code here, to run once:
  ed.generatePrivateKey(pvk);
  ed.derivePublicKey(pbk, pvk);
}

void loop() {
  // put your main code here, to run repeatedly:

}
