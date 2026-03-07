#include "z85.hpp"

// This implementation is tested against Python's `base64.z85encode` via `test.py` in this repository.

static const uint8_t _z85_tbl[] PROGMEM = "0123456789abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";

void z85_print(Print &out, const uint8_t* buffer, const size_t len) {
  for (size_t pos=0; pos<len; pos+=4) {
    const uint8_t left = pos+4<len ? 4 : len-pos;
    uint32_t n = 0;  // If left<4, treat the rest of the bytes as 0!
    memcpy(&n, &buffer[pos], left);
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    n = __builtin_bswap32(n);  // Arduino is little-endian, Z85 uses big-endian
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#error "Big endian unsupported - I don't have a big-endian system to test on"
#else
#error "Endianness unknown"
#endif
                out.write(pgm_read_byte(_z85_tbl + (n / 52200625) % 85 ));
                out.write(pgm_read_byte(_z85_tbl + (n / 614125  ) % 85 ));
    if (left>1) out.write(pgm_read_byte(_z85_tbl + (n / 7225    ) % 85 ));
    if (left>2) out.write(pgm_read_byte(_z85_tbl + (n / 85      ) % 85 ));
    if (left>3) out.write(pgm_read_byte(_z85_tbl +  n             % 85 ));
  }
}
