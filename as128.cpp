#include "as128.hpp"
#include "z85.hpp"

// These implementations are tested against Python's `ascon` via `test.py` in this repository.

/* https://rweather.github.io/arduinolibs/classAscon128.html
 * see the `install_deps.py` script in this repo for installation */
#include <Ascon128.h>

static Ascon128 cipher;

void as128_encrypt_print_z85(Print& out, const uint8_t secret[16], const uint8_t iv[16], const uint8_t* buffer, const size_t len) {
  // note .setKey also resets internal state
  cipher.setKey(secret, 16);  // key size is always 16
  cipher.setIV(iv, 16);       // IV size is always 16
  cipher.addAuthData(iv, 16);
  z85_print(out, iv, 16);
  // Encrypt in blocks of 16 bytes because then we can use the same buffer for the tag too.
  const size_t CRYPT_BUF_SZ = 16;
  static uint8_t crypt_buf[CRYPT_BUF_SZ];
  for(size_t pos=0; pos<len; pos+=CRYPT_BUF_SZ) {
    const uint8_t left = pos+CRYPT_BUF_SZ<len ? CRYPT_BUF_SZ : len-pos;
    cipher.encrypt(crypt_buf, &buffer[pos], left);
    z85_print(out, crypt_buf, left);
  }
  cipher.computeTag(crypt_buf, 16);  // tag size is always 16
  z85_print(out, crypt_buf, 16);
  cipher.clear();
}

bool as128_decrypt(const uint8_t secret[16], uint8_t* buffer, const size_t len, uint8_t* output) {
  if (len<32) return false;
  cipher.setKey(secret, 16);  // key size is always 16
  cipher.setIV(buffer, 16);   // IV size is always 16
  cipher.addAuthData(buffer, 16);  // assume buffer starts with IV as additional data
  cipher.decrypt(output, buffer+16, len-32);
  return cipher.checkTag(buffer+len-16, 16);  // assume buffer ends with tag
}
