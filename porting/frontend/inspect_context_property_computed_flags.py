#!/usr/bin/env python3
"""Replay original GetContext property construction and exact added type flags."""
import argparse,hashlib,json,struct
from pathlib import Path
BASE=0x140000000
IMAGE_SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
def need(ok,message):
 if not ok:raise ValueError(message)
def inspect(path):
 data=Path(path).read_bytes();need(hashlib.sha256(data).hexdigest()==IMAGE_SHA,'late image hash');windows=[]
 def read(va,n):
  b=data[va-BASE:va-BASE+n];need(len(b)==n,'original window');windows.append({'va':hex(va),'size':n,'sha256':hashlib.sha256(b).hexdigest(),'bytes':b.hex()});return b
 def exact(va,h):need(read(va,len(bytes.fromhex(h))).hex()==h,'original instruction '+hex(va))
 def call(va,target):
  b=read(va,5);need(b[0]==0xe8 and va+5+struct.unpack_from('<i',b,1)[0]==target,'original direct call '+hex(va))
 call(0x1408234ae,0x142ec43b0);call(0x142ec46e2,0x142ec2160)
 exact(0x142ec21d7,'418b451883e01f44896424508b8482383cec024803c2ffe0')
 targets={kind:BASE+struct.unpack('<I',read(0x142ec3c38+kind*4,4))[0] for kind in (0x11,0x12)}
 need(targets=={0x11:0x142ec2e8b,0x12:0x142ec2b2f},'class/object dispatch')
 # FPropertyParams+10 is the raw flags. Both original cases OR the identical type mask.
 exact(0x142ec2b9b,'498b4510');exact(0x142ec2bb9,'480bc1')
 exact(0x142ec2bc1,'4889442428');call(0x142ec2bd1,0x142e75a40)
 exact(0x142ec2f0f,'4d8b7510');exact(0x142ec2f3e,'4c0bf1')
 exact(0x142ec2f46,'4c89742428');call(0x142ec2f5b,0x142e75a40)
 masks=[]
 for va in (0x142ec2baf,0x142ec2f2f):
  b=read(va,10);need(b[:2]==b'\x48\xb9','original MOVABS');masks.append(struct.unpack_from('<Q',b,2)[0])
 need(masks==[0x0008001040000200]*2,'original exact computed flags')
 # After push rdi/sub rsp30/call return, caller stack+28 is constructor stack+68.
 exact(0x142e75a4a,'574883ec30');exact(0x142e75a7c,'488b44246848894740')
 params=read(0x14783de30,72);array=struct.unpack_from('<Q',params,48)[0]
 pointers=struct.unpack('<3Q',read(array,24));properties=[]
 expected=[('ReturnValue',0x10000000000580,0x12,16),('Class',0x14000000000080,0x11,8),('ContextObject',0x10000000000080,0x12,0)]
 for pointer,(name,flags,kind,offset) in zip(pointers,expected):
  b=read(pointer,56 if kind==0x11 else 48);need(struct.unpack_from('<QIIII',b,16)==(flags,kind,0x45,1,offset),'raw original property fields')
  name_va=struct.unpack_from('<Q',b)[0];need(read(name_va,len(name)+1)==name.encode()+b'\0','original property name')
  properties.append({'name':name,'raw_flags':hex(flags),'computed_flags':hex(masks[0]),'constructed_flags':hex(flags|masks[0]),'kind':hex(kind),'offset':offset,'array_dim':1})
 return {'passed':True,'image_sha256':IMAGE_SHA,'computed_flags':hex(masks[0]),'properties':properties,'windows':windows,'scope':'Original static constructor dataflow, not an original live heap measurement or complete runtime acceptance.'}
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--image',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();report=inspect(a.image)
 with a.output.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps({k:v for k,v in report.items() if k!='windows'}))
if __name__=='__main__':main()
