
#include <stdlib.h>
#include <stdint.h>

/* ********** ********** ********** ********** Z85 ********** ********** ********** ********** */

static const uint8_t _z85_tbl[] = "0123456789abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";

/** Write a byte array to the specified stream using the Z85 encoding, with the extension
 * that, as opposed to the specification, buffers of any length can be encoded.
 *
 * This implementation is tested against Python's `base64.z85encode` via `test.py` in this repository.
 *
 * References:
 * - https://rfc.zeromq.org/spec/32/
 * - https://github.com/zeromq/rfc/blob/master/src/spec_32.c
 */
void print_z85(Print &out, const uint8_t* buffer, const size_t len) {
  for (size_t pos=0; pos<len; pos+=4) {
    const uint8_t left = pos+4<len ? 4 : len-pos;
    uint32_t n = __builtin_bswap32(*(uint32_t*)&buffer[pos]);  // Arduino is little-endian, Z85 uses big-endian
                out.write(_z85_tbl[(n / 52200625) % 85]);
                out.write(_z85_tbl[(n / 614125  ) % 85]);
    if (left>1) out.write(_z85_tbl[(n / 7225    ) % 85]);
    if (left>2) out.write(_z85_tbl[(n / 85      ) % 85]);
    if (left>3) out.write(_z85_tbl[ n             % 85]);
  }
}

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
  print_z85(Serial, data, len/2);
  Serial.write('\n');
}

/* ********** ********** ********** ********** Ascon-128 ********** ********** ********** ********** */

/* https://rweather.github.io/arduinolibs/classAscon128.html
 * see the `install-deps.sh` script for installation
 */
#include "Ascon128.h"

/* ********** ********** IV ********** ********** */

// Arduino is little-endian so this works:
typedef struct { uint64_t lsb; uint64_t msb; } uint128_t;
typedef union  { uint128_t i; uint8_t b[16]; } uint128buf_t;
/* Arduino quirk: b/c uint128_t is used as a function argument here,
 * I need to include the function prototype after the typedef. */
inline uint128_t& operator++(uint128_t& x);
inline uint128_t& operator++(uint128_t& x) { if (!(++x.lsb)) x.msb++; return x; }

/* ********** ********** crypt_test ********** ********** */

void crypt_test(const uint8_t* buffer, const size_t len) {
  static Ascon128 cipher = Ascon128();
  static uint128buf_t iv = { .i = {0, 0} };
  // note .setKey also resets internal state
  cipher.setKey((const uint8_t*)"Super Secret! :)", 16);  // key size is always 16
  // Note: In theory, could also use `const uint32_t m = millis()` for IV - wraps after ~49.7 days (!)
  cipher.setIV(iv.b, 16);  // IV size is always 16
  cipher.addAuthData(iv.b, 16);
  // IV can be decoded in Python by `int.from_bytes(z85decode(buf[:20]), byteorder='little')`
  print_z85(Serial, iv.b, 16);
  ++iv.i;

  // Encrypt in blocks of 16 bytes because then we can use the same buffer for the tag too.
  const size_t CRYPT_BUF_SZ = 16;
  static uint8_t crypt_buf[CRYPT_BUF_SZ];
  for(size_t pos=0; pos<len; pos+=CRYPT_BUF_SZ) {
    const uint8_t left = pos+CRYPT_BUF_SZ<len ? CRYPT_BUF_SZ : len-pos;
    cipher.encrypt(crypt_buf, &buffer[pos], left);
    print_z85(Serial, crypt_buf, left);
  }
  cipher.computeTag(crypt_buf, 16);  // tag size is always 16
  print_z85(Serial, crypt_buf, 16);
  cipher.clear();
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
      crypt_test(buffer+1, buf_len-1);
      break;
    case 'z':
      z85_test(buffer+1, buf_len-1);
      break;
    default:
      Serial.println("Unrecognized command");
  }

}
