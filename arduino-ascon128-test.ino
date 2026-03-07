#include "hex.hpp"
#include "z85.hpp"
#include "as128.hpp"
#include "uint128.hpp"
#include "as128z85.hpp"

/* Analyzing memory usage on AVR:
 * - In the Arduino IDE, Sketch -> Export Compiled Binary
 *   (ELF file gets written to "build" folder in sketch directory)
 * - Locate the avr-* tools:
 *   - Windows: ~/AppData/Local/Arduino15/packages/arduino/tools/avr-gcc/<VERSION>/bin
 *   - Linux: ~/.arduino15/packages/arduino/tools/avr-gcc/<VERSION>/bin/
 * - `avr-size -C <FILE>.elf` - just shows the numbers that the GUI shows too
 * - `avr-nm -S -C -l --format=sysv --size-sort -td <FILE>.elf | grep -E '\.(data|bss|noinit)'`
 * - `avr-objdump -Cwt -j.bss -j.data -j.noinit <FILE>.elf`
 *   - `| perl -wM5.014 -Mvars='$x' -nle '/\.[a-z]+\h+([0-9a-f]+)\h+/i and $x+=hex($1)}{print $x'`
 *
 * (Though both of the latter don't exactly reach the numbers that `avr-size` does.)
 *
 * An analysis of this code reveals that, other than the variables that can be seen throughout the
 * code, two other major contributors to the RAM usage are Serial with 157 bytes and RNG from the
 * Crypto module with 148 bytes - though I haven't yet been able to trace why the latter is needed.
 */

const size_t MAIN_BUF_SZ = 320;

/* ********** ********** Hex & Z85 Test ********** ********** */

void hex_test(const uint8_t* buffer, const size_t len) {
  hex_print(Serial, buffer, len);
  Serial.println();
}

void z85_test(const uint8_t* buffer, const size_t len) {
  z85_print(Serial, buffer, len);
  Serial.println();
}

/* ********** ********** ********** ********** Ascon-128 Tests ********** ********** ********** ********** */

const uint8_t* SECRET = (uint8_t*)"Super Secret! :)";

// Note: In theory, could maybe use `const uint32_t m = millis()` for IV in some cases? Wraps after ~49.7 days!
/** Returns a new IV. */
const uint8_t* next_iv() {
  static uint128_t iv = {0, 0};
  static uint8_t ivb[16];
  memcpy(ivb, &iv, 16);
  iv++;
  return ivb;
}

void as128_enc_z85_test(const uint8_t* buffer, const size_t len) {
  as128_encrypt_print_z85(Serial, SECRET, next_iv(), buffer, len);
  Serial.println();
}

const size_t BUF2_SZ = MAIN_BUF_SZ/2-16;
static uint8_t buf2[BUF2_SZ];

void _write_buf2(const uint8_t iv[16], const size_t len) {
  static uint8_t out_buf[32];
  memcpy(out_buf, iv, 16);  // first the IV
  hex_encode(out_buf, 16);
  Serial.write(out_buf, 32);
  for(size_t pos=0; pos<len; pos+=16) {
    const uint8_t left = pos+16<len ? 16 : len-pos;
    memcpy(out_buf, &buf2[pos], left);
    hex_encode(out_buf, left);
    Serial.write(out_buf, left*2);
  }
  Serial.println();
}

void as128_enc_test(const uint8_t* buffer, const size_t len) {
  if (len+16 > BUF2_SZ) {
    Serial.println(F("Not enough memory?"));
    return;
  }
  const uint8_t* iv_buf = next_iv();
  as128_encrypt(SECRET, iv_buf, buffer, len, buf2);
  _write_buf2(iv_buf, len+16);
}

void as128_dec_test(const uint8_t* buffer, const size_t len) {
  if (len-32 > BUF2_SZ) {
    Serial.println(F("Not enough memory?"));
    return;
  }
  if (!as128_decrypt(SECRET, buffer, len, buf2)) {
    Serial.println(F("Decrypt failed"));
    return;
  }
  _write_buf2(buffer, len-32);
}

/* ********** ********** ********** ********** Main ********** ********** ********** ********** */

void setup() {
  Serial.begin(115200);
  Serial.println(F("Ready"));
}

void loop() {
  while (!Serial.available());  // block until we get activity on the serial port

  static uint8_t buffer[MAIN_BUF_SZ];
  // readBytesUntil does not include the \n in the resulting buffer.
  const size_t buf_len = Serial.readBytesUntil('\n', buffer, MAIN_BUF_SZ-1);
  if (!buf_len) return;
  if (!hex_decode(buffer+1, buf_len-1)) {
    Serial.println(F("Hex decode failed"));
    return;
  }

  switch(buffer[0]) {
    case 'h':
      hex_test(buffer+1, (buf_len-1)/2);
      break;
    case 'z':
      z85_test(buffer+1, (buf_len-1)/2);
      break;
    case 'c':
      as128_enc_z85_test(buffer+1, (buf_len-1)/2);
      break;
    case 'e':
      as128_enc_test(buffer+1, (buf_len-1)/2);
      break;
    case 'd':
      as128_dec_test(buffer+1, (buf_len-1)/2);
      break;
    default:
      Serial.print(F("0x"));
      Serial.print(buffer[0], HEX);
      Serial.println(F(": Unrecognized command"));
  }

}
