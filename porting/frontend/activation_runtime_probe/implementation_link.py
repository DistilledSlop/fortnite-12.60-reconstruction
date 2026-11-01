"""Bounded x86_64 non-LTO object code and link-recipe identity, fail closed.
Not a hermetic toolchain/shared-library/asset attestation or original ABI proof.
"""
import hashlib,re,shlex,struct,subprocess
from pathlib import Path
import verify_metadata as m

def sha(p):return m.sha(p)
def rows(debug,wanted):
 result={};proc=subprocess.Popen(['nm','-a','-S','--defined-only',str(debug)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
 for line in proc.stdout:
  parts=line.split()
  if len(parts)==4 and parts[3] in wanted:
   v=(int(parts[0],16),int(parts[1],16));m.need(parts[3] not in result or result[parts[3]]==v,'ambiguous linked symbol '+parts[3]);result[parts[3]]=v
 err=proc.stderr.read();rc=proc.wait();proc.stdout.close();proc.stderr.close();m.need(rc==0,'nm linked implementation '+err[-200:]);return result

def relocs(e,s):
 sec,start,size,_=s;out=[]
 for sh in e.sections:
  if sh[1]!=4 or sh[7]!=sec:continue
  for o in range(sh[4],sh[4]+sh[5],sh[9]):
   pos,info,add=m.unpack('QQq',e.data,o)
   if start<=pos<start+size:out.append((pos-start,info&0xffffffff,e.tables[sh[6]][info>>32],add))
 return out

def exact_code(e,s,linked,syms):
 """Resolve every relocation; compare all remaining bytes and exact code length.
 Supports ordinary x86_64 absolute/PC relative relocations. Unsupported forms fail.
 """
 name=s[3];m.need(name in syms,'defined linked implementation '+name);addr,size=syms[name];raw=bytearray(e.raw(s));m.need(0<size<=65536 and size==len(raw),'exact implementation symbol size '+name)
 actual=linked.read(addr,size);evidence=[];covered=set()
 for delta,kind,target,add in relocs(e,s):
  width={1:8,2:4,4:4,10:4,11:4}.get(kind);m.need(width is not None,'unsupported code relocation '+str(kind)+' '+name)
  m.need(delta+width<=size and not covered.intersection(range(delta,delta+width)),'overlapping code relocation');covered.update(range(delta,delta+width))
  sec,value,n,tname=target;place=addr+delta
  if tname in syms:
   dest=syms[tname][0];resolution={'symbol':tname,'address':dest}
  elif 0<sec<len(e.sections) and e.sections[sec][1]==1 and not e.sections[sec][2]&4:
   # Merged local constants lack a linked symbol. Decode the actual relocation,
   # then verify its complete string or bounded constant payload at that target.
   signed=kind in (2,4,11);encoded=int.from_bytes(actual[delta:delta+width],'little',signed=signed)
   dest=encoded-add+(place if kind in (2,4) else 0)
   sh=e.sections[sec];offset=value
   # Section symbol addends identify the constant, PC32 addend includes -4.
   literal_offset=offset+add+(4 if kind in (2,4) else 0)
   m.need(0<=literal_offset<sh[5],'local constant offset')
   body=e.data[sh[4]+literal_offset:sh[4]+sh[5]]
   if sh[2]&0x20:
    end=body.find(b'\0');m.need(0<=end<4096,'bounded terminated constant');payload=body[:end+1]
   else:payload=body[:min(n or 16,len(body))]
   m.need(payload and linked.read(dest+add+(4 if kind in (2,4) else 0),len(payload))==payload,'linked local constant payload '+name)
   resolution={'local_constant_sha256':hashlib.sha256(payload).hexdigest(),'bytes':len(payload)}
  else:raise ValueError('unresolved code relocation '+tname+' in '+name)
  value=dest+add-(place if kind in (2,4) else 0)
  lo,hi=(-(1<<(width*8-1)),(1<<(width*8-1))-1) if kind in (2,4,11) else (0,(1<<(width*8))-1)
  m.need(lo<=value<=hi,'relocation overflow');raw[delta:delta+width]=value.to_bytes(width,'little',signed=kind in (2,4,11));evidence.append({'offset':delta,'kind':kind,**resolution})
 m.need(bytes(raw)==actual,'exact relocated implementation bytes '+name)
 return {'symbol':name,'address':addr,'size':size,'object_code_sha256':hashlib.sha256(e.raw(s)).hexdigest(),'linked_code_sha256':hashlib.sha256(actual).hexdigest(),'relocations':evidence}

METHODS=('ActivateWidget','DeactivateWidget','IsActivated','NativeActivateRequest','NativeDeactivateRequest','NativeHandleActivated','NativeHandleDeactivated')
def audit(cpp_object,glue_object,binary,debug):
 m.need(m.build_id(binary)==m.build_id(debug),'implementation ELF/debug build ID');e=m.ObjectELF(cpp_object);g=m.ObjectELF(glue_object);linked=m.LinkedELF(binary)
 implementations={name:e.sym('UCommonActivatableWidget'+str(len(name))+name,'Ev') for name in METHODS}
 # Include complete constructor (C2 may alias C1 at identical code/address).
 implementations['constructor']=e.sym('UCommonActivatableWidgetC2','ERK18FObjectInitializer')
 thunks={name:g.sym('UCommonActivatableWidget'+str(len(name)+4)+'exec'+name,'') for name in METHODS[:3]}
 selected=[(e,s) for s in implementations.values()]+[(g,s) for s in thunks.values()]
 helpers=[];seen={s[3] for _,s in selected}
 for obj,s in selected:
  for _,_,target,_ in relocs(obj,s):
   name=target[3]
   if name in obj.symbols and name not in seen and ('CommonActivationCore' in name or ('UCommonActivatableWidget' in name and 'clE' in name)):
    helper=obj.symbols[name]
    if obj.sections[helper[0]][2]&4:
     m.need(len(selected)<256,'bounded activation helper closure');seen.add(name);helpers.append((obj,helper));selected.append((obj,helper))
 wanted={s[3] for _,s in selected}
 for obj,s in selected:wanted.update(t[3] for _,_,t,_ in relocs(obj,s) if t[3])
 syms=rows(debug,wanted);code={k:exact_code(e,v,linked,syms) for k,v in implementations.items()};native={}
 for name,s in thunks.items():
  impl=implementations[name][3]
  refs=[x for x in relocs(g,s) if x[2][3]==impl and x[1] in (2,4) and x[0]>0 and g.raw(s)[x[0]-1] in (0xe8,0xe9)]
  m.need(refs,'actual native exec implementation relocation '+name)
  native[name]=exact_code(g,s,linked,syms)
 return {'passed':True,'implementation':code,'native_exec':native,'out_of_line_helpers':{s[3]:exact_code(obj,s,linked,syms) for obj,s in helpers},'scope':'Exact defined non-LTO implementation/thunk code, resolved relocations and actual native-exec implementation references; inlined helpers are covered by caller bytes.'}

def recipe(script,response,binary,objects):
 lines=Path(script).read_text().splitlines();commands=[shlex.split(x) for x in lines if x.strip() and not x.lstrip().startswith('#')]
 candidates=[c for c in commands if '-o' in c and any(x.startswith('-Wl,@') for x in c)]
 m.need(len(candidates)==1,'unique UBT link command');cmd=candidates[0]
 m.need(Path(cmd[0]).is_absolute() and Path(cmd[0]).is_file(),'absolute existing link driver')
 m.need(cmd.count('-o')==1 and cmd[cmd.index('-o')+1]==str(binary),'exact linked output path')
 m.need([x for x in cmd if x.startswith('-Wl,@')]==['-Wl,@'+str(response)],'exact link response command')
 tokens=shlex.split(Path(response).read_text());m.need(not any(x.startswith('@') or '-flto' in x for x in cmd+tokens),'bounded non-LTO single response recipe')
 m.need('-fuse-ld=lld' in cmd and '-Wl,--build-id' in cmd,'known UBT lld recipe')
 for obj in set(map(str,objects)):m.need(tokens.count(obj)==1,'exact fresh object link inclusion '+obj)
 return {'script':{'path':str(script),'sha256':sha(script)},'response':{'path':str(response),'sha256':sha(response)},'driver':{'path':cmd[0],'sha256':sha(cmd[0])},'objects':sorted(set(map(str,objects))),'scope':'Recorded UBT recipe plus exact linked code evidence; script presence alone is not an execution claim.'}
