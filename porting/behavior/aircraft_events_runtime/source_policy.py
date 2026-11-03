"""Emit recovered process-lifetime timing storage without inventing a console identity."""
from pathlib import Path
import os
import struct

HERE = Path(__file__).resolve().parent
IMAGE = Path("/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin")

def sources():
    fd = os.open(IMAGE, os.O_RDONLY)
    try:
        for address, value in ((0x1478c2360, 0.33), (0x1478c2400, 1.0)):
            if os.pread(fd, 4, address - 0x140000000) != struct.pack("<f", value):
                raise ValueError("Original aircraft timing initializer changed: " + hex(address))
    finally:
        os.close(fd)
    return {"FortAircraftTimingRuntime.h": (HERE / "FortAircraftTimingRuntime.h").read_text(),
            "FortAircraftTimingRuntime.cpp": (HERE / "FortAircraftTimingRuntime.cpp.in").read_text()}
