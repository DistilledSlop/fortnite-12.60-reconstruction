"""Emit the partial actual pending-save request support type."""
import hashlib
from pathlib import Path

HERE = Path(__file__).resolve().parent
MODULE = 'FortniteGame'
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')


def sources():
    # Membership read and request allocation/initialization/copy windows.
    windows = (
        (0x14254c270, 148, '3c05e50696cfa657024c39c5af5b920f04ed2bf68cc60ffd8ea0e88a15d52f4c'),
        (0x1425360c0, 1141, 'dc38e8a2246e41b0f0de6c852506ce32b6feda0d168b088d0bb3547ccff24b60'),
    )
    with IMAGE.open('rb') as stream:
        for address, length, digest in windows:
            stream.seek(address - 0x140000000)
            if hashlib.sha256(stream.read(length)).hexdigest() != digest:
                raise ValueError('pending save original capture changed: ' + hex(address))
    return {'FortPendingSaveRequest.h': (HERE / 'FortPendingSaveRequest.h').read_text()}
