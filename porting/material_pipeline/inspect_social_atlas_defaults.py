#!/usr/bin/env python3
"""Validate pinned original atlas constructor proof and decode this asset's tags."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

BASE=0x140000000
IMAGE='604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d'
PROOF='787cd43f8289a5ddad33905c4ef9e8c22bad628f6d1a0ee690a45d26fd0b6815'
ASSET='39d404e1c6b2af2cd07a4786890f47b7c7fa72757ffbad3ef65cb7e517972e23'
EXPORT='45a48f299e3cf56282695ae000376f3105dbaaf8ab9cf83d829a97a73bc21037'

def sha(data):return hashlib.sha256(data).hexdigest()
def read(path, expected=None):
 raw=path.read_bytes()
 if expected is not None and sha(raw)!=expected:raise ValueError('unexpected source hash: '+str(path))
 return raw

def unpack(fmt,data,pos,end):
 if pos<0 or pos+struct.calcsize(fmt)>min(len(data),end):raise ValueError('read exceeds package boundary')
 return struct.unpack_from(fmt,data,pos)

def decode_tags(header,payload):
 if unpack('<6i',header,0,len(header))!=(-1641380927,-7,0,0,0,0):raise ValueError('unexpected package summary')
 hlen,folder=unpack('<ii',header,24,len(header))
 if hlen!=len(header) or folder!=5 or header[32:37]!=b'None\0':raise ValueError('unexpected header/folder')
 summary=37;flags,count,offset=unpack('<Iii',header,summary,len(header))
 if flags!=0x80000000 or count!=43 or offset!=193:raise ValueError('unexpected name map')
 names=[];cursor=offset
 for _ in range(count):
  n,=unpack('<i',header,cursor,len(header));cursor+=4
  if n==0 or abs(n)>1024:raise ValueError('bad name length')
  size=n if n>0 else -2*n
  raw,=unpack(f'<{size}s',header,cursor,len(header));cursor+=size
  unpack('<I',header,cursor,len(header));cursor+=4
  txt=raw.decode('utf8' if n>0 else 'utf-16le')
  if not txt.endswith('\0'):raise ValueError('unterminated name')
  names.append(txt[:-1])
 exp_count,exp_off=unpack('<ii',header,summary+20,len(header));depends,=unpack('<i',header,summary+36,len(header))
 if exp_count!=1 or depends-exp_off!=104:raise ValueError('unexpected export table')
 class_index,=unpack('<i',header,exp_off,len(header));name,num=unpack('<ii',header,exp_off+16,len(header));size,offset=unpack('<qq',header,exp_off+28,len(header))
 if class_index!=-1 or names[name]!='CA_SocialPanel' or num or offset!=hlen or size!=8545 or size>len(payload):raise ValueError('wrong atlas export')
 # Import 0 is the class reference used by the single export.
 imports,import_off=unpack('<ii',header,summary+28,len(header))
 if imports!=25:raise ValueError('unexpected import count')
 imp_name,imp_num=unpack('<ii',header,import_off+20,len(header))
 if imp_num or names[imp_name]!='CurveLinearColorAtlas':raise ValueError('wrong atlas class import')
 def fname(pos):
  i,num=unpack('<ii',payload,pos,size)
  if not 0<=i<len(names) or num:raise ValueError('bad property FName')
  return names[i]
 pos=0;rows=[]
 while True:
  start=pos;name=fname(pos);pos+=8
  if name=='None':break
  kind=fname(pos);pos+=8
  length,index=unpack('<ii',payload,pos,size);pos+=8
  if length<0 or index:raise ValueError('bad tag length/index')
  extra=None
  if kind=='StructProperty':
   extra=fname(pos);pos+=8;unpack('<16s',payload,pos,size);pos+=16
  elif kind=='ArrayProperty':extra=fname(pos);pos+=8
  elif kind!='UInt32Property':raise ValueError('unexpected tag type: '+kind)
  guid,=unpack('<B',payload,pos,size);pos+=1
  if guid!=0 or pos+length>size:raise ValueError('unexpected tag GUID or payload extent')
  rows.append(dict(name=name,kind=kind,extra=extra,tag_start=start,value_start=pos,value_size=length,tag_end=pos+length,raw_hex=payload[start:pos+length].hex()))
  pos+=length
 if [(x['name'],x['kind'],x['extra'],x['value_size']) for x in rows]!=[('TextureSize','UInt32Property',None,4),('GradientCurves','ArrayProperty','ObjectProperty',48),('ImportedSize','StructProperty','IntPoint',8),('LightingGuid','StructProperty','Guid',16)]:raise ValueError('unexpected atlas tag sequence')
 if unpack('<I',payload,rows[0]['value_start'],size)!=(32,) or unpack('<i',payload,rows[1]['value_start'],size)!=(11,):raise ValueError('unexpected source values')
 absent=['SRGB','Filter','AddressX','AddressY','CompressionSettings','LODGroup']
 if set(absent)&{r['name'] for r in rows}:raise ValueError('texture property explicitly serialized')
 return dict(header_size=hlen,name_map_offset=193,name_map=names,name_map_end=cursor,export_table_offset=exp_off,export_table_hex=header[exp_off:exp_off+104].hex(),class_import_index=class_index,import_table_offset=import_off,export_offset=offset,export_size=size,tag_end=pos,tags=rows,terminator_raw_hex=payload[pos-8:pos].hex(),absent_fields=absent,full_tag_stream_hex=payload[:pos].hex())

def verify_proof(proof_path,image):
 proof=json.loads(read(proof_path,PROOF));root=proof_path.parent
 if proof['schema']!='social-atlas-defaults-proof-1' or proof['runtime_claim'] is not False or proof['image']['sha256']!=IMAGE:raise ValueError('proof identity differs')
 if sha(image)!=IMAGE:raise ValueError('wrong original memory image')
 for k in ('inputs','outputs'):
  for row in proof[k]:
   path=Path(row.get('snapshot',row.get('path')));path=path if path.is_absolute() else root/path
   read(path,row['sha256'])
 for row in proof['raw_windows']:
  va=int(row['va'],16);n=row['size'];window=read(root/row['path'],row['sha256'])
  if len(window)!=n or image[va-BASE:va-BASE+n]!=window:raise ValueError('original memory window differs')
 constructor=image[0x1441589a0-BASE:0x1441589f9-BASE]
 for sequence in ['e852a04a00','806365fe','66c7435c0601','66c783d90000000101']:
  if bytes.fromhex(sequence) not in constructor:raise ValueError('constructor operation absent')
 if image[0x144804af0-BASE:0x144804af5-BASE]!=bytes.fromhex('80496501c3'):raise ValueError('SRGB setter differs')
 # Original generated property records bind constructor offsets to names.
 for va,name,off in [(0x147decad0,'Filter',0x5d),(0x147decb00,'CompressionSettings',0x5c),(0x147deca40,'LODGroup',0x5f),(0x147decd28,'AddressX',0xd9),(0x147deccf8,'AddressY',0xda)]:
  p=va-BASE; ptr,=struct.unpack_from('<Q',image,p); actual,=struct.unpack_from('<i',image,p+36)
  end=image.index(b'\0',ptr-BASE,ptr-BASE+128)
  if image[ptr-BASE:end].decode('ascii')!=name or actual!=off:raise ValueError('reflected atlas field differs')
 p=0x147dec978-BASE;ptr,=struct.unpack_from('<Q',image,p);setter,=struct.unpack_from('<Q',image,p+48)
 if image[ptr-BASE:ptr-BASE+5]!=b'SRGB\0' or setter!=0x144804af0:raise ValueError('SRGB reflection differs')
 for table,index,expected in [(0x146c3a780,1,'TF_Bilinear'),(0x146c3a830,6,'TC_HDR'),(0x146c3a700,1,'TA_Clamp'),(0x146c3b6a0,0,'TEXTUREGROUP_World')]:
  ptr,val=struct.unpack_from('<QQ',image,table-BASE+16*index);end=image.index(b'\0',ptr-BASE,ptr-BASE+128)
  if val!=index or image[ptr-BASE:end].decode('ascii')!=expected:raise ValueError('original texture enum differs')
 return proof

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--repo',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args();root=a.repo.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False)
 proof=root/'porting/generated/material_pipeline/social-atlas-defaults-proof-1/receipt.json';dump=root/'porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin';asset=Path('/mnt/vibrato/fortnite-extracted-full/FortniteGame/Content/UI/SocialPanel/Art/CA_SocialPanel')
 image=read(dump);verify_proof(proof,image);header=read(asset.with_suffix('.uasset'),ASSET);payload=read(asset.with_suffix('.uexp'),EXPORT);decoded=decode_tags(header,payload)
 (out/'serialized-tags.json').write_text(json.dumps(decoded,indent=2)+'\n')
 sources=[Path(__file__).resolve(),proof,asset.with_suffix('.uasset'),asset.with_suffix('.uexp')];(out/'inputs').mkdir();items=[]
 for p in sources:
  data=read(p);h=sha(data);(out/'inputs'/h).write_bytes(data);items.append(dict(source=str(p),snapshot='inputs/'+h,sha256=h,bytes=len(data)))
 summary=dict(schema='social-atlas-independent-tags-v1',image_sha256=IMAGE,constructor_proof_sha256=PROOF,inputs=items,outputs=[dict(path='serialized-tags.json',sha256=sha((out/'serialized-tags.json').read_bytes()))],no_runtime_claim=True,limitations=['Tag parsing is bounded to this exact asset and four observed tag types.','LODGroup is absent as a serialized tag; its original inherited value is not proven.'])
 (out/'receipt.json').write_text(json.dumps(summary,indent=2)+'\n');print(sha((out/'receipt.json').read_bytes()))
if __name__=='__main__':main()
