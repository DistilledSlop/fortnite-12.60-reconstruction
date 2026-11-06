"""Emit native quickbar swap support, guarded by original captured code windows."""
import hashlib
from pathlib import Path
HERE = Path(__file__).resolve().parent
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
WINDOWS = ((5402723392, 2800, 'bd478e80a1e3ad67b29e8bd91b243192bf7bc457c724adac3c17d9baeb4bf23d'), (5402587808, 1884, '3a2f91532d65195cdf0c5360111f7c21ebfa1038babd8bf96a42a941f74b0799'), (5402582352, 176, '5fcc4d0883e304604cc1801031a6162f170539ccabeccbe20a1450ad44e92f33'), (5402680064, 352, '588a03a05d37637d6bacd405634e9e7f579d1c344f4466bc30c16e92d29f34e4'), (5402676064, 384, '9821cbd160d4f27702422bb2f179b8516dd46a2817c4e82e61b372142654e3ca'), (5402563104, 384, 'b1c66eaf6c3d40e2cd826f71984d0a9044acc951488fe768415a72d34d2c9461'), (5402586240, 1148, 'e2190d3755f1ce0fcc8804441fe2b8edc6101ed469f5fcc9b797c52cb540e896'), (5402521648, 256, '70a7b3a2a0c89bf18f8d21634d554698a650267f606a889aa00bd3c40483c3ee'), (5402611824, 400, '4e87929133fe92679a2d9127f65ec0b4043ea2e195f26dab4b6267323d62c9ac'), (5402528384, 544, 'b31e3eab0b9ccddeff37053dd9fc7a93e30db9ac1622b42d20713d02906f37b9'))

def sources():
    with IMAGE.open('rb') as stream:
        for address, length, digest in WINDOWS:
            stream.seek(address - 0x140000000)
            if hashlib.sha256(stream.read(length)).hexdigest() != digest:
                raise ValueError('quickbar swap original capture changed: ' + hex(address))
    return {'QuickBarSwapNative.h': (HERE / 'QuickBarSwapNative.h').read_text()}
