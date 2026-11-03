#!/usr/bin/env python3
"""Inventory same-class, parameterless direct-wrapper native-target collisions.

This deliberately emits no proposals: collisions require additional snippet and
maintained/generated declaration audits before they can be promoted.
"""
import collections, hashlib, json, mmap, re, sys
from pathlib import Path
sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
ROOT=Path(__file__).resolve().parents[3]; REF=ROOT/'porting/generated/reflection'
IMAGE=Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
SHA='338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'; BASE=0x140000000
OUT=Path('/tmp/gameplay-bulk-same-class-alias'); load=lambda p:json.loads(p.read_text())
NET=0x80000000|0x01000000|0x00200000|0x4000|0x1000|0x100|0x80|0x40
ACCESS={0x20000,0x40000,0x80000}
WRAP='push rbx; sub rsp, 0x20; mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
PAT=re.compile(re.escape(WRAP)+r'call (0x[0-9a-f]+); (mov (?:byte|dword|qword) ptr \[rbx\], (?:al|eax|rax)|movss dword ptr \[rbx\], xmm0); add rsp, 0x20; pop rbx; ret$')
KINDS={'Bool':'mov byte ptr [rbx], al','Byte':'mov byte ptr [rbx], al','Int':'mov dword ptr [rbx], eax','Float':'movss dword ptr [rbx], xmm0','Object':'mov qword ptr [rbx], rax','Class':'mov qword ptr [rbx], rax'}
def main():
 if hashlib.file_digest(IMAGE.open('rb'),'sha256').hexdigest()!=SHA: raise SystemExit('image digest mismatch')
 native=load(REF/'native_funcs.json'); manifest=load(ROOT/'porting/behavior/MANIFEST.json')
 selected={s.strip() for s in (ROOT/'porting/native_class_set.txt').read_text().splitlines() if s.strip() and not s.lstrip().startswith('#')}
 names=load(REF/'class_name_map.json'); parents=load(REF/'class_parent_map.json'); refl=load(REF/'fortnite_game_reflection.json')
 byname=collections.defaultdict(list)
 for va,n in names.items(): byname[n].append(va)
 blocks={x['block_va'].lower():x for x in refl['classes_in_groups_with_fort_names']}
 ported={(e.get('class'),e.get('function')):e for e in manifest if e.get('status')=='ported'}
 md=Cs(CS_ARCH_X86,CS_MODE_64); groups=collections.defaultdict(list); counts=collections.Counter()
 with IMAGE.open('rb') as f, mmap.mmap(f.fileno(),0,access=mmap.ACCESS_READ) as img:
  for cls,fs in sorted(native.items()):
   vs=byname.get(cls,[])
   if cls not in selected or len(vs)!=1: continue
   va=vs[0]; b=blocks.get(va.lower()); parent=parents.get(va,{})
   if not b or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(b['class_flags'],0)&0x4000: continue
   for name,entry in sorted(fs.items()):
    if not isinstance(entry,dict) or not re.fullmatch(r'[A-Za-z_]\w*',name): continue
    try: flags=int(entry['flags'],0); addr=int(entry['exec_addr'],0)
    except (KeyError,ValueError,TypeError): continue
    if flags&0x8000000 or not flags&0x400 or flags&(~0xffffffcf|NET|0x20000000|0x2000) or flags&0xe0000 not in ACCESS: continue
    ps=entry.get('params')
    if not isinstance(ps,list) or len(ps)!=1: continue
    p=ps[0]
    try: pf=int(p['property_flags'],0)
    except (KeyError,ValueError,TypeError): continue
    if p.get('name')!='ReturnValue' or p.get('array_dim')!=1 or not pf&0x400 or pf&0x08000000 or p.get('kind') not in KINDS: continue
    if addr<BASE or addr-BASE+96>len(img): continue
    ins=[]
    for i in md.disasm(img[addr-BASE:addr-BASE+96],addr):
     ins.append((i.mnemonic+' '+i.op_str).rstrip())
     if i.mnemonic in ('ret','jmp'): break
    match=PAT.fullmatch('; '.join(ins))
    if not match or match.group(2)!=KINDS[p['kind']]: continue
    counts['eligible_direct_wrappers']+=1
    # The entire reflected return property, flags and class match exactly.
    key=(cls,flags,json.dumps(ps,sort_keys=True),match.group(1))
    groups[key].append((name,hex(addr)))
 collisions=[]
 for (cls,flags,params,target),items in sorted(groups.items()):
  sources=[(n,a) for n,a in items if (cls,n) in ported and ported[cls,n].get('kind')=='exec']
  unknown=[(n,a) for n,a in items if (cls,n) not in ported and (cls,n+'_Implementation') not in ported]
  for n,a in unknown:
   for source,sa in sources:
    collisions.append(dict(class_=cls,function=n,exec_addr=a,source=source,source_exec=sa,target=target,flags=hex(flags)))
 OUT.mkdir(parents=True,exist_ok=True)
 report={'image_sha256':SHA,'counts':dict(counts,collisions_requiring_audit=len(collisions),proposals=0),'collisions':collisions}
 (OUT/'report.json').write_text(json.dumps(report,indent=2)+'\n'); (OUT/'manifest-entries.json').write_text('[]\n')
 print(json.dumps(report['counts'],indent=2))
if __name__=='__main__': main()
