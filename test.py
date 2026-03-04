#!/usr/bin/env python
# pylint: disable=missing-function-docstring
"""Test of Ascon-128 on Arduino."""
from itertools import product
from base64 import z85encode, z85decode
# https://github.com/pyserial/pyserial
import serial  # type: ignore[import-untyped]
# https://github.com/meichlseder/pyascon
import ascon   # type: ignore[import-untyped]

# spell: ignore baudrate


def do_z85_test(ser: serial.Serial):
    for length in range(6):
        for combo in product((0, 1, 128, 255), repeat=length):
            t = bytes(combo)
            # Send
            ser.write(b'z' + t.hex().encode('ASCII') + b'\n')
            # Receive
            rx = ser.readline().rstrip(b'\r\n')
            exp = z85encode(t)
            if rx != exp:
                raise RuntimeError(f"{t=}: {rx=} != {exp=}")
            print(f"OK {t.hex()} {rx.decode('ASCII')}")


CRYPT_KEY = b'Super Secret! :)'  # must be 16 bytes

_prev_iv: int = -1

ENCRYPT_TESTS = (
    b'',
    b'a',
    b'ab',
    b'abc',
    b'abcd',
    b'\x00\x09\x0d\x18\x7F\x8A\xFF',  # can't include \x0a here
    b'Hello, World! ab',  # 16 bytes
    b'Hello, World! This is a test.',
    # spell: disable
    b'Lorem ipsum dolor sit amet, consectetur adipiscing elit sapien.',
    # 126+\n+\0:
    b'Lorem ipsum dolor sit amet, consectetur \xE4dipiscing elit. Pellentesque'
    b' mi magna, pulvinar non \xEEpsum eget, finibus \xE6lquet metus.',
    # spell: enable
)


def do_encrypt_test(ser: serial.Serial):
    for t in ENCRYPT_TESTS:
        # Send
        ser.write(b'c' + t + b'\n')
        # Receive
        rx = ser.readline().rstrip(b'\r\n')
        # Split
        if len(rx) < 40:
            raise ValueError(f"data too short: {rx!r}")
        iv = z85decode(rx[:20])
        got = z85decode(rx[20:-20])
        tag = z85decode(rx[-20:])
        # Check IV
        ivn = int.from_bytes(iv, byteorder='little')
        global _prev_iv  # pylint: disable=global-statement
        if ivn <= _prev_iv:
            raise ValueError(f"IV repeated: {iv!r}")
        _prev_iv = ivn
        # Decrypt
        out = ascon.decrypt(CRYPT_KEY, iv, iv, got+tag)
        if out != t:  # mismatch
            print(f"##### {t!r} #####")
            print(f" {rx=} iv={ivn!r}")
            # Show expected encryption result
            enc = ascon.encrypt(CRYPT_KEY, iv, iv, t)
            exp = z85encode(iv) + z85encode(enc[:-16]) + z85encode(enc[-16:])
            print(f"{exp=}")
            print(f"    {enc[:-16]!r} tag={enc[-16:]!r}")
            print(f"{got=} {tag=}")
            print(f"{out=}")
            raise RuntimeError(f"{t=} != {out=}")
        print(f"OK {t.decode('CP1252')!r} {rx.decode('ASCII')}")


DECRYPT_TESTS = (
    b'',
    b'a',
    b'ab',
    b'abc',
    b'abcd',
    b'\x00\x09\x0d\x0a\x18\x7F\x8A\xAB\xFF',
    b'Hello, World! ab',  # 16 bytes
    b'Hello, World! This is a test.',
    # spell: disable
    # sending hex encoded: 63 bytes * 2 + IV and tag as 64 bytes = 190 bytes
    b'Lorem ipsum dolor sit \xE6met, consectetur \xE4dipiscing el\xEEt '
    b'sapien.',
    # spell: enable
)


def do_decrypt_test(ser: serial.Serial):
    for t in DECRYPT_TESTS:
        global _prev_iv  # pylint: disable=global-statement
        _prev_iv += 1
        iv = _prev_iv.to_bytes(16, byteorder='little')
        enc = iv + ascon.encrypt(CRYPT_KEY, iv, iv, t)
        # Send
        ser.write(b'd' + enc.hex().encode('ASCII') + b'\n')
        # Receive
        rx = ser.readline().rstrip(b'\r\n')
        exp = f"{iv.hex()} {t.hex()}".encode('ASCII')
        if rx != exp:
            raise RuntimeError(f"{t=}: {rx=} != {exp=}")
        print(f"OK {t.decode('CP1252')!r} {enc.hex()}")


def main():
    with serial.Serial(
            port='COM29', baudrate=115200, timeout=5) as ser:
        print("Waiting for boot...")
        if not ser.read_until(b'Ready\r\n').endswith(b'Ready\r\n'):
            raise RuntimeError('Failed to get "Ready" from Arduino')
        do_z85_test(ser)
        do_encrypt_test(ser)
        do_decrypt_test(ser)


if __name__ == '__main__':
    main()
