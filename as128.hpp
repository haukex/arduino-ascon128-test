#pragma once
#include <Arduino.h>

/** This function encrpyts the given buffer using the Ascon-128 algorithm.
 * The IV is used as the "associated data" (unencrypted but checksummed data).
 * The result is written to the given stream with the Z85 encoding. */
void as128_encrypt_print_z85(Print& out, const uint8_t secret[16], const uint8_t iv[16], const uint8_t* buffer, const size_t len);

/** This function decrypts the given buffer using the Ascon-128 algorithm, assuming
 * the buffer was generated with the same structure as ``as128_encrypt_print_z85`` outputs,
 * that is, the "associated data" is exactly the IV and is the first 16 bytes of the buffer,
 * and the tag is the last 16 bytes of the buffer. The ``output`` buffer **MUST** hold at least
 * ``len-32`` bytes, and this is also the size of the resulting output buffer.
 * If decryption fails, returns false. */
bool as128_decrypt(const uint8_t secret[16], uint8_t* buffer, const size_t len, uint8_t* output);
