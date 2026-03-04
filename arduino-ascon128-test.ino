#include <stdlib.h>
#include <stdint.h>
#include "hex.hpp"
#include "z85.hpp"
#include "uint128.hpp"
#include "as128.hpp"

const size_t MAIN_BUF_SZ = 256;

/* ********** ********** z85_test ********** ********** */

void z85_test(uint8_t* buffer, const size_t len) {
  if (!hex_decode(buffer, len)) {
    Serial.println("Hex decode failed");
    return;
  }
  z85_print(Serial, buffer, len/2);
  Serial.write('\n');
}

/* ********** ********** ********** ********** Ascon-128 ********** ********** ********** ********** */

const uint8_t* secret = (uint8_t*)"Super Secret! :)";

void as128_enc_test(const uint8_t* buffer, const size_t len) {
  static uint128_t iv = {0, 0};
  static uint8_t iv_buf[16];
  memcpy(iv_buf, &iv, 16);
  iv++;
  // Note: In theory, could also use `const uint32_t m = millis()` for IV - wraps after ~49.7 days (!)
  as128_encrypt_print_z85(Serial, secret, iv_buf, buffer, len);
  Serial.write('\n');
  // IV can be decoded in Python by `int.from_bytes(z85decode(buf[:20]), byteorder='little')`
}

void as128_dec_test(uint8_t* buffer, const size_t len) {
  if (!hex_decode(buffer, len)) {
    Serial.println("Hex decode failed");
    return;
  }
  const size_t BUF_SZ = MAIN_BUF_SZ/2-32;
  static uint8_t plain[BUF_SZ];
  if (!as128_decrypt(secret, buffer, len/2, plain)) {
    Serial.println("Decrypt failed");
    return;
  }
  // write results out as hex
  static uint8_t out_buf[32];
  memcpy(out_buf, buffer, 16);  // first the IV
  hex_encode(out_buf, 16);
  Serial.write(out_buf, 32);
  Serial.print(' ');
  // then the plaintext
  const size_t p_len = len/2 - 32;  // length of plaintext
  for(size_t pos=0; pos<p_len; pos+=16) {
    const uint8_t left = pos+16<p_len ? 16 : p_len-pos;
    memcpy(out_buf, &plain[pos], left);
    hex_encode(out_buf, left);
    Serial.write(out_buf, left*2);
  }
  Serial.println();
}

/* ********** ********** ********** ********** Main ********** ********** ********** ********** */

void setup() {
  Serial.begin(115200);
  Serial.println("Ready");
}

void loop() {
  while (!Serial.available());  // block until we get activity on the serial port

  static uint8_t buffer[MAIN_BUF_SZ];
  // readBytesUntil does not include the \n in the resulting buffer.
  const size_t buf_len = Serial.readBytesUntil('\n', buffer, MAIN_BUF_SZ-1);
  if (!buf_len) return;
  buffer[buf_len] = '\0';

  switch(buffer[0]) {
    case 'z':
      z85_test(buffer+1, buf_len-1);
      break;
    case 'c':
      as128_enc_test(buffer+1, buf_len-1);
      break;
    case 'd':
      as128_dec_test(buffer+1, buf_len-1);
      break;
    default:
      Serial.print("0x");
      Serial.print(buffer[0], HEX);
      Serial.println(": Unrecognized command");
  }

}
