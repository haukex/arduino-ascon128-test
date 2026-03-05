#pragma once
#include <Arduino.h>

/** This function encrpyts the given buffer using the Ascon-128 algorithm.
 * The IV is used as the "associated data" (unencrypted but checksummed data).
 * The ``output`` buffer **MUST** hold at least ``len+16`` bytes, and this is
 * also the size of the resulting output buffer. Don't forget to tranmit the
 * IV (associated data) to the receiver before transmitting the encrypted output. */
void as128_encrypt(const uint8_t secret[16], const uint8_t iv[16], const uint8_t* buffer, size_t len, uint8_t* output);

/** This function decrypts the given buffer using the Ascon-128 algorithm, assuming the buffer
 * was generated with the following structure: the "associated data" is exactly the IV and is the
 * first 16 bytes of the buffer, and the tag is the last 16 bytes of the buffer. The ``output``
 * buffer **MUST** hold at least ``len-32`` bytes, and this is also the size of the resulting
 * output buffer. If decryption fails, returns false. */
bool as128_decrypt(const uint8_t secret[16], const uint8_t* buffer, size_t len, uint8_t* output);
