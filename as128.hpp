#pragma once
#include <Arduino.h>

void as128_encrypt_print_z85(Print& out, const uint8_t secret[16], const uint8_t iv[16], const uint8_t* buffer, const size_t len);
