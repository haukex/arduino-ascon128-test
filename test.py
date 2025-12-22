#!/usr/bin/env python
"""Test of Ascon-128 on Arduino."""
import time
from base64 import z85decode
# https://github.com/pyserial/pyserial
import serial  # type: ignore[import-untyped]
# https://github.com/meichlseder/pyascon
import ascon   # type: ignore[import-untyped]

# spell: ignore baudrate

KEY = b'Super Secret! :)'  # must be 16 bytes

TESTS = (
    b'a',
    b'ab',
    b'abc',
    b'abcd',
    b'Hello, World! ab',
    b'Hello, World! This is a test.',
    # spell: disable
    b'Lorem ipsum dolor sit amet, consectetur adipiscing elit sapien.',
    b'Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque mi'
    b' magna, pulvinar non ipsum eget, finibus aliquet metus.',
    # spell: enable
)

with serial.Serial(port='/dev/ttyACM0', baudrate=115200, timeout=5) as ser:
    print("Waiting for boot...")
    time.sleep(2)  # not sure why this is required but it is (Arduino resets?)
    seen_iv: set[bytes] = set()
    for t in TESTS:
        print(f"##### {t=} #####")
        ser.write(t)
        ser.write(b'\n')

        buf = ser.readline().rstrip(b'\r\n')
        print(f" {buf=}")

        iv = z85decode(buf[:20])
        print(f"  {iv=} {int.from_bytes(iv, byteorder='little')}")
        if iv in seen_iv:
            raise RuntimeError("nonce seen twice")
        seen_iv.add(iv)

        # Note that the sender guarantees that the data doesn't contain
        # NUL bytes, and pads the data to multiples of 4 using NUL bytes
        # to fulfill the Z85 specification.
        data = z85decode(buf[20:-20])
        tag = z85decode(buf[-20:])
        print(f"{data=} {tag=}")

        enc = ascon.encrypt(KEY, iv, iv, t)
        exp = enc[:-16]
        exp_tag = enc[-16:]
        print(f" {exp=} tag={exp_tag!r}")

        out = ascon.decrypt(KEY, iv, iv, data.rstrip(b'\0')+tag)
        print(f" {out=}")
        if out != t:
            raise RuntimeError(f"{out=} != {t=}")
        print("OK")
