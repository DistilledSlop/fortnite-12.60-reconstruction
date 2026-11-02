#!/usr/bin/env python3
"""Bounded provenance checks against the original runtime mapping."""
import os
import struct

IMAGE = "/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin"
BASE = 0x140000000


def main():
    fd = os.open(IMAGE, os.O_RDONLY)
    try:
        def read(address, size):
            data = os.pread(fd, size, address - BASE)
            if len(data) != size:
                raise AssertionError(f"short image read at {address:x}")
            return data

        for table in (0x146FC7BF0, 0x146FC80C8):
            for slot, target in ((0x260, 0x14516C7C0), (0x2A8, 0x145169F10)):
                assert struct.unpack('<Q', read(table + slot, 8))[0] == target
        for address, expected in (
            (0x1459ECEE0, b'SBox\0'),
            (0x1459ECEE8, b'SOverlay\0'),
            (0x1459F7328, b'SScaleBox\0'),
            (0x1407936E3, bytes.fromhex('89b7d0010000')),  # scale stretch default None
            (0x140792041, bytes.fromhex('4889b7a80100004889b7b0010000')),  # zero padding
            (0x14516A614, bytes.fromhex('49c786b001000000000000')),  # scale alignments Fill
            (0x14516A791, bytes.fromhex('4c89b0a0010000')),  # box alignments Fill
            (0x145169FD1, bytes.fromhex('488d86e8020000')),
            (0x14516A6DD, bytes.fromhex('498d87c8020000')),
            (0x14516A825, bytes.fromhex('498d97b8020000')),
            (0x14516C7D7, bytes.fromhex('488d87e8020000')),
            (0x14516C81A, bytes.fromhex('488d87d8020000')),
            (0x14516C85D, bytes.fromhex('488d87b8020000')),
            (0x14516C8A0, bytes.fromhex('488d87c8020000')),
        ):
            assert read(address, len(expected)) == expected, hex(address)
        for address, name in (
            (0x146E430E8, 'FortUI.EnableLetoBorder'),
            (0x146E43118, 'FortUI.LetoBorderWidth'),
            (0x146E43150, 'FortUI.LetoBorderedContentScale'),
        ):
            encoded = (name + '\0').encode('utf-16-le')
            assert read(address, len(encoded)) == encoded
        for instruction, target in (
            (0x1405FFBE6, 0x147E1B7D0),
            (0x1405FFD36, 0x147E1B7D4),
            (0x1405FFDA6, 0x147E1B7D8),
        ):
            lea = read(instruction, 7)
            assert lea[:3] == bytes.fromhex('4c8d05')
            assert instruction + 7 + struct.unpack('<i', lea[3:])[0] == target
        print('root Slate virtual slots, instructions and console storage verified')
    finally:
        os.close(fd)


if __name__ == '__main__':
    main()
