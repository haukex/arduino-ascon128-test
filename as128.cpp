#include "as128.hpp"

/** https://rweather.github.io/arduinolibs/classAscon128.html
 * See the `install_deps.py` script in this repo for installation. */
#include <Ascon128.h>

// These implementations are tested against Python's `ascon` via `test.py` in this repository.

static Ascon128 cipher;  // uses approx. 60 bytes
// https://github.com/rweather/arduinolibs/blob/37a76b8f/libraries/CryptoLW/src/Ascon128.h#L52

void as128_encrypt(const uint8_t secret[16], const uint8_t iv[16], const uint8_t* buffer, const size_t len, uint8_t* output) {
  // note .setKey also resets internal state
  cipher.setKey(secret, 16);  // key size is always 16
  cipher.setIV(iv, 16);       // IV size is always 16
  cipher.addAuthData(iv, 16);
  cipher.encrypt(output, buffer, len);
  cipher.computeTag(output+len, 16);  // tag size is always 16
  cipher.clear();
}

bool as128_decrypt(const uint8_t secret[16], const uint8_t* buffer, const size_t len, uint8_t* output) {
  if (len<32) return false;
  cipher.setKey(secret, 16);
  cipher.setIV(buffer, 16);
  cipher.addAuthData(buffer, 16);  // buffer starts with IV as additional data
  cipher.decrypt(output, buffer+16, len-32);
  const bool rv = cipher.checkTag(buffer+len-16, 16);  // buffer ends with tag
  cipher.clear();
  return rv;
}
