#include <stdlib.h>
#include <stdint.h>
#include "hex.hpp"
#include "z85.hpp"
#include "as128.hpp"

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
