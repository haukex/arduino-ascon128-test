#pragma once
#include <Arduino.h>

/** This function encrpyts the given buffer using the Ascon-128 algorithm.
 * The IV is used as the "associated data" (unencrypted but checksummed data).
 * The result is directly written to the given stream with the Z85 encoding.
 * This is an alternative to ``as128_encrypt`` for memory-constrained systems. */
void as128_encrypt_print_z85(Print& out, const uint8_t secret[16], const uint8_t iv[16], const uint8_t* buffer, size_t len);
