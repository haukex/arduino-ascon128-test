#include "hex.hpp"

int8_t _hex_nib2int(uint8_t c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

// This implementation is tested as part of the Z85 and Ascon-128 decryption tests in this repository.

bool hex_decode(uint8_t* buffer, const size_t len) {
  if (len%2) return false;
  for (size_t i=0; i<len; i+=2) {
    const int8_t upper = _hex_nib2int(buffer[i]);
    if (upper<0) return false;
    const int8_t lower = _hex_nib2int(buffer[i+1]);
    if (lower<0) return false;
    buffer[i/2] = (upper << 4) | lower;
  }
  return true;
}

// This implementation is tested as part of the Ascon-128 decryption test in this repository.

static const uint8_t _hex_tbl[] = "0123456789abcdef";

void hex_encode(uint8_t* buffer, const size_t len) {
  for (size_t i=len-1; ; i--) {
    uint8_t b = buffer[i];
    buffer[i*2] = _hex_tbl[(b >> 4) & 0xF];
    buffer[i*2+1] = _hex_tbl[b & 0xF];
    if (!i) break;
  }
}
