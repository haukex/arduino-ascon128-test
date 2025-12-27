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

CRYPT_KEY = b'Super Secret! :)'  # must be 16 bytes

CRYPT_TESTS = (
    b'a',
    b'ab',
    b'abc',
    b'abcd',
    b'Hello, World! ab',
    b'Hello, World! This is a test.',
    # spell: disable
    b'Lorem ipsum dolor sit amet, consectetur adipiscing elit sapien.',
    # 126+\n+\0:
    b'Lorem ipsum dolor sit amet, consectetur \xE4dipiscing elit. Pellentesque'
    b' mi magna, pulvinar non \xEEpsum eget, finibus \xE6lquet metus.',
    # spell: enable
)

_prev_iv: int = -1


def do_crypt_test(ser: serial.Serial):
    for t in CRYPT_TESTS:
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
            raise RuntimeError(f"{out=} != {t=}")
        print(f"OK {t.decode('CP1252')} {rx.decode('ASCII')}")


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
                raise RuntimeError(f"{rx=} != {exp=}")
            print(f"OK {t.hex()} {rx.decode('ASCII')}")


def main():
    with serial.Serial(
            port='COM29', baudrate=115200, timeout=5) as ser:
        print("Waiting for boot...")
        if not ser.read_until(b'Ready\r\n').endswith(b'Ready\r\n'):
            raise RuntimeError('Failed to get "Ready" from Arduino')
        do_z85_test(ser)
        do_crypt_test(ser)


if __name__ == '__main__':
    main()
