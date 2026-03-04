#pragma once
#include <Arduino.h>

/** Decode the buffer as a string of hex characters *in place*!
 * The length must be divisible by two.
 * The length of the resulting buffer will be the length divided by two.
 * If decoding the string fails, returns false.
 */
bool hex_decode(uint8_t* buffer, const size_t len);
