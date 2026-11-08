#!/usr/bin/env python3
"""Read an exact original cooked PF_B8G8R8A8 eight-mip, all-inline login-art texture frame (T_SkewButtonShape_*).

Both textures are cooked with FirstMipToSerialize=1: the package keeps platform SizeX/SizeY 256x128 but stores only
mips 1..8 (128x64 down to 1x1) and no mip 0. UE 4.25 FTexturePlatformData::SerializeCooked patches SizeX/SizeY from
Mips[0] on load, so the runtime logical size is the first stored mip. Every size/offset below is derived from the
original bytes and cross-checked against the CUE4Parse report; unsupported shapes fail closed.
"""
from pathlib import Path
import hashlib, json, struct
SPECS = {
 'T_SkewButtonShape_B_DF': dict(package='FortniteGame/Content/UI/Login/Art/T_SkewButtonShape_B_DF', sizes=(936, 44301),
   hashes={'.uasset': '798458fef31c3638c1a94826a81e6e4b7068af9ab58f592ea42478ee1b12f516', '.uexp': '7ad32c393c128bda111eaf692705442d718265888b1700708b78e3dbda04e208'}),
 'T_SkewButtonShape_DF': dict(package='FortniteGame/Content/UI/Login/Art/T_SkewButtonShape_DF', sizes=(973, 44327),
   hashes={'.uasset': 'f673d1c2bc9f9e4e7a8b89d2c53462ce00156412cfb36eae5fe1013abc6d55f2', '.uexp': '344a63f7058a7728005a3e74570887a9da59cc4fb2727c1b092dc1af08ebf9b0'}),
}
SERIALIZED = (256, 128)
MIPS = [(128, 64), (64, 32), (32, 16), (16, 8), (8, 4), (4, 2), (2, 1), (1, 1)]
def sha(x): return hashlib.sha256(x).hexdigest()
def frame(header, exports, cue, name):
 spec = SPECS[name]
 for suffix, data in (('.uasset', header), ('.uexp', exports)):
  if sha(data) != spec['hashes'][suffix]: raise ValueError('original source pair changed: ' + suffix)
 if (len(header), len(exports)) != spec['sizes']: raise ValueError('source file sizes changed')
 if struct.unpack_from('<6i', header) != (-1641380927, -7, 0, 0, 0, 0): raise ValueError('original unversioned summary differs')
 start = struct.unpack_from('<q', header, 169)[0]
 if start != len(header) + len(exports) - 4: raise ValueError('bulk-data start field differs')
 marker = b'PF_B8G8R8A8\0'
 if exports.count(marker) != 1: raise ValueError('ambiguous native platform frame')
 pos = exports.index(marker)
 if struct.unpack_from('<IIIi', exports, pos - 16) != (SERIALIZED[0], SERIALIZED[1], 1, len(marker)): raise ValueError('native platform dimensions differ')
 cursor = pos + len(marker)
 first, count = struct.unpack_from('<ii', exports, cursor); cursor += 8
 if (first, count) != (1, 8) or len(cue['mips']) != 8 or cue['firstMip'] != 1: raise ValueError('cooked first-mip/mip chain differs')
 mips = []
 for level, (w, h) in enumerate(MIPS):
  cooked, flags, elements, size, stored = struct.unpack_from('<iiiiq', exports, cursor); cursor += 24
  expected = w * h * 4
  if cooked != 1 or flags != 0x48 or elements != expected or size != expected: raise ValueError('unsupported native mip header')
  if stored != len(header) + cursor or cursor + size > len(exports): raise ValueError('inline stored offset differs')
  data = exports[cursor:cursor + size]; uexp_offset = cursor; cursor += size
  if struct.unpack_from('<iii', exports, cursor) != (w, h, 1): raise ValueError('stored mip dimensions differ')
  cursor += 12
  report = cue['mips'][level]
  if (report['width'], report['height'], report['depth'], report['bulkBytes']) != (w, h, 1, size) or report['bulkOffset'] != stored + start:
   raise ValueError('CUE mip shape or fixed-up offset differs')
  mips.append(dict(level=level, width=w, height=h, depth=1, cooked=cooked, bulk_flags=flags, element_count=elements, bgra_bytes=size,
                   stored_bulk_offset=stored, cue_reported_bulk_offset=report['bulkOffset'], uexp_offset=uexp_offset, bgra_sha256=sha(data)))
 virtual, none_index, none_number, tag = struct.unpack_from('<iiiI', exports, cursor)
 if (virtual, none_number, tag) != (0, 0, 0x9e2a83c1) or cursor + 16 != len(exports): raise ValueError('native terminal/sentinel frame differs')
 first_pixels = bytearray(exports[mips[0]['uexp_offset']:mips[0]['uexp_offset'] + mips[0]['bgra_bytes']])
 for i in range(0, len(first_pixels), 4): first_pixels[i], first_pixels[i + 2] = first_pixels[i + 2], first_pixels[i]
 rgba = Path(cue['rgba']).read_bytes()
 if bytes(first_pixels) != rgba or sha(rgba) != cue['rgbaSha256'] or sha(Path(cue['png']).read_bytes()) != cue['pngSha256']:
  raise ValueError('decoded first-mip pixels differ')
 return dict(source_class='Texture2D', virtual_texture=False, pixel_format='PF_B8G8R8A8', canonical_decoded_format='RGBA8',
   serialized_size_x=SERIALIZED[0], serialized_size_y=SERIALIZED[1], first_mip_serialized=1,
   logical_width=MIPS[0][0], logical_height=MIPS[0][1], first_mip=0, mip_count=8, bulk_data_start_offset=start, bulk_data_start_field_offset=169,
   mip_receipts=mips, rgba_sha256=sha(rgba), terminal=dict(offset=cursor, virtual_texture=False, none_index=none_index, none_number=none_number, package_tag=hex(tag)),
   scope='Original cooked mips 1..8 all inline; runtime logical size is the first stored mip per FTexturePlatformData::SerializeCooked. No GPU readback or residency claim.')
if __name__ == '__main__':
 import argparse
 ap = argparse.ArgumentParser(); ap.add_argument('--texture', choices=sorted(SPECS), required=True); ap.add_argument('--texture-report', type=Path, required=True); a = ap.parse_args()
 source = Path('/mnt/vibrato/fortnite-extracted-full') / SPECS[a.texture]['package']
 print(json.dumps(frame(source.with_suffix('.uasset').read_bytes(), source.with_suffix('.uexp').read_bytes(), json.loads(a.texture_report.read_text()), a.texture), indent=2))
