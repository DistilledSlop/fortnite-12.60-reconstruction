"""Guard image evidence and emit startup FName storage for client post-process."""
import hashlib
import struct
from pathlib import Path

HERE = Path(__file__).resolve().parent
MODULE = 'FortniteGame'
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')


def sources():
    windows = (
        (0x141c93b20, 813, 'b4d98de566dfd1c6d9de95cfd52bdcf47dfac9b25b4de3964cf6c0e72f5ce53b'),
        (0x142321a70, 157, '2dc490e4300ee79e8b4218beb795fc833e193fec4cae0c51f3fd2f1f3ac60675'),
        (0x14194f570, 11, '1e9d03071c856d2e9b6555a0c846a6056f8ee60ea88730b25f46023c6f5d7dc5'),
        (0x14029ec50, 25, '08612853c5d611848861d9621210b82fda4f4cc4e988c33afc9c65e8a0c53f94'),
        (0x145f60e48, 58, '4911ccbbb13921c21ce2dc368da867146c3c990cffa59f1cb1e863db6b666d8e'),
    )
    with IMAGE.open('rb') as f:
        for address, size, digest in windows:
            f.seek(address - 0x140000000)
            if hashlib.sha256(f.read(size)).hexdigest() != digest:
                raise ValueError('volume client original image changed at ' + hex(address))
        for address, expected in ((0x146115880 + 0xdb0, 0x1406801e0),
                                  (0x146116ac8 + 0xdb0, 0x14194f570)):
            f.seek(address - 0x140000000)
            if f.read(8) != struct.pack('<Q', expected):
                raise ValueError('volume client virtual slot changed at ' + hex(address))
    return {
        'FortVolumeClientPostProcess.h': (HERE / 'FortVolumeClientPostProcess.h').read_text(),
        'FortVolumeClientPostProcess.cpp': (HERE / 'FortVolumeClientPostProcess.cpp.in').read_text(),
    }
