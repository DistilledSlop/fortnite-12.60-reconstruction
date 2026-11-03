#!/usr/bin/env python3
"""Recover exact by-value FName field-copy setters into isolated proposals."""
import argparse
import collections
import hashlib
import importlib.util
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, '/tmp/common-input-capstone')
from capstone import Cs, CS_ARCH_X86, CS_MODE_64

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / 'porting/behavior/native_class_size_evidence_runtime'))
from scan import extract

ROOT = Path(__file__).resolve().parents[3]
REFL = ROOT / 'porting/generated/reflection'
HERE = Path(__file__).resolve().parent
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
BASE = 0x140000000
SHA = '338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680'
# Verified CoreUObject POD structs: reflected name -> byte size.
PODS = {'Vector': 12, 'Rotator': 12, 'Vector2D': 8, 'LinearColor': 16, 'Guid': 16, 'Color': 4, 'Quat': 16, 'Vector4': 16}
# P_FINISH frame advance (r9 form: rcx stays `this`).
ADV = 'mov rax, qword ptr [rdx + 0x20]; xor r9d, r9d; test rax, rax; setne r9b; add r9, rax; mov qword ptr [rdx + 0x20], r9; '
WRAP_HEAD = 'push rbx; sub rsp, (0x[0-9a-f]+); mov rax, qword ptr \\[rdx \\+ 0x20\\]; xor r9d, r9d; test rax, rax; mov rbx, r8; setne r9b; add r9, rax; mov qword ptr \\[rdx \\+ 0x20\\], r9; lea rdx, \\[rsp \\+ (0x[0-9a-f]+)\\]; call (0x[0-9a-f]+); '
NET = 0x80000000 | 0x01000000 | 0x00200000 | 0x4000 | 0x1000 | 0x100 | 0x80 | 0x40
ACCESS = {0x20000: 'public', 0x40000: 'private', 0x80000: 'protected'}
RET_FLAGS = 0x10000000000580  # public-native | Parm | OutParm | ReturnParm; no Const/Reference/etc.
FAMILY = {'eax': 'a', 'rax': 'a', 'ecx': 'c', 'rcx': 'c', 'edx': 'd', 'rdx': 'd'}
GPR4, GPR8 = ('eax', 'ecx', 'edx'), ('rax', 'rcx', 'rdx')
MEM = r'\[(r[a-z0-9]+)(?: \+ (0x[0-9a-f]+|[0-9]+))?\]'
load = lambda p: json.loads(p.read_text())


H=r'(0x[0-9a-f]+|[0-9]+)'
# Exact by-value FName extraction, complete frame advance and pure direct store.
THUNK=re.compile(r'mov qword ptr \[rsp \+ 8\], rbx; mov qword ptr \[rsp \+ 0x18\], rsi; push rdi; sub rsp, 0x20; xor edi, edi; mov rsi, rcx; mov rbx, rdx; mov qword ptr \[rsp \+ 0x38\], rdi; mov rcx, rdx; cmp qword ptr \[rdx \+ 0x20\], rdi; je (0x[0-9a-f]+); mov rdx, qword ptr \[rdx \+ 0x18\]; lea r8, \[rsp \+ 0x38\]; call 0x142ed9cf0; jmp (0x[0-9a-f]+); mov r8, qword ptr \[rdx \+ 0x80\]; mov rax, qword ptr \[r8 \+ 0x20\]; mov qword ptr \[rdx \+ 0x80\], rax; lea rdx, \[rsp \+ 0x38\]; call 0x142ed9d20; mov rax, qword ptr \[rbx \+ 0x20\]; mov rcx, rsi; mov rdx, qword ptr \[rsp \+ 0x38\]; test rax, rax; setne dil; add rdi, rax; mov qword ptr \[rbx \+ 0x20\], rdi; call (0x[0-9a-f]+); mov rbx, qword ptr \[rsp \+ 0x30\]; mov rsi, qword ptr \[rsp \+ 0x40\]; add rsp, 0x20; pop rdi; ret')
LEAF=re.compile(r'mov qword ptr \[rcx \+ (0x[0-9a-f]+)\], rdx; ret')

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--revalidate-own', action='store_true')
    parser.add_argument('--output-dir', type=Path, default=Path('/tmp/gameplay-bulk-native-copy-setter'))
    args = parser.parse_args()
    image = IMAGE.read_bytes()
    digest = hashlib.sha256(image).hexdigest()
    if digest != SHA:
        raise SystemExit('image digest mismatch')
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True

    def decode(addr):
        pos=addr-BASE
        if pos<0 or pos+256>len(image): return [], ''
        ins=[]
        for i in md.disasm(image[pos:pos+256],addr):
            ins.append(i)
            if i.mnemonic=='ret': return ins,'; '.join((x.mnemonic+' '+x.op_str).rstrip() for x in ins)
        return [], ''
    native = load(REFL / 'native_funcs.json')
    names = load(REFL / 'class_name_map.json')
    parents = load(REFL / 'class_parent_map.json')
    blocks = {c['block_va'].lower(): c for c in load(REFL / 'fortnite_game_reflection.json')['classes_in_groups_with_fort_names']}
    bits = load(REFL / 'bool_bits.json')['by_class']
    index = load(REFL / 'native_class_index.json')
    targets = collections.Counter(g.lower() for v in index.values() for g in v.get('gpsc_targets', []))
    size_cache = {}
    manifest = load(ROOT / 'porting/behavior/MANIFEST.json')
    byname = collections.defaultdict(list)
    for va, name in names.items():
        byname[name].append(va)
    selected = {x.strip() for x in (ROOT / 'porting/native_class_set.txt').read_text().splitlines() if x.strip() and not x.lstrip().startswith('#')}
    own = str(HERE.relative_to(ROOT)) + '/'
    ported = {(e.get('class'), e.get('function')) for e in load(ROOT / 'porting/behavior/MANIFEST.json') if e.get('status') == 'ported' and not (args.revalidate_own and (e.get('inc_path') or '').startswith(own))}
    headers = collections.defaultdict(list)
    for module in ('FortniteGame', 'FortniteUI'):
        for p in (ROOT / 'porting/native/Source' / module / 'Generated').glob('*.h'):
            headers[p.stem].append(p)
    spec = importlib.util.spec_from_file_location('gen_native_classes', ROOT / 'porting/gen_native_classes.py')
    generator = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = generator
    spec.loader.exec_module(generator)
    def size_of(va):
     if not isinstance(va,str): return None
     va=va.lower()
     if va in size_cache: return size_cache[va]
     block=blocks.get(va); name=names.get(va); parent=parents.get(va) or {}; entry=index.get(name) or {}
     gs=entry.get('gpsc_targets') or []
     if not block or not name or byname.get(name)!=[va] or parent.get('name')!=name or not parent.get('package') or entry.get('block_va','').lower()!=va or len(gs)!=1 or targets[gs[0].lower()]!=1:
      size_cache[va]=None; return None
     proof=extract(image,md,int(gs[0],0),name,parent['package'])
     if proof is None: size_cache[va]=None; return None
     pairs,_=generator.split_properties(block.get('properties',[]))
     ss={p.get('size_of_outer') for p,_ in pairs if p.get('kind')=='Bool' and isinstance(p.get('size_of_outer'),int) and p['size_of_outer']>0}
     if len(ss)>1 or ss and next(iter(ss))!=proof['size']: raise SystemExit('reflected size conflict: '+name)
     size_cache[va]=(proof['size'],bool(ss),proof['gpsc'])
     return size_cache[va]
    def occupancy(cls,block,own):
     pairs,notes=generator.split_properties(block['properties'])
     if notes: return None
     props=[p for p,_ in pairs]
     locations=[]
     for p in props:
      if p.get('kind')=='Bool':
       bs=[b for b in bits.get(cls,[]) if b['name']==p['name']]
       if len(bs)!=1 or not isinstance(bs[0].get('offset'),int): return None
       loc=bs[0]['offset']
      else: loc=p.get('offset')
      if not isinstance(loc,int) or loc<0 or loc>=own: return None
      locations.append((loc,p))
     fields=[e for e in manifest if e.get('class')==cls and e.get('kind')=='member_field' and e.get('status')=='ported' and not (args.revalidate_own and e.get('recovery_source')=='bulk_native_copy_runtime')]
     for e in fields:
      try: loc=int(str(e['offset']),0)
      except (KeyError,ValueError,TypeError): return None
      if loc<0 or loc>=own: return None
      locations.append((loc,e))
     hs=headers.get(cls,[])
     if len(hs)!=1: return None
     hdr=hs[0].read_text()
     annotated=re.findall(r'// Non-reflected field \(behaviour kind "member_field", offset (0x[0-9a-fA-F]+)\):[^\n]*\n\s*([^;\n]+);',hdr)
     native_fields=[e for e in manifest if e.get('class')==cls and e.get('kind')=='member_field' and e.get('status')=='ported']
     if hdr.count('// Non-reflected field (behaviour kind "member_field"')!=len(annotated): return None
     for off,decl in annotated:
      if not any(str(e.get('offset','')).lower()==off.lower() and (e.get('signature') or '').strip()==decl.strip() for e in native_fields): return None
     offsets=sorted({loc for loc,_ in locations}|{own})
     widths={'Bool':1,'Byte':1,'Int8':1,'Int16':2,'UInt16':2,'Int':4,'UInt32':4,'Float':4,'Int64':8,'UInt64':8,'Double':8,'Name':8,'Object':8,'Class':8,'Array':16,'Str':16}
     spans=[]
     for loc,p in locations:
      kind=p.get('kind'); width=widths.get(kind)
      if kind=='member_field':
       typ=(p.get('signature') or '').split(' ',1)[0]
       width={'bool':1,'uint8':1,'int8':1,'int16':2,'uint16':2,'int32':4,'uint32':4,'float':4,'int64':8,'uint64':8,'double':8}.get(typ)
      dim=p.get('array_dim',1)
      if not isinstance(dim,int) or dim<1: return None
      end=loc+width*dim if width else next(x for x in offsets if x>loc)
      if end>own: return None
      spans.append((loc,end))
     return spans
    definitions = set()
    for path in (ROOT / 'porting/native/Source').rglob('*.cpp'):
        if 'Generated' in path.parts:
            continue
        for cls, name in re.findall(r'\b([AUF]\w*)::(\w+)\s*\(', path.read_text(errors='replace')):
            definitions.add((cls, name))
    fields = {}
    results, rejects, total = [], collections.Counter(), 0
    for cls, funcs in sorted(native.items()):
      for name, fn in sorted(funcs.items()):
        total += 1
        ps=fn.get('params') if isinstance(fn,dict) else None
        if not isinstance(ps,list) or len(ps)!=1: rejects['not_one_parameter']+=1; continue
        pairs,notes=generator.split_properties(ps)
        if notes or len(pairs)!=1 or pairs[0][1]: rejects['parameter_metadata']+=1; continue
        p=pairs[0][0]; pname=p.get('name','')
        if p.get('kind')!='Name' or p.get('type_refs') or p.get('array_dim')!=1 or int(p.get('property_flags','0'),0)!=0x10000000000080 or not re.fullmatch(r'[A-Za-z_]\w*',pname): rejects['not_by_value_fname']+=1; continue
        try: flags,addr=int(fn['flags'],0),int(fn['exec_addr'],0)
        except (KeyError,ValueError,TypeError): rejects['missing_flags_or_address']+=1; continue
        if flags & (~0xffffffcf|NET|0x08000000|0x20000000|0x2000|0x40000000) or not flags&0x400 or (flags&0xe0000) not in ACCESS: rejects['flags']+=1; continue
        if (cls,name) in ported or (cls,name) in definitions: rejects['already_ported_or_maintained']+=1; continue
        vs=byname.get(cls,[])
        if len(vs)!=1 or cls not in selected or not re.fullmatch(r'[A-Za-z_]\w*',name): rejects['class_or_identifier']+=1; continue
        va=vs[0]; parent=parents.get(va,{}) ; block=blocks.get(va.lower())
        if not block or parent.get('name')!=cls or parent.get('package') not in ('/Script/FortniteGame','/Script/FortniteUI') or int(block['class_flags'],0)&0x4000 or len(headers[cls])!=1: rejects['class_metadata']+=1; continue
        ins,asm=decode(addr); m=THUNK.fullmatch(asm)
        if not m: rejects['unmatched_thunk']+=1; continue
        je=next((i for i in ins if i.mnemonic=='je'),None); jmp=next((i for i in ins if i.mnemonic=='jmp'),None)
        alt=next((i for i in ins if i.mnemonic=='mov' and i.op_str=='r8, qword ptr [rdx + 0x80]'),None); join=next((ins[k+1] for k,i in enumerate(ins[:-1]) if i.mnemonic=='call' and i.op_str=='0x142ed9d20'),None)
        if not (je and jmp and alt and join and int(je.op_str,0)==alt.address and int(jmp.op_str,0)==join.address): rejects['branch_targets']+=1; continue
        target=int(m.group(3),0); leaf=decode(target)[1]; q=LEAF.fullmatch(leaf)
        if not q: rejects['leaf_not_pure_copy']+=1; continue
        offset=int(q.group(1),0); own_proof=size_of(va); parent_proof=size_of(parent.get('parent_va'))
        if not own_proof or not parent_proof or not parent_proof[0]<=offset or offset+8>own_proof[0]: rejects['size_proof_or_bounds']+=1; continue
        hdr=headers[cls][0].read_text(); declaration=re.findall(r'\bclass\s+\w+_API\s+'+re.escape(cls)+r'\s*:\s*public\s+(\w+)',hdr)
        if declaration!=[parent.get('parent_name')] or names.get(parent.get('parent_va'))!=parent.get('parent_name'): rejects['parent_mismatch']+=1; continue
        spans=occupancy(cls,block,own_proof[0])
        if spans is None: rejects['unknown_occupancy']+=1; continue
        same=[e for e in manifest if e.get('class')==cls and e.get('kind')=='member_field' and e.get('status')=='ported' and e.get('offset')==hex(offset) and (e.get('signature') or '').startswith('FName ')]
        if same:
         if len(same)!=1: rejects['ambiguous_existing_field']+=1; continue
         prop=same[0]['function']
         # Existing field overlaps itself; independently reject overlap with another member.
         if sum(offset<end and start<offset+8 for start,end in spans)!=1: rejects['existing_field_overlap']+=1; continue
        else:
         if any(offset<end and start<offset+8 for start,end in spans): rejects['occupied']+=1; continue
         prop=f'NativeRecoveredFName_0x{offset:x}'
         taken={x.get('name') for x in block['properties']}|set(funcs)|{e.get('function') for e in manifest if e.get('class')==cls}
         if prop in taken: rejects['field_name_collision']+=1; continue
         fields[(cls,offset)]={'class':cls,'function':prop,'kind':'member_field','signature':f'FName {prop}','signature_status':'proposed','access':'public','offset':hex(offset),'inc_path':None,'confidence':'high','address':None,'status':'ported','recovery_source':'bulk_native_copy_setter_runtime','notes':'Original pure copy identifies storage; initialization remains unported. Source recovery only.'}
        sig=f'void {name}(FName {pname})'
        decl=re.search(r'^\s*[^\n;]*\b'+re.escape(name)+r'\s*\(',hdr,re.M)
        if decl and not re.search(r'^\s*'+re.escape(sig)+r'\s*;',hdr,re.M): rejects['function_header_mismatch']+=1; fields.pop((cls,offset),None); continue
        inc=HERE/f'{addr:x}_{cls}_{name}.inc'; body=f'// Original exec {hex(addr)}; pure writer {hex(target)}.\n// CONFIDENCE: HIGH\nthis->{prop} = {pname};\n'
        if inc.exists() and inc.read_text()!=body: raise SystemExit('existing body differs: '+str(inc))
        inc.write_text(body)
        entry={'class':cls,'function':name,'kind':'exec','signature':sig,'signature_status':'proposed','inc_path':str(inc.relative_to(ROOT)),'confidence':'high','address':hex(addr),'status':'ported','reflection_flags':fn['flags'],'static':False,'access':ACCESS[flags&0xe0000],'notes':f'Exact FFrame extraction and pure FName store at {hex(target)} to same-class field {prop} offset {hex(offset)}; source recovery only.'}
        results.append({'entry':entry,'exec':asm,'leaf':leaf,'target':hex(target),'offset':offset,'parent_size':parent_proof[0],'own_size':own_proof[0],'parent_gpsc':parent_proof[2],'own_gpsc':own_proof[2],'existing_field':bool(same)})
    proposed=list(fields.values())
    for i,a in enumerate(proposed):
      for b in proposed[:i]:
        if a['class']==b['class'] and abs(int(a['offset'],0)-int(b['offset'],0))<8: raise SystemExit('overlapping proposed fields')
    args.output_dir.mkdir(parents=True,exist_ok=True)
    (args.output_dir/'manifest-entries.json').write_text(json.dumps(proposed+[r['entry'] for r in results],indent=2)+'\n')
    report={'image_sha256':digest,'inventory_records':total,'candidates':len(results),'fields':len(fields),'rejections':dict(rejects),'results':results}
    (args.output_dir/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({k:report[k] for k in ('inventory_records','candidates','fields','rejections')},indent=2))
if __name__=='__main__': main()
