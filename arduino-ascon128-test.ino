
/* https://rweather.github.io/arduinolibs/classAscon128.html
 * Install "Crypto by Rhys Weatherley 0.4.0", though unfortunately that doesn't seem to include the following:
 * wget https://github.com/rweather/arduinolibs/raw/refs/heads/master/libraries/CryptoLW/src/Ascon128.cpp
 * wget https://github.com/rweather/arduinolibs/raw/refs/heads/master/libraries/CryptoLW/src/Ascon128.h
 * wget https://github.com/rweather/arduinolibs/raw/refs/heads/master/libraries/CryptoLW/src/Ascon128AVR.cpp
 * (could probably also get the CryptoLW lib from GH and install that;
 * hint `arduino-cli config set library.enable_unsafe_install true`)
 */
#include "Ascon128.h"

const uint8_t z85_tbl[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";
/** Encodes 32 bits into 5 ASCII bytes according to the Z85 specification.
 *
 * References:
 * - https://rfc.zeromq.org/spec/32/
 * - https://github.com/zeromq/rfc/blob/master/src/spec_32.c
 */
void z85_enc(const uint8_t in[4], uint8_t out[5]) {
  uint32_t n = __builtin_bswap32(*(uint32_t*)in);  // Arduino is little-endian, Z85 uses big-endian
  for (uint8_t i=4; i>0; i--) { out[i] = z85_tbl[n % 85]; n /= 85; }
  out[0] = z85_tbl[n % 85];  // save one division by pulling this out of the loop
}

// Arduino is Little-Endian so this works:
typedef struct {
  uint64_t lsb;
  uint64_t msb;
} uint128_t;
typedef union {
  uint128_t i;
  uint8_t b[16];
} uint128buf_t;
/* Arduino quirk: b/c uint128_t is used as a function argument here,
 * I need to include the function prototype after the typedef. */
inline void inc_uint128(uint128_t &x);
inline void inc_uint128(uint128_t &x) { if (!(++x.lsb)) x.msb++; }

/** Prints up to 16 bytes encoded into up to 20 Z85-encoded ASCII characters.
 * `sz` must be 16 or less; if it is less, then it is assumed the rest of the
 * buffer is padded properly, e.g. with NUL bytes. */
void print_buf16_z85(uint8_t buf[16], size_t sz) {
  if (!sz) return;
  const size_t m = sz<16 ? (sz-1)/4+1 : 4;
  static uint8_t out_buf[20];
  for(size_t i=0; i<m; i++)
    z85_enc(&buf[i*4], &out_buf[i*5]);
  Serial.write(out_buf, m*5);
}

const uint8_t KEY[] = "Super Secret! :)";  // must be 16 bytes

void setup() {
  Serial.begin(115200);
}

void loop() {
  while (!Serial.available());  // block until we get activity on the serial port

  const size_t BUF_SZ = 128;
  static uint8_t buffer[BUF_SZ];
  //memset(buffer, 0, BUF_SZ);
  // readBytesUntil does not include the \n in the resulting buffer.
  const size_t buf_len = Serial.readBytesUntil('\n', buffer, BUF_SZ-1);
  buffer[buf_len] = '\0';

  static Ascon128 cipher;
  static uint128buf_t iv = {0,0};
  // note .setKey also resets internal state
  cipher.setKey(KEY, 16);  // key size is always 16
  // Note: In theory, could also use `const uint32_t m = millis()` for IV - wraps after ~50 days (!)
  cipher.setIV(iv.b, 16);  // IV size is always 16
  cipher.addAuthData(iv.b, 16);
  // IV can be decoded in Python by `int.from_bytes(z85decode(buf[:20]), byteorder='little')`
  print_buf16_z85(iv.b, 16);
  inc_uint128(iv.i);

  /* We'll encrypt in blocks of 16 bytes because that just makes it easier in that
   * we can use one function to print the IV, Tag, and encrypted data, and it saves
   * a bit of RAM because we only need one 16 byte and one 20 byte buffer. */
  const size_t CRYPT_BUF_SZ = 16;
  static uint8_t crypt_buf[CRYPT_BUF_SZ];
  const size_t slen = strlen((const char*)buffer);  // is safe because we explicitly NUL-terminate above
  for(size_t pos=0; pos<slen; pos+=CRYPT_BUF_SZ) {
    const uint8_t sz = pos+CRYPT_BUF_SZ<slen ? CRYPT_BUF_SZ : slen-pos;
    /* Pad data with NUL bytes that the receiver can strip - this obviously means that
     * the data buffer shouldn't end on NUL bytes; which the `strlen` above guarantees. */
    if (sz<CRYPT_BUF_SZ) memset(crypt_buf, 0, CRYPT_BUF_SZ);
    cipher.encrypt(crypt_buf, &buffer[pos], sz);
    print_buf16_z85(crypt_buf, sz);
  }
  cipher.computeTag(crypt_buf, 16);  // tag size is always 16
  print_buf16_z85(crypt_buf, 16);
  cipher.clear();

  Serial.println();
}
