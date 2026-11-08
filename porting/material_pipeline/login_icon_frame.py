#!/usr/bin/env python3
"""Read an exact original 512-square ten-mip BC1 login-icon texture frame without CUE offset-fixup assumptions.

Parameterized copy of google_frame.py (left untouched: its hash is a provenance input of the pinned Google bundle).
Every size/offset is derived from the original files and cross-checked; unsupported shapes fail closed.
"""
from pathlib import Path
import hashlib,json,struct
SPECS={
 'T_TempIcon_Login_Facebook':dict(package='FortniteGame/Content/UI/Login/Art/T_TempIcon_Login_Facebook',sizes=(734,3310,172032),hashes={'.uasset':'663dee7eb64a456fadee74614cedefc4d110ffa4f8b65a8b0c2c7fe5ed0fd932','.uexp':'112af1d2fa19de568878c40ca8906410fd5a32c4a789cdb9137b24d4002c0394','.ubulk':'533f0fa29c2f4193cdffa078d8c7eb5fb38478603d6a1ec7f4f3df16500fa049'}),
 'T_TempIcon_Login_Ps4':dict(package='FortniteGame/Content/UI/Login/Art/T_TempIcon_Login_Ps4',sizes=(724,3310,172032),hashes={'.uasset':'98413486af4874ba1635043b3d79af3405e6a814c67354e5943e854ace06c02c','.uexp':'a0c028a3b5a6a8c4af07c76b0977c0cef91ebaa32badf521fc97b2190d6aa321','.ubulk':'52cc8384bd2df41016a8165cd192ca4543e00c9f0fe7d3d9fdb8ccf47e395779'}),
 'T_TempIcon_Login_XBox':dict(package='FortniteGame/Content/UI/Login/Art/T_TempIcon_Login_XBox',sizes=(726,3310,172032),hashes={'.uasset':'efee985b5db765883a2faba23fe9dda8e3202d4bd278e32e45a5c85448d1d8ae','.uexp':'55d67c74c18dc179dc4872799f82e88770c9cdd88e4668cff5376c9d39c89514','.ubulk':'f1e78d298a5dd58f00c0c7eda9d3f1c8d879d1febd64b4fdb681d442eab0307c'}),
}
def sha(x):return hashlib.sha256(x).hexdigest()
def frame(header,exports,bulk,cue,name):
 spec=SPECS[name];HASHES=spec['hashes']
 for suffix,data in [('.uasset',header),('.uexp',exports),('.ubulk',bulk)]:
  if sha(data)!=HASHES[suffix]:raise ValueError('original source pair/bulk changed: '+suffix)
 if (len(header),len(exports),len(bulk))!=spec['sizes']:raise ValueError('source file sizes changed')
 if struct.unpack_from('<6i',header)!=(-1641380927,-7,0,0,0,0):raise ValueError('original unversioned summary differs')
 header_size,folder_len=struct.unpack_from('<ii',header,24)
 if header_size!=len(header) or not 1<=folder_len<=128:raise ValueError('header summary differs')
 summary=32+folder_len;flags,count,names_offset=struct.unpack_from('<Iii',header,summary)
 if not flags&0x80000000 or not 1<=count<128:raise ValueError('filtered name map differs')
 names=[];n=names_offset
 for _ in range(count):
  size=struct.unpack_from('<i',header,n)[0];n+=4
  if not 1<=size<=256:raise ValueError('unsupported texture name entry')
  text=header[n:n+size]
  if not text.endswith(b'\0'):raise ValueError('texture name terminator differs')
  names.append(text[:-1].decode());n+=size+4
 start=struct.unpack_from('<q',header,169)[0]
 if start!=len(header)+len(exports)-4:raise ValueError('bulk-data start field differs')
 marker=b'PF_DXT1\0'
 if exports.count(marker)!=1:raise ValueError('ambiguous native platform frame')
 pos=exports.index(marker)
 if struct.unpack_from('<IIIi',exports,pos-16)!=(512,512,1,8):raise ValueError('native platform dimensions differ')
 cursor=pos+8;first,num=struct.unpack_from('<ii',exports,cursor);cursor+=8
 if (first,num)!=(0,10) or len(cue['mips'])!=10:raise ValueError('mip chain differs')
 mips=[];external_end=0
 for level in range(10):
  cooked,flags,elements,size,stored=struct.unpack_from('<iiiiq',exports,cursor);cursor+=24;logical=max(512>>level,1);physical=max(logical,4);expected=(physical//4)**2*8
  if cooked!=1 or elements!=expected or size!=expected or flags!=(0x501 if level<3 else 0x48):raise ValueError('unsupported native mip header')
  if level<3:
   offset=stored+start
   if offset!=external_end or offset<0 or offset+size>len(bulk):raise ValueError('external relative bulk range differs')
   data=bulk[offset:offset+size];external_end=offset+size;storage='ubulk';uexp_offset=None
  else:
   offset=cursor
   if stored!=len(header)+cursor or cursor+size>len(exports):raise ValueError('inline stored offset differs')
   data=exports[cursor:cursor+size];cursor+=size;storage='uexp';uexp_offset=offset
  dimensions=struct.unpack_from('<iii',exports,cursor);cursor+=12
  if dimensions!=(physical,physical,1):raise ValueError('stored physical dimensions differ')
  report=cue['mips'][level]
  if (report['width'],report['height'],report['depth'],report['bulkBytes'])!=(physical,physical,1,size):raise ValueError('CUE mip shape differs')
  # CUE fixes up all offsets by BulkDataStartOffset, including inline ones.
  if report['bulkOffset']!=stored+start:raise ValueError('CUE reported offset does not match identified fixup')
  mips.append(dict(level=level,logical_width=logical,logical_height=logical,stored_width=physical,stored_height=physical,depth=1,cooked=cooked,bulk_flags=flags,element_count=elements,compressed_bytes=size,stored_bulk_offset=stored,cue_reported_bulk_offset=report['bulkOffset'],storage=storage,resolved_file_offset=offset,uexp_offset=uexp_offset,bc1_sha256=sha(data)))
 if external_end!=len(bulk):raise ValueError('external mip chain does not exactly cover ubulk')
 virtual,none_index,none_number,tag=struct.unpack_from('<iiiI',exports,cursor)
 if (virtual,none_number,tag)!=(0,0,0x9e2a83c1) or not 0<=none_index<len(names) or names[none_index]!='None' or cursor+16!=len(exports):raise ValueError('native terminal/sentinel frame differs')
 return dict(source_class='Texture2D',virtual_texture=False,logical_width=512,logical_height=512,pixel_format='PF_DXT1',canonical_decoded_format='RGBA8',first_mip=0,mip_count=10,bulk_data_start_offset=start,bulk_data_start_field_offset=169,external_mips=3,inline_mips=7,mip_receipts=mips,terminal=dict(offset=cursor,virtual_texture=False,none_index=none_index,none_number=none_number,package_tag=hex(tag)),scope='Original signed stored offsets, resolved external/inline ranges and all 10 BC1 payloads. No GPU readback or residency claim.')
if __name__=='__main__':
 import argparse
 ap=argparse.ArgumentParser();ap.add_argument('--texture',choices=sorted(SPECS),required=True);ap.add_argument('--texture-report',type=Path,required=True);a=ap.parse_args()
 source=Path('/mnt/vibrato/fortnite-extracted-full')/SPECS[a.texture]['package'];cue=json.loads(a.texture_report.read_text())
 print(json.dumps(frame(source.with_suffix('.uasset').read_bytes(),source.with_suffix('.uexp').read_bytes(),source.with_suffix('.ubulk').read_bytes(),cue,a.texture),indent=2))
