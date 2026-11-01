#!/usr/bin/env python3
"""Read exact original Google BC1 frame without CUE offset-fixup assumptions."""
from pathlib import Path
import hashlib,json,struct
PACKAGE='FortniteGame/Content/UI/Login/Art/T_TempIcon_Login_Google'
HASHES={'.uasset':'dd5b879570082e8a9ce2b5422bce4defab5cb4ce33fc1504c8adaf8d05928077','.uexp':'d8ccb6760cb477ccca194f1e33596a6b4f9298fabd8ec7e8d8acb7750c5ea4d4','.ubulk':'7e16cf5d986d0d1fc186102733357143beeac6f401850a0e91acd0ef17d0af72'}
def sha(x):return hashlib.sha256(x).hexdigest()
def frame(header,exports,bulk,cue):
 for suffix,data in [('.uasset',header),('.uexp',exports),('.ubulk',bulk)]:
  if sha(data)!=HASHES[suffix]:raise ValueError('original source pair/bulk changed: '+suffix)
 if (len(header),len(exports),len(bulk))!=(730,3310,172032):raise ValueError('source file sizes changed')
 if struct.unpack_from('<6i',header)!=(-1641380927,-7,0,0,0,0):raise ValueError('original unversioned summary differs')
 header_size,folder_len=struct.unpack_from('<ii',header,24)
 if header_size!=730 or not 1<=folder_len<=128:raise ValueError('header summary differs')
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
 if start!=len(header)+len(exports)-4 or start!=4036:raise ValueError('bulk-data start field differs')
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
 port=Path('/opt/fortnite/FortniteClient-12.60-CL-13477524/porting');source=Path('/mnt/vibrato/fortnite-extracted-full')/PACKAGE;cue=json.loads((port/'generated/material_pipeline/google-texture-1/texture-report.json').read_text());result=frame(source.with_suffix('.uasset').read_bytes(),source.with_suffix('.uexp').read_bytes(),source.with_suffix('.ubulk').read_bytes(),cue);out=port/'generated/material_pipeline/google-native-frame-1.json';assert not out.exists();out.write_text(json.dumps(result,indent=2)+'\n');print(out,sha(out.read_bytes()))
