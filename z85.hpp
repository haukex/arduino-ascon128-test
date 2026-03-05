#pragma once
#include <Arduino.h>

/** Write a byte array to the specified stream using the Z85 encoding, with the extension
 * that, as opposed to the specification, buffers of any length can be encoded.
 * (This is the same as the ``z85encode`` and ``z85decode`` functions provided by Python's
 * ``base64`` module as of Python 3.13.)
 *
 * References:
 * - https://rfc.zeromq.org/spec/32/
 * - https://github.com/zeromq/rfc/blob/master/src/spec_32.c
 */
void z85_print(Print &out, const uint8_t* buffer, size_t len);
