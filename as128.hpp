#pragma once
#include <Arduino.h>

// Arduino is little-endian so this works:
typedef struct { uint64_t lsb; uint64_t msb; } uint128_t;
typedef union  { uint128_t i; uint8_t b[16]; } uint128buf_t;
inline uint128_t& operator++(uint128_t& x);  // preincrement
inline uint128_t& operator++(uint128_t& x) { if (!(++x.lsb)) x.msb++; return x; }
// IV can be decoded in Python by `int.from_bytes(z85decode(buf[:20]), byteorder='little')`

void as128_print_z85(Print& out, const uint8_t secret[16], const uint8_t iv[16], const uint8_t* buffer, const size_t len);
