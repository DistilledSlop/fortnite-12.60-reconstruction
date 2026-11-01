#!/usr/bin/env python3
"""Compare fresh UHT object AND linked ELF bytes against the original memory image."""
import argparse, hashlib, json, re, struct, subprocess
from pathlib import Path

def sha(p):
    with open(p,'rb') as f:
        h=hashlib.sha256()
        for b in iter(lambda:f.read(1<<20),b''): h.update(b)
        return h.hexdigest()
def need(v,s):
    if not v: raise ValueError(s)
def unpack(fmt,b,o=0): return struct.unpack_from('<'+fmt,b,o)
class ObjectELF:
 def __init__(self,p):
    self.data=Path(p).read_bytes();b=self.data;need(b[:6]==b'\x7fELF\x02\x01' and unpack('H',b,16)[0]==1,'ELF64 relocatable')
    off=unpack('Q',b,40)[0];size,n=unpack('HH',b,58);self.sections=[unpack('IIQQQQIIQQ',b,off+i*size) for i in range(n)];self.symbols={};self.tables={}
    for i,s in enumerate(self.sections):
     if s[1]!=2: continue
     ns=self.sections[s[6]];names=b[ns[4]:ns[4]+ns[5]];table=[]
     for j in range(s[4],s[4]+s[5],s[9]):
      ni,_,_,sec,val,sz=unpack('IBBHQQ',b,j);name=names[ni:names.index(0,ni)].decode();item=(sec,val,sz,name);table.append(item)
      if sec and sec<len(self.sections): self.symbols[name]=item
     self.tables[i]=table
 def sym(self,part,end):
    a=[v for k,v in self.symbols.items() if part in k and k.endswith(end)];need(len(a)==1,'unique symbol '+part+' '+end);return a[0]
 def raw(self,s):
    sec,v,n,_=s;off=self.sections[sec][4]+v;return self.data[off:off+n]
 def rel(self,s,delta,literal=False):
    sec,v,_,_=s;found=[]
    for sh in self.sections:
     if sh[1]!=4 or sh[7]!=sec:continue
     for o in range(sh[4],sh[4]+sh[5],sh[9]):
      addr,info,add=unpack('QQq',self.data,o)
      if addr!=v+delta:continue
      rs,rv,_,name=self.tables[sh[6]][info>>32]
      if literal:
       need(0<rs<len(self.sections),'literal section');pos=self.sections[rs][4]+rv+add;end=self.data.index(0,pos,pos+256);found.append(self.data[pos:end].decode('ascii'))
      else:found.append(name)
    need(len(found)==1,'unique relocation');return found[0]

class LinkedELF:
 def __init__(self,p):
    self.path=Path(p);f=self.path.open('rb');h=f.read(64);need(h[:6]==b'\x7fELF\x02\x01' and unpack('H',h,16)[0]==2,'linked executable ELF64 ET_EXEC');off=unpack('Q',h,32)[0];ent,n=unpack('HH',h,54);f.seek(off);ph=f.read(ent*n);f.close();self.loads=[]
    for i in range(n):
     typ,flags,ofs,va,pa,filesz,memsz,al=unpack('IIQQQQQQ',ph,i*ent)
     if typ==1:self.loads.append((va,va+filesz,ofs))
 def read(self,va,n):
    maps=[x for x in self.loads if x[0]<=va and va+n<=x[1]];need(len(maps)==1,'linked address file backed');a,_,ofs=maps[0]
    with self.path.open('rb') as f:
     f.seek(ofs+va-a);b=f.read(n)
    need(len(b)==n,'linked read');return b
 def cstr(self,va):return self.read(va,128).split(b'\0',1)[0].decode('ascii')

def symbols(debug):
    prefix='Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics'; needles=[prefix,'Z_Construct_UFunction_UBlueprintContextLibrary_GetContext','Z_Construct_UClass_UBlueprintContextLibrary','Z_Construct_UClass_USubsystem_NoRegister','Z_Construct_UClass_UObject_NoRegister','Z_Construct_UClass_UClass','UBlueprintContextLibrary14execGetContext','StaticRegisterNativesUBlueprintContextLibrary']
    result=[]
    p=subprocess.Popen(['nm','-a','-S','--defined-only',str(debug)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
    for line in p.stdout:
     if any(n in line for n in needles):
      m=re.match(r'^([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+\S\s+(\S+)$',line)
      if m:result.append((int(m[1],16),int(m[2],16),m[3]))
    err=p.stderr.read();rc=p.wait();p.stdout.close();p.stderr.close();need(rc==0,'nm debug: '+err[-300:]);return result

def one_syms(s,part,end=None):
    a=[x for x in s if part in x[2] and (end is None or x[2].endswith(end))];need(len(a)==1,'linked symbol '+part+' '+str(end));return a[0]
def build_id(p):
    r=subprocess.run(['readelf','-n',str(p)],capture_output=True,text=True);m=re.findall(r'Build ID: ([0-9a-f]+)',r.stdout);need(len(m)==1,'ELF build ID');return m[0]
def lea_to(code,addr,dest):
    for i in range(len(code)):
     if i+7<=len(code) and code[i]==0x48 and code[i+1] in (0x8d,0x8b) and code[i+2]&0xc7==5 and addr+i+7+unpack('i',code,i+3)[0]==dest:return True
     if i+5<=len(code) and 0xb8<=code[i]<=0xbf and unpack('I',code,i+1)[0]==dest:return True
     if i+10<=len(code) and code[i]==0x48 and 0xb8<=code[i+1]<=0xbf and unpack('Q',code,i+2)[0]==dest:return True
    return False

def instruction_ref(binary,addr,size,dest):
    need(0<size<=65536,'bounded function size')
    r=subprocess.run(['objdump','-d','--start-address='+hex(addr),'--stop-address='+hex(addr+size),str(binary)],capture_output=True,text=True)
    need(r.returncode==0,'objdump linked function')
    for line in r.stdout.splitlines():
     parts=line.split('\t')
     if len(parts)<3 or not re.match(r'^\s*(?:lea|mov)[a-z]*\s',parts[-1]):continue
     for imm,comment in re.findall(r'\$(0x[0-9a-fA-F]+)|#\s*(?:0x)?([0-9a-fA-F]+)',parts[-1]):
      if int(imm or comment,16)==dest:return True
    return False

def audit(original,cpp,obj,exe,debug):
 need(sha(original)=='604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d','original image hash')
 with open(original,'rb') as f:
  def read(va,n): f.seek(va-0x140000000);b=f.read(n);need(len(b)==n,'original window');return b
  raw=read(0x14783de30,72);need(unpack('Q',raw,0)[0]==0x140823460 and unpack('Q',raw,8)[0]==0 and read(unpack('Q',raw,16)[0],11).split(b'\0')[0]==b'GetContext','original owner/name'); need(lea_to(read(0x140823490,128),0x140823490,0x14783de30),'original constructor binds params');need(unpack('Q',raw,40)[0]==24 and unpack('III',raw,56)==(3,0x45,0x14022409),'original function')
  arr=unpack('Q',raw,48)[0];ptrs=unpack('QQQ',read(arr,24));orig=[read(v,56 if i==1 else 48) for i,v in enumerate(ptrs)]
  expected=[(0x10000000000580,0x12,0x45,1,16),(0x14000000000080,0x11,0x45,1,8),(0x10000000000080,0x12,0x45,1,0)]
  for p,t,name in zip(orig,expected,('ReturnValue','Class','ContextObject')):
   need(unpack('QIIII',p,16)==t and read(unpack('Q',p,0)[0],len(name)+1)==name.encode()+b'\0','original property fields/name')
  need(unpack('Q',orig[0],40)[0]==0x1447ff950 and unpack('QQ',orig[1],40)==(0x1447ff950,0x142e497d0) and unpack('Q',orig[2],40)[0]==0x142eea520,'original property type getters')
 text=Path(cpp).read_text();need('DEFINE_FUNCTION(UBlueprintContextLibrary::execGetContext)' in text and 'Z_Construct_UFunction_UBlueprintContextLibrary_GetContext' in text,'fresh generated native exec')
 e=ObjectELF(obj);prefix='Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics';fn=e.sym(prefix,'10FuncParamsE');f=e.raw(fn);need(len(f)==72 and f[40:48]==raw[40:48] and f[56:72]==raw[56:72],'object function fields');need(e.rel(fn,16,True)=='GetContext' and e.rel(fn,0).endswith('Z_Construct_UClass_UBlueprintContextLibraryv'),'object owner/name')
 registration=e.sym('StaticRegisterNativesUBlueprintContextLibrary','E5Funcs');need(len(e.raw(registration))==16 and e.rel(registration,0,True)=='GetContext' and 'UBlueprintContextLibrary14execGetContext' in e.rel(registration,8),'object native registration pair')
 pa=e.sym(prefix,'12PropPointersE');need(e.rel(fn,48)==pa[3],'object property array');names=['ReturnValue','Class','ContextObject'];props=[]
 for i,name in enumerate(names):
  sym=e.sym(prefix,'NewProp_'+name+'E');data=e.raw(sym);need(e.rel(pa,8*i)==sym[3] and e.rel(sym,0,True)==name,'object property order/name '+name);need(data[16:40]==orig[i][16:40],'object property fields '+name)
  typ=['Z_Construct_UClass_USubsystem_NoRegisterv','Z_Construct_UClass_USubsystem_NoRegisterv','Z_Construct_UClass_UObject_NoRegisterv'][i];need(e.rel(sym,40).endswith(typ),'object type '+name)
  if i==1:need(e.rel(sym,48).endswith('Z_Construct_UClass_UClassv'),'object class getter')
  props.append(sym)
 need(build_id(exe)==build_id(debug),'binary/debug build ID');syms=symbols(debug);linked=LinkedELF(exe)
 lf=one_syms(syms,prefix,'10FuncParamsE');b=linked.read(lf[0],72);need(b[40:48]==raw[40:48] and b[56:72]==raw[56:72] and linked.cstr(unpack('Q',b,16)[0])=='GetContext','linked function fields')
 owner=one_syms(syms,'Z_Construct_UClass_UBlueprintContextLibrary','Z_Construct_UClass_UBlueprintContextLibraryv');need(unpack('Q',b,0)[0]==owner[0],'linked owner getter')
 parr=one_syms(syms,prefix,'12PropPointersE');need(unpack('Q',b,48)[0]==parr[0],'linked property array pointer');pointers=unpack('QQQ',linked.read(parr[0],24))
 for i,name in enumerate(names):
  q=one_syms(syms,prefix,'NewProp_'+name+'E');need(pointers[i]==q[0],'linked property order');p=linked.read(q[0],56 if i==1 else 48);need(p[16:40]==orig[i][16:40] and linked.cstr(unpack('Q',p,0)[0])==name,'linked property fields '+name)
  type_name=['Z_Construct_UClass_USubsystem_NoRegister','Z_Construct_UClass_USubsystem_NoRegister','Z_Construct_UClass_UObject_NoRegister'][i];target=one_syms(syms,type_name,type_name+'v');need(unpack('Q',p,40)[0]==target[0],'linked type '+name)
  if i==1:need(unpack('Q',p,48)[0]==one_syms(syms,'Z_Construct_UClass_UClass','Z_Construct_UClass_UClassv')[0],'linked class getter')
 creator=one_syms(syms,'Z_Construct_UFunction_UBlueprintContextLibrary_GetContext','v');need(instruction_ref(exe,creator[0],creator[1],lf[0]),'linked constructor binds FuncParams')
 execsym=one_syms(syms,'UBlueprintContextLibrary14execGetContext');registration=one_syms(syms,'StaticRegisterNativesUBlueprintContextLibrary','E5Funcs');pair=linked.read(registration[0],16);need(linked.cstr(unpack('Q',pair,0)[0])=='GetContext' and unpack('Q',pair,8)[0]==execsym[0],'linked native registration pair')
 register=one_syms(syms,'StaticRegisterNativesUBlueprintContextLibrary','Ev');need(instruction_ref(exe,register[0],register[1],registration[0]),'linked native registration uses pair')
 return {'function_original_hex':raw.hex(),'original_property_hex':[x.hex() for x in orig],'linked_function_hex':b.hex(),'build_id':build_id(exe),'artifacts':{str(p):sha(p) for p in (original,cpp,obj,exe,debug)},'passed':True}

def main():
 p=argparse.ArgumentParser();[p.add_argument('--'+name,required=True,type=Path) for name in ('original','cpp','object','binary','debug')];a=p.parse_args();print(json.dumps(audit(a.original,a.cpp,a.object,a.binary,a.debug),indent=2))
if __name__=='__main__':main()
