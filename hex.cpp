#include "hex.hpp"

int8_t _hex_nib2int(uint8_t c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

// This implementation is tested as part of the Z85 test in this repository.

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
