#pragma once
#include <Arduino.h>

#if !defined(__BYTE_ORDER__) || (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
#error "Endianness unknown or not supported"
#endif

// Arduino is little-endian so this struct works when using memcpy to a 16-byte buffer:
typedef struct { uint64_t lsb; uint64_t msb; } uint128_t;

inline uint128_t& operator++(uint128_t& x)  // preincrement
  { if (!(++x.lsb)) x.msb++; return x; } 

// the dummy `int` parameter distinguishes post- from pre-increment
inline uint128_t operator++(uint128_t& x, int)  // postincrement
  { uint128_t old = x; ++x; return old; }

inline bool operator==(const uint128_t& a, const uint128_t& b)
  { return (a.msb == b.msb) && (a.lsb == b.lsb); }

inline bool operator>(const uint128_t& a, const uint128_t& b)
  { return (a.msb > b.msb) || (a.msb == b.msb && a.lsb > b.lsb); }

inline bool operator<(const uint128_t& a, const uint128_t& b)
  { return (a.msb < b.msb) || (a.msb == b.msb && a.lsb < b.lsb); }
