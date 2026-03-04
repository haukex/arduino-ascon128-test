#include <stdlib.h>
#include <stdint.h>
#include "z85.hpp"
#include "as128.hpp"

/* ********** ********** z85_test ********** ********** */

uint8_t hex_chr(uint8_t c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return 0;
}

void z85_test(const uint8_t* buffer, const size_t len) {
  const size_t BUF_SZ = 16;
  if (len%2 || len>BUF_SZ*2) {
    Serial.println("Bad length");
    return;
  }
  static uint8_t data[BUF_SZ];
  for (size_t i=0; i<len; i+=2)
    data[i/2] = (hex_chr(buffer[i]) << 4) | hex_chr(buffer[i+1]);
  z85_print(Serial, data, len/2);
  Serial.write('\n');
}

/* ********** ********** ********** ********** Ascon-128 ********** ********** ********** ********** */

void as128_test(const uint8_t* buffer, const size_t len) {
  static uint128buf_t iv = { .i = {0, 0} };
  // Note: In theory, could also use `const uint32_t m = millis()` for IV - wraps after ~49.7 days (!)
  as128_print_z85(Serial, (const uint8_t*)"Super Secret! :)", iv.b, buffer, len);
  ++iv.i;
  Serial.write('\n');
}

/* ********** ********** ********** ********** Main ********** ********** ********** ********** */

void setup() {
  Serial.begin(115200);
  Serial.println("Ready");
}

void loop() {
  while (!Serial.available());  // block until we get activity on the serial port

  const size_t BUF_SZ = 128;
  static uint8_t buffer[BUF_SZ];
  // readBytesUntil does not include the \n in the resulting buffer.
  const size_t buf_len = Serial.readBytesUntil('\n', buffer, BUF_SZ-1);
  buffer[buf_len] = '\0';

  switch(buffer[0]) {
    case 'c':
      as128_test(buffer+1, buf_len-1);
      break;
    case 'z':
      z85_test(buffer+1, buf_len-1);
      break;
    default:
      Serial.println("Unrecognized command");
  }

}
