#pragma once
#include <Arduino.h>

/** Decode the buffer as a string of hex characters *in place!*
 * The length must be divisible by two.
 * The length of the resulting buffer will be the length divided by two.
 * If decoding the string fails, returns false. */
bool hex_decode(uint8_t* buffer, size_t len);

/** Encode the buffer as a string of hex characters *in place!*
 * **WARNING:** The actual size of the available memory in the buffer *must*
 * be at least ``len*2``, and this is the length of the resulting buffer. */
void hex_encode(uint8_t* buffer, size_t len);
