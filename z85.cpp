#include "z85.hpp"

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
void z85_print(Print &out, const uint8_t* buffer, const size_t len) {
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
