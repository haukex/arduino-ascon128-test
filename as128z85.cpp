#include "as128z85.hpp"
#include "z85.hpp"

/** https://rweather.github.io/arduinolibs/classAscon128.html
 * See the `install_deps.py` script in this repo for installation. */
#include <Ascon128.h>

// This implementation is tested against Python's `ascon` via: https://github.com/haukex/arduino-ascon128-test/blob/main/test.py

static Ascon128 cipher;  // uses approx. 60 bytes

void as128_encrypt_print_z85(Print& out, const uint8_t secret[16], const uint8_t iv[16], const uint8_t* buffer, size_t len) {
  cipher.setKey(secret, 16);
  cipher.setIV(iv, 16);
  cipher.addAuthData(iv, 16);
  z85_print(out, iv, 16);
  // Encrypt in blocks of 16 bytes because then we can use the same buffer for the tag too.
  constexpr uint8_t CRYPT_BUF_SZ = 16;
  static uint8_t crypt_buf[CRYPT_BUF_SZ];  // static so it's seen in the Arduino compile output more obviously
  while (len) {
    if (len>CRYPT_BUF_SZ) {
      cipher.encrypt(crypt_buf, buffer, CRYPT_BUF_SZ);
      buffer += CRYPT_BUF_SZ;
      z85_print(out, crypt_buf, CRYPT_BUF_SZ);
      len -= CRYPT_BUF_SZ;
    }
    else {
      cipher.encrypt(crypt_buf, buffer, len);
      z85_print(out, crypt_buf, len);
      break;
    }
  }
  cipher.computeTag(crypt_buf, 16);
  z85_print(out, crypt_buf, 16);
  cipher.clear();
}
